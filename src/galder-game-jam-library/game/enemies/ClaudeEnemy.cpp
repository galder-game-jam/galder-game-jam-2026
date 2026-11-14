#include "ClaudeEnemy.h"
#include "../projectiles/Projectile.h"
#include "../../system/World.h"
#include <cmath>
#include <optional>
#include <vector>

namespace ggj
{
    void ClaudeEnemy::move(const raylib::Vector2& toMove)
    {
    }

    void ClaudeEnemy::update(float timeDelta)
    {
        if (m_body == nullptr)
            return;

        // Invulnerability flash
        if (m_invulTimer > 0.f)
        {
            m_invulTimer -= timeDelta;
            m_flashTimer += timeDelta;
            while (m_flashTimer >= FLASH_INTERVAL)
            {
                m_flashTimer -= FLASH_INTERVAL;
                m_isVisible = !m_isVisible;
            }
            if (m_invulTimer <= 0.f)
            {
                m_invulTimer = 0.f;
                m_isVisible  = true;
            }
        }

        // Boss AI. Only the host rolls states; clients get theirs through applyNetworkState().
        if (!m_isSittingDuck)
        {
            if (isAuthority())
            {
                m_stateTimer -= timeDelta;
                if (m_stateTimer <= 0.f)
                {
                    ClaudeState newState;
                    do
                    {
                        newState = static_cast<ClaudeState>(GetRandomValue(0, CLAUDE_STATE_COUNT - 1));
                    } while (newState == m_bossState);
                    setBossState(newState);
                    m_stateTimer = m_stateInterval;
                }
            }

            const std::optional<raylib::Vector2> target = (m_bossState == ClaudeState::Walk && m_world != nullptr) ? m_world->getPlayerPosition(m_targetPlayerId) : std::nullopt;
            // The chased player left mid-walk: go after someone else.
            if (m_bossState == ClaudeState::Walk && !target.has_value() && isAuthority())
                pickTarget();

            if (target.has_value())
            {
                const raylib::Vector2 myPos = getPosition();
                const float dx = target->x - myPos.x;
                const float dy = target->y - myPos.y;
                const float dist = std::sqrt(dx * dx + dy * dy);

                if (dist > 1.f)
                {
                    m_body->SetLinearVelocity({ (dx / dist) * m_followSpeed, (dy / dist) * m_followSpeed });
                    m_flip = dx < 0.f;
                }
                else
                {
                    m_body->SetLinearVelocity({ 0.f, 0.f });
                }
            }
            else
            {
                m_body->SetLinearVelocity({ 0.f, 0.f });
            }
        }
        else
        {
            m_body->SetLinearVelocity({ 0.f, 0.f });
        }

        PhysicsObject::update(timeDelta);

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void ClaudeEnemy::setBossState(ClaudeState newState)
    {
        m_bossState = newState;
        switch (newState)
        {
            case ClaudeState::Idle:
                m_animation = m_idleAnimation;
                break;
            case ClaudeState::Walk:
                m_animation = m_walkAnimation;
                if (isAuthority())
                    pickTarget();
                break;
            case ClaudeState::Attack:
                m_animation = m_attackAnimation;
                // Clients receive the host's Claudlings over the network instead.
                if (m_world != nullptr && isAuthority())
                    m_world->spawnClaudlings(getPosition(), m_followSpeed / 2.f, m_knockback / 2.f);
                break;
        }
    }

    void ClaudeEnemy::pickTarget()
    {
        if (m_world == nullptr)
            return;

        const std::vector<uint32_t> playerIds = m_world->getActivePlayerIds();
        if (playerIds.empty())
            return;
        m_targetPlayerId = playerIds[static_cast<size_t>(GetRandomValue(0, static_cast<int>(playerIds.size()) - 1))];
    }

    bool ClaudeEnemy::isAuthority() const
    {
        return m_world == nullptr || m_world->isEnemyAuthority();
    }

    void ClaudeEnemy::startInvulnerability()
    {
        m_invulTimer = INVUL_DURATION;
        m_flashTimer = 0.f;
    }

    EnemyNetworkState ClaudeEnemy::getNetworkState() const
    {
        EnemyNetworkState state = PhysicsSprite::getNetworkState();
        state.aiState = static_cast<uint8_t>(m_bossState);
        state.targetPlayerId = m_targetPlayerId;
        return state;
    }

    void ClaudeEnemy::applyNetworkState(const EnemyNetworkState& state)
    {
        PhysicsSprite::applyNetworkState(state);

        // Someone (maybe another peer) hit the boss on the host: flash like a local hit would.
        if (state.lives < m_lives && m_invulTimer <= 0.f)
            startInvulnerability();
        m_lives = state.lives;
        m_targetPlayerId = state.targetPlayerId;

        if (state.aiState < CLAUDE_STATE_COUNT)
        {
            const ClaudeState hostState = static_cast<ClaudeState>(state.aiState);
            if (hostState != m_bossState)
                setBossState(hostState);
        }
    }

    void ClaudeEnemy::onNetworkDamage(int32_t damage)
    {
        // The attacker lives on a client, which gets the kill credit through the network instead.
        takeDamage(&m_soundManager, nullptr, false, damage);
    }

    void ClaudeEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage)
    {
        // Already dead (waiting for removal): don't count the boss kill a second time.
        if (shouldBeDestroyed())
            return;

        // The host owns the boss's health. Send the hit there; the lives it syncs back drive the
        // HP bar and the hit flash, and it removes the boss everywhere when it dies.
        if (!isAuthority())
        {
            m_world->reportEnemyDamage(*this, damage);
            return;
        }

        if (m_invulTimer > 0.f)
            return;

        m_lives -= damage;

        if (m_body != nullptr)
            m_body->ApplyLinearImpulseToCenter({fromLeft ? -50.f : 50.f, 0.f}, true);

        if (m_lives <= 0)
        {
            if (soundManager != nullptr)
                soundManager->playSound(SoundName::EnemyDie);
            destroy();
            if (attacker != nullptr)
                attacker->getUserData()->enemiesKilled++;
            if (m_world != nullptr)
                m_world->clearBoss();
        }
        else
        {
            startInvulnerability();
        }
    }

    void ClaudeEnemy::onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage)
    {
        takeDamage(soundManager, attacker, fromLeft, damage);
    }

    void ClaudeEnemy::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        UserData* userDataB = b->getUserData();

        if (userDataB->getObjectType() == ObjectType::Player)
        {
            b2Vec2 vel = m_body->GetLinearVelocity();
            float len = std::sqrt(vel.x * vel.x + vel.y * vel.y);
            if (len < 0.001f)
            {
                const raylib::Vector2 myPos     = getPosition();
                const raylib::Vector2 playerPos = b->getPosition();
                const float dx = playerPos.x - myPos.x;
                const float dy = playerPos.y - myPos.y;
                len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.001f)
                {
                    vel = { dx / len, dy / len };
                    len = 1.f;
                }
            }
            if (len > 0.001f)
                b->getBody()->ApplyLinearImpulseToCenter({ (vel.x / len) * m_knockback, (vel.y / len) * m_knockback }, true);
        }
        else if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            takeDamage(&m_soundManager, p->getOwner(), false, PROJECTILE_DAMAGE);
        }
    }

} // ggj
