//
// Created by robin on 03.11.22.
//

#ifndef GAME_DEV_TEMPLATE_PHYSICSSPRITE_H
#define GAME_DEV_TEMPLATE_PHYSICSSPRITE_H

#include "raylib-cpp.hpp"
#include "box2d/box2d.h"
#include "../enums/PhysicsShape.h"
#include "../../gamedev-base/GameDevBase.h"
#include "PhysicsObject.h"

namespace ggj
{

    class PhysicsSprite : public PhysicsObject
    {
        public:
            PhysicsSprite() = default;
            PhysicsSprite(b2Body *body, const raylib::Vector2 &physicsSize, const raylib::Vector2 &spriteSize, const raylib::Rectangle &drawingRect, raylib::Texture * texture, const UserData &userData, bool isVisible = true)
            : PhysicsObject(body, physicsSize, (body->GetFixtureList()[0].GetShape()->GetType() == b2Shape::e_circle) ? PhysicsShape::Circle : PhysicsShape::Rectangle, userData, raylib::Color::Red(), isVisible),
              m_spriteSize{spriteSize}, m_drawingRect {drawingRect}, m_texture {texture}
            {
                if(m_shape == PhysicsShape::Circle)
                    m_origin = raylib::Vector2(spriteSize.x / 2, spriteSize.y - physicsSize.y);
                else
                    m_origin = raylib::Vector2(spriteSize.x / 2, spriteSize.y - physicsSize.y / 2);
            }
            void draw() override;
            [[nodiscard]] bool isFlipped() const override { return m_flip; }
            void setFlipped(bool isFlipped) override { m_flip = isFlipped; }

        protected:
            /*!
             * Replaces the body's fixture (box or circle) with a box of the given size, keeping its
             * bottom edge and horizontal centre where they were. Map objects only define a small
             * 16x16 shape (often an ellipse) at the feet, which is far smaller than the visible
             * character; growing it upwards makes the hitbox cover the body without moving where
             * the sprite is drawn.
             *
             * @param hitboxSize New hitbox size in pixels
             */
            void resizeHitboxKeepingBottom(const raylib::Vector2 &hitboxSize)
            {
                b2Fixture* const oldFixture = m_body->GetFixtureList();
                if (oldFixture == nullptr)
                    return;

                // For circles m_size holds the radius (see World), for boxes the full size.
                const float bottomOffset = (m_shape == PhysicsShape::Circle) ? m_size.x : m_size.y / 2.f;
                const b2Vec2 halfSize = ConvertToB2Vec2({hitboxSize.x / 2.f, hitboxSize.y / 2.f});
                const b2Vec2 center = ConvertToB2Vec2({0.f, bottomOffset - hitboxSize.y / 2.f});

                b2PolygonShape box;
                box.SetAsBox(halfSize.x - b2_polygonRadius, halfSize.y - b2_polygonRadius, center, 0.f);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &box;
                fixtureDef.density = oldFixture->GetDensity();
                fixtureDef.friction = oldFixture->GetFriction();
                fixtureDef.restitution = oldFixture->GetRestitution();
                fixtureDef.isSensor = oldFixture->IsSensor();
                fixtureDef.filter = oldFixture->GetFilterData();

                m_body->DestroyFixture(oldFixture);
                m_body->CreateFixture(&fixtureDef);
                m_size = hitboxSize;
                m_shape = PhysicsShape::Rectangle;
            }

            raylib::Texture *m_texture {nullptr};
            raylib::Rectangle m_drawingRect;
            bool m_flip {false};
            raylib::Color m_tint {WHITE}; ///< Colour multiplied into the sprite when drawn; lets a sprite telegraph attacks.
            raylib::Vector2 m_spriteSize {0.f, 0.f};
    };

} // dev

#endif //GAME_DEV_TEMPLATE_PHYSICSSPRITE_H
