//
// Created by soberga on 12.11.22.
//

#ifndef GAME_DEV_TEMPLATE_GHOST_H
#define GAME_DEV_TEMPLATE_GHOST_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../interfaces/game/IPlayerLocator.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"
#include <optional>

namespace ggj
{
    /*!
     * Floats in place until a player comes within its follow range, then chases that player
     * until they get away. Map properties: follow_range (pixels) and follow_speed; with either
     * left at 0 the ghost never moves.
     */
    class Ghost : public PhysicsSprite
    {
        public:
            Ghost() = default;
            Ghost(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &animationManager, IMapper &mapper, ggj::ISoundManager &soundManager, b2Body *body,
            const raylib::Vector2 &physicsSize,
            const raylib::Vector2 &spriteSize,
            const raylib::Rectangle &drawingRect, raylib::Texture * texture, const UserData &userData, float followRange, float followSpeed, IPlayerLocator* players,
            int lives = 1, bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager {animationManager}, m_mapper{mapper},
              m_soundManager {soundManager}, m_followRange{followRange}, m_followSpeed{followSpeed}, m_players{players}
            {
                m_lives = lives;
                // The map only gives a 16x16 box at the feet; cover the visible cloud-mate instead.
                resizeHitboxKeepingBottom(raylib::Vector2(26.f, 43.f));
                m_body->SetFixedRotation(true);
                m_animation = m_animationManager.getAnimation(AnimationName::EnemyGhostFly);
                m_body->SetLinearVelocity({0.f, 0.f});
            }
            void move(const raylib::Vector2 &toMove) override;

            void update(float timeDelta) override;

            void beginContact(PhysicsObject *a, PhysicsObject *b, b2Contact *contact) override;

            /*!
             * Decides which player the ghost follows. It keeps its current target while that player
             * stays in range; otherwise it takes the first player (lowest id) found in range.
             *
             * @param currentTarget Player being followed now, if any
             * @param ghostPosition Ghost position in pixels
             * @param followRange Follow range in pixels; 0 or less never follows anyone
             * @param players Every player in the session
             * @return The player to follow, or nothing if no player is in range
             */
            [[nodiscard]] static std::optional<uint32_t> PickTarget(std::optional<uint32_t> currentTarget, const raylib::Vector2& ghostPosition, float followRange,
                                                                    const IPlayerLocator& players);

        private:
            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &m_animationManager;
            IMapper &m_mapper;
            ggj::ISoundManager &m_soundManager;
            Animation m_animation;
            float m_followRange { 0.f }; ///< Pixels.
            float m_followSpeed { 0.f };
            IPlayerLocator* m_players { nullptr };
            std::optional<uint32_t> m_targetPlayerId {}; ///< Network player id being followed.
    };
} // ggj
#endif //GAME_DEV_TEMPLATE_GHOST_H
