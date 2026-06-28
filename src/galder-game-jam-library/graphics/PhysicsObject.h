//
// Created by robin on 30.10.22.
//

#ifndef GAME_DEV_TEMPLATE_PHYSICSOBJECT_H
#define GAME_DEV_TEMPLATE_PHYSICSOBJECT_H

#include "raylib-cpp.hpp"
#include "box2d/box2d.h"
#include "../enums/PhysicsShape.h"
#include "../../gamedev-base/GameDevBase.h"
#include "../system/UserData.hpp"

namespace ggj
{
    class ISoundManager;

    class PhysicsObject : public ggj::IGameObject<raylib::Vector2>
    {
        public:
        PhysicsObject() = default;
        ~PhysicsObject() override
        {
            if (m_body != nullptr)
            {
                m_body->GetWorld()->DestroyBody(m_body);
            }
        }
        PhysicsObject(b2Body* body, const raylib::Vector2& size, PhysicsShape shape, const UserData& userData, raylib::Color color = raylib::Color::Red(), bool isVisible = true)
            : m_body { body }, m_size { size }, m_shape { shape }, m_userData { userData }, m_origin { size.x / 2, size.y / 2 }, m_color { color }
        {
            m_isVisible = isVisible;
        }
        void rotate(float degrees) override;
        void scale(float scale) override;
        void move(const raylib::Vector2& toMove) override;
        void destroy() override { ggj::IGameObject<raylib::Vector2>::destroy(); }
        void update(float timeDelta) override;
        void draw() override;

        virtual void beginContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) {}
        virtual void endContact(PhysicsObject* a, PhysicsObject* b, b2Contact* contact) {}
        virtual void preSolve(PhysicsObject* a, PhysicsObject* b, b2Contact* contact, const b2Manifold* oldManifold) {}
        virtual void postSolve(PhysicsObject* a, PhysicsObject* b, b2Contact* contact, const b2ContactImpulse* impulse) {}

        /*!
         * Called by the melee hitbox when this object is struck. Default behaviour: play death
         * sound, apply knockback, destroy self, credit the kill to @p attacker.
         * Override to implement multi-hit or invulnerability logic.
         *
         * @param soundManager Sound manager for playing hit/death sounds.
         * @param fromLeft     True if the swing came from the left side (determines knockback direction).
         * @param attacker     The owning PhysicsObject of the hitbox (for kill credits).
         */
        virtual void onMeleeHit(ISoundManager* soundManager, bool fromLeft, PhysicsObject* attacker);

        b2Body* getBody();
        UserData* getUserData();

        /*! Network sync id (0 = not a networked object). Assigned by World by spawn order. */
        [[nodiscard]] uint32_t getNetworkId() const { return m_networkId; }
        void setNetworkId(uint32_t id) { m_networkId = id; }

        /*!
             * Hard-sets the object to an authoritative position received over the network.
             * getPosition() returns body position * PPM, so the inverse conversion is applied here.
             */
        void setNetworkPosition(const raylib::Vector2& pixelPosition)
        {
            if (m_body != nullptr)
                m_body->SetTransform(ConvertToB2Vec2(pixelPosition), m_body->GetAngle());
            m_position = pixelPosition;
        }

        //Static helpers
        static constexpr float PPM = 30.f;
        static b2Vec2 ConvertToB2Vec2(const raylib::Vector2& vector);
        [[nodiscard]] static raylib::Vector2 ConvertToVector2(const b2Vec2& vec2);
        static float ConvertToDegrees(float radians);

        [[nodiscard]] int getLives() const { return m_lives; }

        protected:
        b2Body* m_body { nullptr };
        PhysicsShape m_shape { PhysicsShape::None };
        int m_lives { 1 };

        raylib::Vector2 m_size {};
        raylib::Vector2 m_origin {};
        raylib::Color m_color {};
        UserData m_userData {};
        raylib::Vector2 m_velocity {};
        uint32_t m_networkId { 0 };

        public:
        [[nodiscard]] raylib::Vector2 getVelocity() const;
    };

} // namespace ggj

#endif //GAME_DEV_TEMPLATE_PHYSICSOBJECT_H
