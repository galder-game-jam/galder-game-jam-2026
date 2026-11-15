//
// Created by soberga on 12.11.22.
//

#ifndef GAME_DEV_TEMPLATE_BAT_H
#define GAME_DEV_TEMPLATE_BAT_H

#include "../../graphics/PhysicsSprite.h"
#include "../../managers/AnimationManager.h"
#include "../../interfaces/system/IMapper.h"
#include "../../../gamedev-base/interfaces/managers/ISoundManager.h"
#include "../../enums/SoundName.h"

namespace ggj
{

    class Bat : public PhysicsSprite
    {
        public:
            Bat() = default;
            Bat(ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &animationManager, IMapper &mapper, ggj::ISoundManager &soundManager, b2Body *body,
            const raylib::Vector2 &physicsSize,
            const raylib::Vector2 &spriteSize,
            const raylib::Rectangle &drawingRect, raylib::Texture * texture, const UserData &userData, const raylib::Vector2 &velocity, bool isFacingLeft, int lives = 1, bool isVisible = true)
            : PhysicsSprite(body, physicsSize, spriteSize, drawingRect, texture, userData, isVisible), m_animationManager {animationManager}, m_mapper{mapper},
              m_soundManager {soundManager}, m_velocity{velocity}
            {
                m_lives = lives;
                // Draw the sprite centred on the body so the hitbox lines up with the visible bat.
                // PhysicsSprite's default origin hangs the sprite below/around the hitbox.
                m_origin = raylib::Vector2(spriteSize.x / 2.f, spriteSize.y / 2.f);
                m_body->SetFixedRotation(true);
                m_animation = m_animationManager.getAnimation(AnimationName::EnemyBatMove);
                m_body->SetLinearVelocity({m_velocity.x, m_velocity.y});
                // Map property face_left: the bat keeps this facing however it flies or turns.
                m_flip = isFacingLeft;
            }
            void move(const raylib::Vector2 &toMove) override;

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

            float m_turnTimer{0.f};
            float m_turnTriggerValue{3.f};
            ggj::IAnimationManager<ggj::Animation, ggj::AnimationName> &m_animationManager;
            IMapper &m_mapper;
            ggj::ISoundManager &m_soundManager;
            Animation m_animation;
            raylib::Vector2 m_velocity {0.f, 0.f};

    };

} // ggj

#endif //GAME_DEV_TEMPLATE_BAT_H
