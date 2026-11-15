#include "ButcherEnemy.h"
#include "../projectiles/Projectile.h"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

namespace ggj
{
    float ButcherEnemy::randomRange(const float min, const float max)
    {
        constexpr int32_t STEPS = 1000;
        return min + (max - min) * static_cast<float>(GetRandomValue(0, STEPS)) / static_cast<float>(STEPS);
    }

    void ButcherEnemy::move(const raylib::Vector2& toMove) {}

    bool ButcherEnemy::isAuthority() const
    {
        return m_world == nullptr || m_world->isEnemyAuthority();
    }

    void ButcherEnemy::pickTarget()
    {
        std::vector<uint32_t> playerIds;
        if (m_world != nullptr)
            playerIds = m_world->getActivePlayerIds();
        if (playerIds.empty())
        {
            m_targetPlayerId.reset();
            return;
        }

        // Rotate to someone new: drop the previous target from the pool unless they are the only one left.
        if (m_targetPlayerId.has_value() && playerIds.size() > 1)
            std::erase(playerIds, *m_targetPlayerId);

        m_targetPlayerId = playerIds[static_cast<size_t>(GetRandomValue(0, static_cast<int>(playerIds.size()) - 1))];
    }

    std::optional<raylib::Vector2> ButcherEnemy::targetPosition() const
    {
        if (m_world == nullptr || !m_targetPlayerId.has_value())
            return std::nullopt;
        return m_world->getPlayerCenter(*m_targetPlayerId);
    }

    raylib::Vector2 ButcherEnemy::directionToTarget() const
    {
        const std::optional<raylib::Vector2> target = targetPosition();
        if (!target.has_value())
            return { 0.f, 0.f };

        const raylib::Vector2 myPos = getPosition();
        const float dx = target->x - myPos.x;
        const float dy = target->y - myPos.y;
        const float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < 1.f)
            return { 0.f, 0.f };

        return { dx / dist, dy / dist };
    }

    void ButcherEnemy::faceTowards(const raylib::Vector2& direction)
    {
        if (direction.x != 0.f)
            m_flip = direction.x < 0.f;
    }

    float ButcherEnemy::getStateProgress() const
    {
        return m_stateDuration > 0.f ? 1.f - std::max(0.f, m_stateTimer) / m_stateDuration : 1.f;
    }

    ButcherState ButcherEnemy::ChooseAttack(float roll, bool isEnraged)
    {
        const float cleaverChance = isEnraged ? ENRAGED_CLEAVER_ATTACK_CHANCE : CLEAVER_ATTACK_CHANCE;
        const float spiderChance = isEnraged ? ENRAGED_SPIDER_ATTACK_CHANCE : SPIDER_ATTACK_CHANCE;

        if (roll < cleaverChance)
            return ButcherState::CleaverWindup;
        if (roll < cleaverChance + spiderChance)
            return ButcherState::SpiderWindup;
        // Sometimes the butcher just stops to wipe his cleaver.
        return ButcherState::Idle;
    }

    float ButcherEnemy::NextRingStartAngle(float previousStartAngle, int32_t cleaverCount)
    {
        if (cleaverCount <= 0)
            return previousStartAngle;
        return previousStartAngle + std::numbers::pi_v<float> / static_cast<float>(cleaverCount);
    }

    EnemyNetworkState ButcherEnemy::getNetworkState() const
    {
        EnemyNetworkState state = PhysicsSprite::getNetworkState();
        state.aiState = static_cast<uint8_t>(m_state);
        state.targetPlayerId = m_targetPlayerId.value_or(0);
        return state;
    }

    void ButcherEnemy::applyNetworkState(const EnemyNetworkState& state)
    {
        PhysicsSprite::applyNetworkState(state);
        // The host owns the boss's health and attack cycle. Mirror both so every peer shows the same HP,
        // enrage tint, telegraph colour and target.
        if (state.lives < m_lives && m_invulTimer <= 0.f)
        {
            startInvulnerability();
            // The hit happened on the host (or another peer): all that is known here is that it bled.
            emitBlood({ 0.f, 0.f }, state.lives <= 0 ? KILL_BLOOD_STRENGTH : 1.f);
        }
        m_lives = state.lives;
        m_targetPlayerId = state.targetPlayerId;

        if (state.aiState < BUTCHER_STATE_COUNT)
        {
            const ButcherState hostState = static_cast<ButcherState>(state.aiState);
            if (hostState != m_state)
                setState(hostState);
        }
    }

    void ButcherEnemy::throwCleavers(float startAngle)
    {
        // Clients get the host's cleavers over the network instead of rolling their own.
        if (m_world == nullptr || !isAuthority())
            return;

        const int32_t count = isEnraged() ? m_config.enragedCleaverCount : m_config.normalCleaverCount;
        m_world->spawnCleaverRing(getPosition(), count, startAngle, m_config.cleaverSpeed, CLEAVER_RING_RADIUS);
    }

    void ButcherEnemy::throwSpider()
    {
        if (m_world == nullptr || !isAuthority())
            return;

        // One spider per player, all thrown at once, each landing where its victim stands right now.
        for (const raylib::Vector2& target : m_world->getPlayerCenters())
            m_world->spawnThrownSpider(getPosition(), target, m_config.spiderFlightTime);
    }

    void ButcherEnemy::die()
    {
        if (m_world != nullptr)
            m_world->spawnCleaverRing(getPosition(), m_config.deathCleaverCount, randomRange(0.f, 2.f * std::numbers::pi_v<float>), m_config.cleaverSpeed, CLEAVER_RING_RADIUS);

        m_soundManager.playSound(SoundName::EnemyDie);
        destroy();
        if (m_killer != nullptr)
            m_killer->getUserData()->enemiesKilled++;
        else if (m_remoteKillerId.has_value() && m_world != nullptr)
            m_world->creditRemoteKill(*this, *m_remoteKillerId);
        if (m_world != nullptr)
            m_world->clearBoss();
    }

    void ButcherEnemy::setState(ButcherState newState)
    {
        const ButcherState previousState = m_state;
        m_state = newState;

        // A fresh cycle picks who to chase.
        if (isAuthority() && (newState == ButcherState::Walk || !m_targetPlayerId.has_value()))
            pickTarget();

        switch (newState)
        {
            case ButcherState::Idle:
                m_stateTimer = isEnraged() ? ENRAGED_IDLE_DURATION : IDLE_DURATION;
                m_tint = WHITE;
                break;
            case ButcherState::Walk:
                m_stateTimer = randomRange(WALK_MIN_DURATION, WALK_MAX_DURATION);
                m_tint = WHITE;
                break;
            case ButcherState::CleaverWindup:
            {
                // A ring chained after the last one (enraged) starts quickly; a fresh attack takes its time.
                const bool isChained = previousState == ButcherState::Recover && m_volleysLeft > 0;
                if (!isChained)
                {
                    m_volleysLeft = isEnraged() ? ENRAGED_VOLLEYS : 1;
                    m_ringsThrown = 0;
                }
                m_stateTimer = isChained ? CHAINED_WINDUP_DURATION : (isEnraged() ? ENRAGED_CLEAVER_WINDUP_DURATION : CLEAVER_WINDUP_DURATION);
                m_tint = RED;
                break;
            }
            case ButcherState::SpiderWindup:
                m_volleysLeft = 0;
                m_stateTimer = isEnraged() ? ENRAGED_SPIDER_WINDUP_DURATION : SPIDER_WINDUP_DURATION;
                m_tint = raylib::Color { 140, 255, 140, 255 };
                break;
            case ButcherState::Recover:
                m_stateTimer = RECOVER_DURATION;
                m_tint = WHITE;
                break;
            case ButcherState::Dying:
                m_stateTimer = DYING_DURATION;
                m_tint = WHITE;
                m_invulTimer = 0.f;
                m_isVisible = true;
                break;
        }

        m_stateDuration = m_stateTimer;
    }

    void ButcherEnemy::update(float timeDelta)
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
                m_isVisible = true;
            }
        }

        m_stateTimer -= timeDelta;
        m_animTime += timeDelta;
        // Only the host runs the attack cycle; clients are moved along by applyNetworkState().
        const bool canAdvance = isAuthority() && m_stateTimer <= 0.f;

        // Derived from lives, so a client that joins after the enrage still sees the butcher red.
        const float tintStep = timeDelta / ENRAGE_TINT_FADE_DURATION;
        m_enrageTint = std::clamp(m_enrageTint + (isEnraged() ? tintStep : -tintStep), 0.f, 1.f);

        b2Vec2 velocity { 0.f, 0.f };

        switch (m_state)
        {
            case ButcherState::Idle:
                faceTowards(directionToTarget());
                if (canAdvance)
                    setState(ButcherState::Walk);
                break;

            case ButcherState::Walk:
            {
                // Walk round pillars and furniture instead of pushing into them.
                const raylib::Vector2 direction = m_steering.steer(*m_body, directionToTarget());
                faceTowards(direction);
                const float speed = m_followSpeed * (isEnraged() ? ENRAGED_SPEED_MULTIPLIER : 1.f);
                velocity = { direction.x * speed, direction.y * speed };
                if (canAdvance)
                    setState(ChooseAttack(randomRange(0.f, 1.f), isEnraged()));
                break;
            }

            case ButcherState::CleaverWindup:
                // Keep tracking the player while rooted so the telegraph points at them.
                faceTowards(directionToTarget());
                // Flash red, faster as the throw nears, so the volley can be seen coming.
                m_tint = (static_cast<int32_t>(m_animTime * (8.f + 12.f * getStateProgress())) % 2 == 0) ? RED : WHITE;
                if (canAdvance)
                {
                    const int32_t count = isEnraged() ? m_config.enragedCleaverCount : m_config.normalCleaverCount;
                    const float startAngle = (m_ringsThrown == 0) ? randomRange(0.f, 2.f * std::numbers::pi_v<float>) : NextRingStartAngle(m_lastRingAngle, count);
                    m_lastRingAngle = startAngle;
                    ++m_ringsThrown;
                    --m_volleysLeft;
                    throwCleavers(startAngle);
                    setState(ButcherState::Recover);
                }
                break;

            case ButcherState::SpiderWindup:
                faceTowards(directionToTarget());
                if (canAdvance)
                {
                    throwSpider();
                    setState(ButcherState::Recover);
                }
                break;

            case ButcherState::Recover:
                if (canAdvance)
                    setState(m_volleysLeft > 0 ? ButcherState::CleaverWindup : ButcherState::Idle);
                break;

            case ButcherState::Dying:
                // Flash between sickly red and white, faster as the end nears.
                m_tint = (static_cast<int32_t>(m_animTime * (6.f + 10.f * getStateProgress())) % 2 == 0) ? RED : WHITE;
                if (canAdvance)
                {
                    die();
                    return;
                }
                break;
        }

        m_body->SetLinearVelocity(velocity);

        PhysicsObject::update(timeDelta);

        if (m_animation.isValid())
        {
            m_animation.update(timeDelta);
            m_drawingRect = m_animation.getDrawingRect();
        }
    }

    void ButcherEnemy::draw()
    {
        if (!m_isVisible || m_texture == nullptr)
        {
            PhysicsSprite::draw();
            return;
        }

        // The sprite sheet is smaller than the boss, so scale the frame up to the sprite size from the map
        // (PhysicsSprite::draw would use the frame size as-is).
        const ButcherPose pose = ComputePose(m_state, getStateProgress(), m_animTime);
        const float sourceWidth = m_flip ? -m_drawingRect.width : m_drawingRect.width;
        const raylib::Rectangle source { m_drawingRect.x, m_drawingRect.y, sourceWidth, m_drawingRect.height };
        const raylib::Rectangle destination { m_position.x, m_position.y, m_spriteSize.x * pose.scaleX, m_spriteSize.y * pose.scaleY };
        // The origin scales with the sprite so the feet stay on the same spot while squashing and stretching.
        const raylib::Vector2 origin { m_origin.x * pose.scaleX, m_origin.y * pose.scaleY };
        // Leaning "forward" means towards the side he faces.
        const float lean = m_flip ? -pose.rotation : pose.rotation;
        if (m_enrageTint > 0.f)
            m_tintShader.begin(ENRAGE_TINT_COLOR, m_enrageTint * ENRAGE_TINT_STRENGTH);
        m_texture->Draw(source, destination, origin, m_rotation + lean, m_tint);
        if (m_enrageTint > 0.f)
            m_tintShader.end();
    }

    ButcherEnemy::ButcherPose ButcherEnemy::ComputePose(ButcherState state, float progress, float time)
    {
        ButcherPose pose;
        const float p = std::clamp(progress, 0.f, 1.f);

        switch (state)
        {
            case ButcherState::Idle:
            {
                // Slow breathing: the chest swells while he sways a touch.
                const float breath = std::sin(time * 2.2f);
                pose.scaleY = 1.f + 0.025f * breath;
                pose.scaleX = 1.f - 0.012f * breath;
                pose.rotation = 1.2f * std::sin(time * 1.1f);
                break;
            }

            case ButcherState::Walk:
            {
                // Heavy waddle: rock from foot to foot, dipping a little on every step.
                const float phase = time * WALK_STEPS_PER_SECOND * std::numbers::pi_v<float>;
                pose.rotation = 5.f * std::sin(phase);
                pose.scaleY = 1.f - 0.045f * std::abs(std::sin(phase));
                pose.scaleX = 1.f + 0.03f * std::abs(std::sin(phase));
                break;
            }

            case ButcherState::CleaverWindup:
            {
                // Rear back with the cleaver held high, trembling harder the closer the throw gets.
                pose.rotation = -(3.f + 12.f * p) + (1.f + 3.f * p) * std::sin(time * 55.f);
                pose.scaleY = 1.f + 0.08f * p;
                pose.scaleX = 1.f - 0.04f * p;
                break;
            }

            case ButcherState::SpiderWindup:
            {
                // Hunch down over the spider in his hand, quivering with glee.
                pose.scaleY = 1.f - 0.12f * p;
                pose.scaleX = 1.f + 0.08f * p;
                pose.rotation = 2.f * std::sin(time * 30.f);
                break;
            }

            case ButcherState::Recover:
            {
                // Follow-through: lunge forward from the throw and settle back.
                const float lunge = 1.f - p;
                pose.rotation = 12.f * lunge;
                pose.scaleX = 1.f + 0.1f * lunge;
                pose.scaleY = 1.f - 0.06f * lunge;
                break;
            }

            case ButcherState::Dying:
            {
                // Convulse and swell like he is about to burst, shaking harder the closer the end gets.
                pose.scaleY = 1.f + 0.2f * p;
                pose.scaleX = 1.f - 0.06f * p;
                pose.rotation = (3.f + 12.f * p) * std::sin(time * 70.f);
                break;
            }
        }

        return pose;
    }

    void ButcherEnemy::startInvulnerability()
    {
        m_invulTimer = INVUL_DURATION;
        m_flashTimer = 0.f;
    }

    void ButcherEnemy::onNetworkDamage(int32_t damage, uint32_t attackerPlayerId)
    {
        // The attacker lives on a client, which gets the kill credit through the network instead.
        takeDamage(&m_soundManager, nullptr, false, damage, attackerPlayerId);
    }

    void ButcherEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId)
    {
        // Already dead (waiting for removal): don't count the boss kill a second time.
        if (shouldBeDestroyed() || m_state == ButcherState::Dying)
            return;

        // The host owns the boss's health. Send the hit there; the lives it syncs back drive the
        // HP bar, the enrage tint and the hit flash, and it removes the boss everywhere when it dies.
        if (!isAuthority())
        {
            if (m_world != nullptr)
                m_world->reportEnemyDamage(*this, damage);
            return;
        }

        if (m_invulTimer > 0.f)
            return;

        m_lives -= damage;
        emitBloodFromAttacker(attacker, fromLeft, m_lives <= 0 ? KILL_BLOOD_STRENGTH : 1.f);

        if (m_lives <= 0)
        {
            // Hold the death for a couple of seconds of convulsing; die() credits whoever landed this blow.
            m_lives = 0;
            m_killer = attacker;
            m_remoteKillerId = remoteAttackerId;
            setState(ButcherState::Dying);
        }
        else
            startInvulnerability();
    }

    void ButcherEnemy::onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage)
    {
        takeDamage(soundManager, attacker, fromLeft, damage);
    }

    void ButcherEnemy::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
    {
        UserData* userDataB = b->getUserData();

        if (userDataB->getObjectType() == ObjectType::Player)
        {
            const raylib::Vector2 myPos = getPosition();
            const raylib::Vector2 playerPos = b->getPosition();
            const float dx = playerPos.x - myPos.x;
            const float dy = playerPos.y - myPos.y;
            const float len = std::sqrt(dx * dx + dy * dy);

            if (len > 0.001f)
                b->getBody()->ApplyLinearImpulseToCenter({ (dx / len) * m_knockback, (dy / len) * m_knockback }, true);
        }
        else if (userDataB->getObjectType() == ObjectType::Projectile)
        {
            Projectile* p = dynamic_cast<Projectile*>(b);
            p->explode();
            takeDamage(&m_soundManager, p->getOwner(), false, PROJECTILE_DAMAGE);
        }
    }

} // namespace ggj
