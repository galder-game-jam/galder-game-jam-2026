//
// Created by robin on 27.06.2026.
//

#ifndef GGJ_NETPLAYSCREEN_H
#define GGJ_NETPLAYSCREEN_H

#include <cstdint>
#include <string>
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../enums/TextureName.h"
#include "../interfaces/manager/INetworkManager.h"

namespace ggj
{
    /*!
     * Screen shown after picking "Netplay" in the main menu. Renders the menu backdrop and a Dear
     * ImGui dialog where the player chooses to host or join, types a name (<= 20 chars) and, as a
     * client, an IP + port. The raylib backdrop is drawn from draw(); the ImGui dialog from
     * drawImGui() (which the GameManager calls inside an ImGui frame).
     */
    class NetplayScreen : public IGameObject<raylib::Vector2>
    {
        public:
        explicit NetplayScreen(ITextureManager<TextureName, raylib::Texture>& textures) : m_textures { textures } { m_isVisible = true; }

        void initialize();

        void drawImGui();

        [[nodiscard]] bool isConfirmed() const { return m_isConfirmed; }
        [[nodiscard]] bool isBack() const { return m_isBack; }
        [[nodiscard]] bool isHost() const { return m_isHost; }
        [[nodiscard]] std::string getPlayerName() const { return m_nameBuffer; }
        [[nodiscard]] std::string getIpAddress() const { return m_ipBuffer; }
        [[nodiscard]] uint16_t getPort() const { return static_cast<uint16_t>(m_port); }

        /*! Clears the confirmed/back flags so the screen can be reused. */
        void reset();

        void rotate(float degrees) override {}
        void scale(float scale) override {}
        void move(const raylib::Vector2& toMove) override {}
        void update(float timeDelta) override {}
        void draw() override;

        private:
        ITextureManager<TextureName, raylib::Texture>& m_textures;
        raylib::Texture* m_backgroundTexture { nullptr };

        bool m_isHost { true };
        char m_nameBuffer[21] { "Player" };
        char m_ipBuffer[64] { "127.0.0.1" };
        int m_port { INetworkManager::DEFAULT_PORT };

        bool m_isConfirmed { false };
        bool m_isBack { false };

        static constexpr float SCREEN_WIDTH = 800.0f;
        static constexpr float SCREEN_HEIGHT = 600.0f;
    };
} // namespace ggj

#endif // GGJ_NETPLAYSCREEN_H
