//
// Created by soberga on 22.11.2025.
//

#ifndef GALDER_GAME_JAM_2025_PROJECT_DIALOG_H
#define GALDER_GAME_JAM_2025_PROJECT_DIALOG_H

#include "Player.h"
#include "raylib-cpp.hpp"
#include "../../gamedev-base/GameDevBase.h"
#include "../managers/DialogManager.h"
#include "../managers/DialogLoader.h"
#include "../enums/ResourceName.h"
namespace ggj
{
    class World;

    class Dialog : public IGameObject<raylib::Vector2>
    {
        public:
        Dialog(IMapper& mapper, ggj::ITextureManager<TextureName, raylib::Texture>& textures, IInputManager<KeyboardKey>& inputManager, IResourceManager<ResourceName>& resources)
            : m_mapper { mapper }, m_textures { textures }, m_inputManager { inputManager }, m_resources { resources }
        {
            m_isVisible = true;
        }
        void initialize();
        void setFont(raylib::Font* font);
        void setPlayer(IPlayer* player);
        void setWorld(World* world);
        void setVisible(bool isVisible);
        void rotate(float degrees) override;

        void scale(float scale) override;
        void move(const raylib::Vector2& toMove) override;
        void update(float timeDelta) override;
        void draw() override;

        private:
        void handleInputs(float timeDelta);
        void refreshVisibleChoices();
        void DrawBox(raylib::Vector2 offset, int width, int height) const;
        void drawDialogBox(raylib::Vector2 offset, int width, int height, const DialogItem& item) const;
        IMapper& m_mapper;
        ITextureManager<TextureName, raylib::Texture>& m_textures;
        IInputManager<KeyboardKey>& m_inputManager;
        IResourceManager<ResourceName>& m_resources;
        raylib::Texture* m_dialogTexture;

        DialogManager m_dialogManager {};
        raylib::Font* m_font = nullptr;

        IPlayer* m_currentPlayer = nullptr;
        World* m_currentWorld = nullptr;

        raylib::Vector2 m_size {};
        raylib::Vector2 m_origin {};
        raylib::Color m_color {};
        UserData m_userData {};
        raylib::Vector2 m_velocity {};

        const float TILE_UNIT = 16.0f;
        const int FONT_SIZE = 15;
        const raylib::Color HUD_FONT_COLOR = BLACK;
        const int HUD_ALPHA = 180;

        raylib::Vector2 m_dialogOffset {};
        raylib::Rectangle m_boxTopLeftRect { TILE_UNIT * 0, TILE_UNIT * 8, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxTopMiddleRect { TILE_UNIT * 2, TILE_UNIT * 8, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxTopRightRect { TILE_UNIT * 4, TILE_UNIT * 8, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxMiddleLeftRect { TILE_UNIT * 0, TILE_UNIT * 10, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxMiddleMiddleRect { TILE_UNIT * 2, TILE_UNIT * 10, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxMiddleRightRect { TILE_UNIT * 4, TILE_UNIT * 10, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxBottomLeftRect { TILE_UNIT * 0, TILE_UNIT * 12, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxBottomMiddleRect { TILE_UNIT * 2, TILE_UNIT * 12, TILE_UNIT * 2, TILE_UNIT * 2 };
        raylib::Rectangle m_boxBottomRightRect { TILE_UNIT * 4, TILE_UNIT * 12, TILE_UNIT * 2, TILE_UNIT * 2 };

        raylib::Rectangle m_dialogTabButtonRect { TILE_UNIT * 0, TILE_UNIT * 15, TILE_UNIT * 2, TILE_UNIT * 1 };
        raylib::Rectangle m_dialogSpaceButtonRect { TILE_UNIT * 4, TILE_UNIT * 15, TILE_UNIT * 3, TILE_UNIT * 1 };
        raylib::Rectangle m_dialogArrowRect { TILE_UNIT * 2, TILE_UNIT * 15, TILE_UNIT * 2, TILE_UNIT * 1 };
        std::string m_dialogId {};
        bool m_draw {};
        int m_currentChoice { 0 };
        std::vector<DialogChoice> m_visibleChoices {};
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2025_PROJECT_DIALOG_H