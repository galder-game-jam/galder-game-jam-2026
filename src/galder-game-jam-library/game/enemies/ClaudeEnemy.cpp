#include "ClaudeEnemy.h"
#include "../projectiles/Projectile.h"
#include "../../system/World.h"
#include <algorithm>
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

        m_shakeTime += timeDelta;
        if (m_bossState == ClaudeState::Transform)
            m_transformElapsed = std::min(m_transformElapsed + timeDelta, TRANSFORM_DURATION);

        // Boss AI. Only the host rolls states; clients get theirs through applyNetworkState().
        // A dying boss always finishes dying, even with the AI switched off for testing.
        if (isAuthority() && (!m_isSittingDuck || m_bossState == ClaudeState::Dying))
        {
            m_stateTimer -= timeDelta;
            if (m_stateTimer <= 0.f)
            {
                finishState();
                if (shouldBeDestroyed())
                    return;
            }
        }

        if (!m_isSittingDuck)
        {
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

    void ClaudeEnemy::draw()
    {
        const raylib::Vector2 restPosition = m_position;
        if (isShaking())
        {
            // Two out-of-step sines read as a nervous rattle rather than a smooth sway.
            m_position.x += SHAKE_AMPLITUDE * std::sin(m_shakeTime * 61.f);
            m_position.y += SHAKE_AMPLITUDE * 0.5f * std::sin(m_shakeTime * 47.f);
        }

        const float enrage = getEnrageProgress();
        if (enrage > 0.f)
            m_paletteSwap.begin(BuildEnragePaletteSwap(enrage));
        PhysicsSprite::draw();
        if (enrage > 0.f)
            m_paletteSwap.end();

        m_position = restPosition;
    }

    PaletteSwap ClaudeEnemy::BuildEnragePaletteSwap(float progress)
    {
        PaletteSwap swap;
        const float t = std::clamp(progress, 0.f, 1.f);
        if (t <= 0.f)
            return swap;

        const auto blend = [t](const ::Color& from, const ::Color& to)
        {
            const auto channel = [t](unsigned char a, unsigned char b)
            { return static_cast<unsigned char>(std::lround(static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t)); };
            return ::Color { channel(from.r, to.r), channel(from.g, to.g), channel(from.b, to.b), 255 };
        };
        const auto add = [&swap, &blend](const ::Color& from, const ::Color& to)
        {
            swap.from[static_cast<size_t>(swap.count)] = from;
            swap.to[static_cast<size_t>(swap.count)] = blend(from, to);
            ++swap.count;
        };

        // The body plus the few slightly different shades on its anti-aliased edges in claude.png.
        add(BODY_COLOR, ENRAGED_BODY_COLOR);
        add(::Color { 190, 117, 83, 255 }, ENRAGED_BODY_COLOR);
        add(::Color { 182, 119, 83, 255 }, ENRAGED_BODY_COLOR);
        add(::Color { 196, 119, 100, 255 }, ENRAGED_BODY_COLOR);
        add(EYE_COLOR, ENRAGED_EYE_COLOR);
        return swap;
    }

    float ClaudeEnemy::getEnrageProgress() const
    {
        if (m_bossState == ClaudeState::Transform)
            return m_transformElapsed / TRANSFORM_DURATION;
        // The lives check covers a client that joined after the host already transformed.
        return (m_hasTransformed || m_lives * 2 <= m_maxLives) ? 1.f : 0.f;
    }

    void ClaudeEnemy::finishState()
    {
        switch (m_bossState)
        {
            case ClaudeState::Transform:
                throwBombs();
                setBossState(ClaudeState::Idle);
                return;
            case ClaudeState::BombLaunch:
                throwBombs();
                break;
            case ClaudeState::Dying:
                die();
                return;
            default:
                break;
        }
        rollNextState();
    }

    void ClaudeEnemy::die()
    {
        m_soundManager.playSound(SoundName::EnemyDie);
        if (m_world != nullptr)
            m_world->spawnBombRing(getPosition(), DEATH_BOMB_COUNT, DEATH_BOMB_RING_RADIUS, DEATH_BOMB_SLIDE, m_bombConfig.minFuseTime, m_bombConfig.maxFuseTime);

        destroy();
        if (m_killer != nullptr)
            ++m_killer->getUserData()->enemiesKilled;
        else if (m_remoteKillerId.has_value() && m_world != nullptr)
            m_world->creditRemoteKill(*this, *m_remoteKillerId);
        if (m_world != nullptr)
            m_world->clearBoss();
    }

    void ClaudeEnemy::rollNextState()
    {
        std::vector<ClaudeState> candidates;
        for (const ClaudeState state : { ClaudeState::Idle, ClaudeState::Walk, ClaudeState::Attack, ClaudeState::BombLaunch })
        {
            if (state == m_bossState || (state == ClaudeState::BombLaunch && !m_hasTransformed))
                continue;
            candidates.push_back(state);
        }
        setBossState(candidates[static_cast<size_t>(GetRandomValue(0, static_cast<int>(candidates.size()) - 1))]);
    }

    void ClaudeEnemy::throwBombs()
    {
        if (m_world != nullptr && isAuthority())
            m_world->spawnBombs(getPosition(), m_bombConfig.bombsPerPlayer, m_bombConfig.minFuseTime, m_bombConfig.maxFuseTime);
    }

    void ClaudeEnemy::setBossState(ClaudeState newState)
    {
        // Leaving the transform (host or client) locks in the enraged look and the faster rolls.
        if (m_bossState == ClaudeState::Transform && newState != ClaudeState::Transform && !m_hasTransformed)
        {
            m_hasTransformed = true;
            m_transformElapsed = TRANSFORM_DURATION;
            m_stateInterval *= ENRAGED_INTERVAL_FACTOR;
        }

        m_bossState = newState;
        m_stateTimer = m_stateInterval;
        switch (newState)
        {
            case ClaudeState::Idle:
                m_animation = m_idleAnimation;
                break;
            case ClaudeState::Transform:
                m_animation = m_idleAnimation;
                m_transformElapsed = 0.f;
                m_stateTimer = TRANSFORM_DURATION;
                break;
            case ClaudeState::BombLaunch:
                m_animation = m_attackAnimation;
                m_stateTimer = m_bombConfig.launchWindupTime;
                break;
            case ClaudeState::Dying:
                m_animation = m_idleAnimation;
                m_stateTimer = DEATH_DURATION;
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
        {
            startInvulnerability();
            // The hit happened on the host (or another peer): all that is known here is that it bled.
            emitBlood({ 0.f, 0.f }, state.lives <= 0 ? KILL_BLOOD_STRENGTH : 1.f);
        }
        m_lives = state.lives;
        m_targetPlayerId = state.targetPlayerId;

        if (state.aiState < CLAUDE_STATE_COUNT)
        {
            const ClaudeState hostState = static_cast<ClaudeState>(state.aiState);
            if (hostState != m_bossState)
                setBossState(hostState);
        }
    }

    void ClaudeEnemy::onNetworkDamage(int32_t damage, uint32_t attackerPlayerId)
    {
        // The attacker lives on a client, which gets the kill credit through the network instead.
        takeDamage(&m_soundManager, nullptr, false, damage, attackerPlayerId);
    }

    void ClaudeEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId)
    {
        // Already dead or dying: don't count the boss kill a second time.
        // Invulnerable while transforming, on every peer.
        if (shouldBeDestroyed() || m_bossState == ClaudeState::Transform || m_bossState == ClaudeState::Dying)
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
        emitBloodFromAttacker(attacker, fromLeft, m_lives <= 0 ? KILL_BLOOD_STRENGTH : 1.f);

        if (m_body != nullptr)
            m_body->ApplyLinearImpulseToCenter({fromLeft ? -50.f : 50.f, 0.f}, true);

        if (m_lives <= 0)
        {
            // Hold the death for a dramatic shake; die() credits whoever landed this blow.
            m_lives = 0;
            m_killer = attacker;
            m_remoteKillerId = remoteAttackerId;
            startInvulnerability();
            setBossState(ClaudeState::Dying);
        }
        else
        {
            startInvulnerability();
            if (!m_hasTransformed && m_lives * 2 <= m_maxLives)
                setBossState(ClaudeState::Transform);
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
