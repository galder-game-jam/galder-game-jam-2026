//
// Created by robin on 13.11.22.
//

#ifndef GAME_DEV_TEMPLATE_SNAKE_H
#define GAME_DEV_TEMPLATE_SNAKE_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"

namespace ggj
{

    class Snake : public PhysicsSprite
    {
        public:
            /*! Impulse pushing a player away on contact when the map gives no `knockback`. */
            static constexpr float DEFAULT_KNOCKBACK = 50.f;

            Snake() = default;
            Snake(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &animationManager, IMapper &mapper, ggj::ISoundManager &soundManager, b2Body *body,
                const raylib::Vector2 &physicsSize,
                const raylib::Vector2 &spriteSize,
                const raylib::Rectangle &drawingRect, raylib::Texture * texture, const UserData &userData, const raylib::Vector2 &velocity, float knockback, int lives = 1, bool isVisible = true)
                    : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager {animationManager}, m_mapper{mapper},
                      m_soundManager {soundManager}, m_velocity{velocity}, m_knockback{knockback}
            {
                m_lives = lives;
                m_body->SetGravityScale(1.f);
                m_body->SetFixedRotation(true);
                m_animation = m_animationManager.getAnimation(AnimationName::EnemySnakeMove);
                m_body->SetLinearVelocity({m_velocity.x, m_velocity.y});
                m_flip = m_velocity.x < 0.f;
            }

            void update(float timeDelta) override;

            void beginContact(PhysicsObject *a, PhysicsObject *b, b2Contact *contact) override;

        private:
            void turn();
            /*!
             * Sends the enemy along @p direction at its current speed (enemy_turn_up/down/left/right).
             *
             * @param direction Unit direction from EnemyTurn::DirectionFromCommand()
             */
            void turnTowards(const raylib::Vector2& direction);

            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &m_animationManager;
            IMapper &m_mapper;
            ggj::ISoundManager &m_soundManager;
            Animation m_animation;
            raylib::Vector2 m_velocity {0.f, 0.f};
            float m_knockback {DEFAULT_KNOCKBACK};
//            b2MassData m_massData{};

    };

} // ggj

#endif //GAME_DEV_TEMPLATE_SNAKE_H
