#include "MageBossEnemy.h"
#include "../projectiles/Projectile.h"
#include "../../system/World.h"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace ggj
{
    namespace
    {
        constexpr ::Color ICE_FLASH { 130, 200, 255, 255 };
        constexpr ::Color FIRE_FLASH { 255, 150, 60, 255 };
        constexpr ::Color CRYSTAL_FLASH { 255, 120, 210, 255 };
        constexpr float TWO_PI = 2.f * std::numbers::pi_v<float>;

        /*! Whether a flash that speeds up from @p slow to @p fast over a state is on its coloured half right now. */
        bool isFlashOn(const float time, const float slow, const float fast, const float progress)
        {
            return static_cast<int32_t>(time * (slow + (fast - slow) * progress)) % 2 == 0;
        }
    } // namespace

    float MageBossEnemy::randomRange(const float min, const float max)
    {
        constexpr int32_t STEPS = 1000;
        return min + (max - min) * static_cast<float>(GetRandomValue(0, STEPS)) / static_cast<float>(STEPS);
    }

    void MageBossEnemy::move(const raylib::Vector2& toMove) {}

    bool MageBossEnemy::isAuthority() const
    {
        return m_world == nullptr || m_world->isEnemyAuthority();
    }

    void MageBossEnemy::pickTarget()
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

    std::optional<raylib::Vector2> MageBossEnemy::targetPosition() const
    {
        if (m_world == nullptr || !m_targetPlayerId.has_value())
            return std::nullopt;
        return m_world->getPlayerCenter(*m_targetPlayerId);
    }

    raylib::Vector2 MageBossEnemy::directionToTarget() const
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

    void MageBossEnemy::faceTowards(const raylib::Vector2& direction)
    {
        if (direction.x != 0.f)
            m_flip = direction.x < 0.f;
    }

    float MageBossEnemy::getStateProgress() const
    {
        return m_stateDuration > 0.f ? 1.f - std::max(0.f, m_stateTimer) / m_stateDuration : 1.f;
    }

    float MageBossEnemy::crystalBurstInterval() const
    {
        return m_config.crystalBurstInterval * (isEnraged() ? ENRAGED_BURST_INTERVAL_FACTOR : 1.f);
    }

    MageState MageBossEnemy::ChooseAttack(float roll, bool isEnraged)
    {
        const float iceChance = isEnraged ? ENRAGED_ICE_ATTACK_CHANCE : ICE_ATTACK_CHANCE;
        const float fireChance = isEnraged ? ENRAGED_FIRE_ATTACK_CHANCE : FIRE_ATTACK_CHANCE;
        const float crystalChance = isEnraged ? ENRAGED_CRYSTAL_ATTACK_CHANCE : CRYSTAL_ATTACK_CHANCE;

        if (roll < iceChance)
            return MageState::IceCast;
        if (roll < iceChance + fireChance)
            return MageState::FireCast;
        if (roll < iceChance + fireChance + crystalChance)
            return MageState::CrystalWindup;
        // Sometimes the mage just stops to turn a page.
        return MageState::Idle;
    }

    int32_t MageBossEnemy::RollIceCircleCount(const MageBossConfig& config, float roll)
    {
        const int32_t span = config.maxIceCircles - config.minIceCircles + 1;
        const int32_t offset = static_cast<int32_t>(std::clamp(roll, 0.f, 1.f) * static_cast<float>(span));
        // A roll of exactly 1 would land one past the end.
        return config.minIceCircles + std::min(offset, span - 1);
    }

    int32_t MageBossEnemy::FireLineCount(bool isEnraged)
    {
        return isEnraged ? ENRAGED_FIRE_LINES : FIRE_LINES;
    }

    std::vector<float> MageBossEnemy::FireLineAngles(float startAngle, int32_t count)
    {
        std::vector<float> angles;
        if (count <= 0)
            return angles;

        angles.reserve(static_cast<size_t>(count));
        for (int32_t i = 0; i < count; ++i)
            angles.push_back(startAngle + TWO_PI * static_cast<float>(i) / static_cast<float>(count));
        return angles;
    }

    EnemyNetworkState MageBossEnemy::getNetworkState() const
    {
        EnemyNetworkState state = PhysicsSprite::getNetworkState();
        state.aiState = static_cast<uint8_t>(m_state);
        state.targetPlayerId = m_targetPlayerId.value_or(0);
        return state;
    }

    void MageBossEnemy::applyNetworkState(const EnemyNetworkState& state)
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

        if (state.aiState < MAGE_STATE_COUNT)
        {
            const MageState hostState = static_cast<MageState>(state.aiState);
            if (hostState != m_state)
                setState(hostState);
        }
    }

    void MageBossEnemy::castIce()
    {
        // Clients get the host's circles over the network instead of rolling their own.
        if (m_world == nullptr || !isAuthority())
            return;

        const int32_t count = RollIceCircleCount(m_config, randomRange(0.f, 1.f));
        m_world->spawnIceCircles(count, m_config.iceCircleRadius, m_config.iceMinDelay, m_config.iceMaxDelay, m_config.freezeSeconds);
    }

    void MageBossEnemy::castFire(float warningSeconds)
    {
        if (m_world == nullptr || !isAuthority())
            return;

        const std::vector<float> angles = FireLineAngles(randomRange(0.f, TWO_PI), FireLineCount(isEnraged()));
        // The fire starts from the middle of where the mage stands, which is also where he is rooted to.
        m_world->spawnFireLines(getPosition(), angles, warningSeconds, m_config.fireBurnSeconds, m_config.fireSpeed);
    }

    void MageBossEnemy::fireCrystalBurst()
    {
        if (m_world == nullptr || !isAuthority())
            return;

        // The victim left during the windup: pick someone else, or nobody is left to shoot at.
        if (!targetPosition().has_value())
            pickTarget();
        const std::optional<raylib::Vector2> target = targetPosition();
        if (!target.has_value())
            return;

        const raylib::Vector2 myPos = getPosition();
        const float aimAngle = std::atan2(target->y - myPos.y, target->x - myPos.x);
        const int32_t count = isEnraged() ? m_config.enragedCrystalsPerBurst : m_config.crystalsPerBurst;
        m_world->spawnCrystalFan(myPos, aimAngle, count, CRYSTAL_SPREAD_DEGREES, m_config.crystalSpeed);
    }

    void MageBossEnemy::die()
    {
        if (m_world != nullptr)
            m_world->spawnCrystalRing(getPosition(), DEATH_CRYSTAL_COUNT, randomRange(0.f, TWO_PI), m_config.crystalSpeed, CRYSTAL_RING_RADIUS);

        m_soundManager.playSound(SoundName::EnemyDie);
        destroy();
        if (m_killer != nullptr)
            m_killer->getUserData()->enemiesKilled++;
        else if (m_remoteKillerId.has_value() && m_world != nullptr)
            m_world->creditRemoteKill(*this, *m_remoteKillerId);
        if (m_world != nullptr)
            m_world->clearBoss();
    }

    void MageBossEnemy::setState(MageState newState)
    {
        m_state = newState;

        // A fresh cycle picks who to chase; the crystal windup picks its victim, so it is anyone, not just the last one chased.
        if (isAuthority() && (newState == MageState::Walk || newState == MageState::CrystalWindup || !m_targetPlayerId.has_value()))
            pickTarget();

        switch (newState)
        {
            case MageState::Idle:
                m_stateTimer = isEnraged() ? ENRAGED_IDLE_DURATION : IDLE_DURATION;
                m_tint = WHITE;
                break;
            case MageState::Walk:
                m_stateTimer = randomRange(WALK_MIN_DURATION, WALK_MAX_DURATION);
                m_tint = WHITE;
                break;
            case MageState::IceCast:
                // The circles appear right away: they are their own telegraph, and the mage hovers until they have sprung.
                m_stateTimer = m_config.iceMaxDelay + ICE_CAST_LINGER;
                m_tint = ICE_FLASH;
                castIce();
                break;
            case MageState::FireCast:
            {
                // The lines appear right away as outlines and ignite when the warning ends.
                const float warning = isEnraged() ? m_config.enragedFireWarningSeconds : m_config.fireWarningSeconds;
                m_stateTimer = warning + FIRE_CAST_AFTERGLOW;
                m_tint = FIRE_FLASH;
                castFire(warning);
                break;
            }
            case MageState::CrystalWindup:
                m_stateTimer = isEnraged() ? ENRAGED_CRYSTAL_WINDUP_DURATION : CRYSTAL_WINDUP_DURATION;
                m_tint = CRYSTAL_FLASH;
                break;
            case MageState::CrystalBursts:
                m_burstsLeft = CRYSTAL_BURSTS;
                m_burstTimer = 0.f; // The first burst goes off right away.
                m_stateTimer = static_cast<float>(CRYSTAL_BURSTS) * crystalBurstInterval() + CRYSTAL_FINAL_LINGER;
                m_tint = CRYSTAL_FLASH;
                break;
            case MageState::Recover:
                m_stateTimer = RECOVER_DURATION;
                m_tint = WHITE;
                break;
            case MageState::Dying:
                m_stateTimer = DYING_DURATION;
                m_tint = WHITE;
                m_invulTimer = 0.f;
                m_isVisible = true;
                break;
        }

        m_stateDuration = m_stateTimer;
    }

    void MageBossEnemy::update(float timeDelta)
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

        // Derived from lives, so a client that joins after the enrage still sees the mage red.
        const float tintStep = timeDelta / ENRAGE_TINT_FADE_DURATION;
        m_enrageTint = std::clamp(m_enrageTint + (isEnraged() ? tintStep : -tintStep), 0.f, 1.f);

        b2Vec2 velocity { 0.f, 0.f };

        switch (m_state)
        {
            case MageState::Idle:
                faceTowards(directionToTarget());
                if (canAdvance)
                    setState(MageState::Walk);
                break;

            case MageState::Walk:
            {
                // Drift round pillars and furniture instead of pushing into them.
                const raylib::Vector2 direction = m_steering.steer(*m_body, directionToTarget());
                faceTowards(direction);
                const float speed = m_followSpeed * (isEnraged() ? ENRAGED_SPEED_MULTIPLIER : 1.f);
                velocity = { direction.x * speed, direction.y * speed };
                if (canAdvance)
                    setState(ChooseAttack(randomRange(0.f, 1.f), isEnraged()));
                break;
            }

            case MageState::IceCast:
                faceTowards(directionToTarget());
                m_tint = isFlashOn(m_animTime, 8.f, 20.f, getStateProgress()) ? ICE_FLASH : WHITE;
                if (canAdvance)
                    setState(MageState::Recover);
                break;

            case MageState::FireCast:
                // Rooted: the lines were shot from this very spot.
                m_tint = isFlashOn(m_animTime, 8.f, 24.f, getStateProgress()) ? FIRE_FLASH : WHITE;
                if (canAdvance)
                    setState(MageState::Recover);
                break;

            case MageState::CrystalWindup:
                // Keep tracking the victim while charging so the telegraph points at them.
                faceTowards(directionToTarget());
                m_tint = isFlashOn(m_animTime, 8.f, 20.f, getStateProgress()) ? CRYSTAL_FLASH : WHITE;
                if (canAdvance)
                    setState(MageState::CrystalBursts);
                break;

            case MageState::CrystalBursts:
                faceTowards(directionToTarget());
                m_burstTimer -= timeDelta;
                if (isAuthority() && m_burstsLeft > 0 && m_burstTimer <= 0.f)
                {
                    fireCrystalBurst();
                    --m_burstsLeft;
                    m_burstTimer = crystalBurstInterval();
                }
                if (canAdvance)
                    setState(MageState::Recover);
                break;

            case MageState::Recover:
                if (canAdvance)
                    setState(MageState::Idle);
                break;

            case MageState::Dying:
                // Flicker between pink and white, faster as the end nears.
                m_tint = isFlashOn(m_animTime, 6.f, 16.f, getStateProgress()) ? CRYSTAL_FLASH : WHITE;
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

    void MageBossEnemy::draw()
    {
        if (!m_isVisible || m_texture == nullptr)
        {
            PhysicsSprite::draw();
            return;
        }

        // The sprite sheet is smaller than the boss, so scale the frame up to the sprite size from the map
        // (PhysicsSprite::draw would use the frame size as-is).
        const MagePose pose = ComputePose(m_state, getStateProgress(), m_animTime);

        // A shadow on the floor under his feet, shrinking the higher he hovers.
        const float feetY = m_position.y + m_size.y;
        const float shadowScale = 1.f - 0.35f * std::clamp(pose.lift / HOVER_HEIGHT, 0.f, 1.f);
        DrawEllipse(static_cast<int32_t>(m_position.x), static_cast<int32_t>(feetY), m_spriteSize.x * 0.2f * shadowScale, m_spriteSize.x * 0.06f * shadowScale,
                    ::Color { 0, 0, 0, 90 });

        const float sourceWidth = m_flip ? -m_drawingRect.width : m_drawingRect.width;
        const raylib::Rectangle source { m_drawingRect.x, m_drawingRect.y, sourceWidth, m_drawingRect.height };
        const raylib::Rectangle destination { m_position.x, m_position.y - pose.lift, m_spriteSize.x * pose.scaleX, m_spriteSize.y * pose.scaleY };
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

    MageBossEnemy::MagePose MageBossEnemy::ComputePose(MageState state, float progress, float time)
    {
        MagePose pose;
        const float p = std::clamp(progress, 0.f, 1.f);
        // Rises quickly at the start of a spell, hangs there and settles at the end.
        const float rise = std::min(p * 5.f, 1.f) * std::min((1.f - p) * 5.f, 1.f);

        switch (state)
        {
            case MageState::Idle:
            {
                // A slow hover: the robe swells a touch as he bobs.
                const float bob = std::sin(time * 2.f);
                pose.lift = 2.f + 2.f * bob;
                pose.scaleY = 1.f + 0.015f * bob;
                pose.rotation = 1.f * std::sin(time * 1.1f);
                break;
            }

            case MageState::Walk:
            {
                // Drifts rather than walks: a steady bob and a slight lean into the direction of travel.
                const float bob = std::sin(time * 4.f);
                pose.lift = 3.f + 2.5f * bob;
                pose.rotation = 3.f + 1.5f * bob;
                break;
            }

            case MageState::IceCast:
            {
                // Rises with arms spread wide, shivering.
                pose.lift = HOVER_HEIGHT * rise;
                pose.scaleY = 1.f + 0.06f * rise;
                pose.scaleX = 1.f - 0.03f * rise;
                pose.rotation = 1.5f * std::sin(time * 40.f) * rise;
                break;
            }

            case MageState::FireCast:
            {
                // Rises and rears back, trembling harder as the fire is about to ignite.
                pose.lift = HOVER_HEIGHT * rise;
                pose.rotation = -(2.f + 8.f * p) + (1.f + 3.f * p) * std::sin(time * 55.f);
                pose.scaleY = 1.f + 0.07f * rise;
                pose.scaleX = 1.f - 0.04f * rise;
                break;
            }

            case MageState::CrystalWindup:
            {
                // Gathers himself: hunches slightly while the crystals charge.
                pose.lift = HOVER_HEIGHT * 0.5f * rise;
                pose.scaleY = 1.f - 0.06f * p;
                pose.scaleX = 1.f + 0.05f * p;
                pose.rotation = 2.f * std::sin(time * 30.f);
                break;
            }

            case MageState::CrystalBursts:
            {
                // Thrusts forward on every burst.
                pose.lift = HOVER_HEIGHT * 0.5f;
                pose.rotation = 6.f + 4.f * std::sin(time * 12.f);
                pose.scaleX = 1.f + 0.03f * std::sin(time * 12.f);
                break;
            }

            case MageState::Recover:
            {
                // Settles back down, a little out of breath.
                pose.lift = HOVER_HEIGHT * 0.5f * (1.f - p);
                pose.rotation = 5.f * (1.f - p);
                pose.scaleY = 1.f - 0.04f * (1.f - p);
                break;
            }

            case MageState::Dying:
            {
                // Convulses and swells like he is about to burst, shaking harder the closer the end gets.
                pose.lift = HOVER_HEIGHT * p;
                pose.scaleY = 1.f + 0.18f * p;
                pose.scaleX = 1.f - 0.05f * p;
                pose.rotation = (3.f + 12.f * p) * std::sin(time * 70.f);
                break;
            }
        }

        return pose;
    }

    void MageBossEnemy::startInvulnerability()
    {
        m_invulTimer = INVUL_DURATION;
        m_flashTimer = 0.f;
    }

    void MageBossEnemy::onNetworkDamage(int32_t damage, uint32_t attackerPlayerId)
    {
        // The attacker lives on a client, which gets the kill credit through the network instead.
        takeDamage(&m_soundManager, nullptr, false, damage, attackerPlayerId);
    }

    void MageBossEnemy::takeDamage(ISoundManager* soundManager, PhysicsObject* attacker, bool fromLeft, int32_t damage, std::optional<uint32_t> remoteAttackerId)
    {
        // Already dead (waiting for removal): don't count the boss kill a second time.
        if (shouldBeDestroyed() || m_state == MageState::Dying)
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
            setState(MageState::Dying);
        }
        else
            startInvulnerability();
    }

    void MageBossEnemy::onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker, int32_t damage)
    {
        takeDamage(soundManager, attacker, fromLeft, damage);
    }

    void MageBossEnemy::beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact)
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
