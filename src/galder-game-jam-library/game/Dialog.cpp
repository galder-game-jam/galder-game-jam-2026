//
// Created by soberga on 22.11.2025.
//

#include "Dialog.h"
#include "../GalderGameJamLibrary.h"

void ggj::Dialog::initialize()
{
    m_color = raylib::Color::White();
    m_size = raylib::Vector2{200, 20};
    m_origin = raylib::Vector2{20, 20};
    m_position = raylib::Vector2{ 5, 0 };

    m_dialogOffset = raylib::Vector2{m_position.x + 200, m_position.y + 380};
    m_dialogTexture = m_textures.get(TextureName::HudRpgSubset);
}

void ggj::Dialog::setFont(raylib::Font *font)
{
    m_font = font;
}

void ggj::Dialog::setPlayer(Player *player)
{
    m_currentPlayer = player;
}

void ggj::Dialog::setWorld(World *world)
{
    m_currentWorld = world;
}
void ggj::Dialog::setVisible(bool isVisible)
{
    m_isVisible = isVisible;
}

void ggj::Dialog::rotate(float degrees)
{
}

void ggj::Dialog::scale(float scale)
{
}

void ggj::Dialog::move(const raylib::Vector2& toMove)
{
}

void ggj::Dialog::update(float timeDelta)
{
    if (m_currentPlayer->isInDialog())
    {
        if (!m_draw)
        {
            m_dialogId = m_currentPlayer->getDialogId();
            m_draw = true;
            m_currentChoice = 0;
        }
        handleInputs(timeDelta);
    }
}

void ggj::Dialog::draw()
{
    if (m_draw)
    {
        if (m_dialogManager.hasDialog(m_dialogId))
        {
            DialogItem *dialogItem = m_dialogManager.getDialog(m_dialogId);
            drawDialogBox(m_dialogOffset, 15, 3, *dialogItem);
        }
        else
        {
            drawDialogBox(m_dialogOffset, 15, 3, DialogItem {m_dialogId, TextFormat("Hmm.. A glitch in the Matrix\nThe dialogId %s could not be found", m_dialogId.c_str())});
        }
    }
}

void ggj::Dialog::DrawBox(raylib::Vector2 offset, int width, int height) const
{
    float grid = TILE_UNIT * 2;
    raylib::Vector2 shrinkflation = raylib::Vector2(4, 4);

    raylib::Rectangle backdrop = raylib::Rectangle(offset + shrinkflation, raylib::Vector2((width+2) * grid, (height+2) * grid) - shrinkflation * 2);
    DrawRectangle(backdrop.x, backdrop.y, backdrop.width, backdrop.height, raylib::Color(143, 86, 59, HUD_ALPHA));

    raylib::Rectangle tiles[3][3] =
    {
        { m_boxTopLeftRect,    m_boxTopMiddleRect,    m_boxTopRightRect    },
        { m_boxMiddleLeftRect, m_boxMiddleMiddleRect, m_boxMiddleRightRect },
        { m_boxBottomLeftRect, m_boxBottomMiddleRect, m_boxBottomRightRect }
    };

    for (int y = 0; y < height + 2; ++y)
    {
        for (int x = 0; x < width + 2; ++x)
        {
            int tx = (x == 0) ? 0 : (x == width + 1 ? 2 : 1);   // left/mid/right tile
            int ty = (y == 0) ? 0 : (y == height + 1 ? 2 : 1);  // top/mid/bottom tile

            const raylib::Rectangle& src = tiles[ty][tx];
            raylib::Vector2 pos = offset + raylib::Vector2{grid * x, grid * y};

            m_dialogTexture->Draw(src, pos);
        }
    }
}

void ggj::Dialog::drawDialogBox(raylib::Vector2 offset, int width, int height, const DialogItem &item) const
{
    DrawBox(offset, width, height);

    std::string name = "Galder";//m_currentPlayer->getName();
    int enemies = m_currentWorld->getEnemies();
    int enemiesLeft = m_currentWorld->getNumberOfEnemiesLeft();

    int enemiesKilled = enemies - enemiesLeft;
    bool isOpen = enemiesKilled == enemies;

    m_font->DrawText(item.text, raylib::Vector2{offset.x + TILE_UNIT, offset.y + 7}, FONT_SIZE, 0, YELLOW);

    std::size_t numberOfChoices = item.choices.size();
    float grid = TILE_UNIT * 2;

    if (numberOfChoices > 0)
    {
        raylib::Vector2 choicesOffset(m_dialogOffset.x + grid * 15, m_dialogOffset.y + grid * -2 );

        int choicesBoxHeight = numberOfChoices < 3 ? 0: numberOfChoices - 2;
        int choicesBowWidth = 1;
        DrawBox(choicesOffset, choicesBowWidth,choicesBoxHeight);

        m_dialogTexture->Draw(m_dialogSpaceButtonRect, choicesOffset + raylib::Vector2{grid * choicesBowWidth + 6, grid * (choicesBoxHeight + 1) + 10});
        for (int i = 0; i < numberOfChoices; ++i)
        {
            auto position = choicesOffset + raylib::Vector2{30, 10 + 20 * static_cast<float>(i)};

            if (m_currentChoice == i)
            {
                m_dialogTexture->Draw(m_dialogArrowRect, position - raylib::Vector2{20, 0});
            }

            m_font->DrawText(item.choices[i].text, position, FONT_SIZE, 0, YELLOW);
        }

    }
    // if (enemiesKilled == 0)
    // {
    //     m_font->DrawText(TextFormat("Hello %s!\n", name.c_str()), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 7}, FONT_SIZE, 0, YELLOW);
    //     m_font->DrawText(TextFormat("Welcome to the world of Galdaria."), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 27}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("Your first task is to purge these lands of every creature in these woods!"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 47}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("I am a bit too old to run around and kill, but I have scouted out the area for you."), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 67}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("Currently, %i creatures are running around. Happy little bastards...", enemiesLeft), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 87}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("Kill them for me, and I will grant you access to my torture chamber!"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 107}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("When the deed is done, you will find a portal in a cave to the north."), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 127}, FONT_SIZE, 0, BLACK);
    // }
    // else if (!isOpen)
    // {
    //     m_font->DrawText(TextFormat("Hello again %s!", name.c_str()), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 7}, FONT_SIZE, 0, YELLOW);
    //     m_font->DrawText(TextFormat("How is the hunt going?"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 27}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("Your clothes are drenched in blood I see... Good!"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 47}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("But you are node quite done yet, %i fuckers to go.", enemiesLeft), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 67}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("Don't come back until their souls have been purged."), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 87}, FONT_SIZE, 0, BLACK);
    // }
    // else
    // {
    //     m_font->DrawText(TextFormat("My god %s!", name.c_str()), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 7}, FONT_SIZE, 0, YELLOW);
    //     m_font->DrawText(TextFormat("You did it!! Thank you for getting rid of all these happy bastards for me!"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 27}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("As promised, you may now enter the portal so I can make you my sex sl... "), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 47}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("I mean..."), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 67}, FONT_SIZE, 0, BLACK);
    //     m_font->DrawText(TextFormat("Collect your reward! Come, come my good friend!"), raylib::Vector2{offset.x + TILE_UNIT, offset.y + 87}, FONT_SIZE, 0, BLACK);
    // }

    m_dialogTexture->Draw(m_dialogTabButtonRect, offset + raylib::Vector2(grid * (width + 1) - 6, grid * (height + 1) - 6 + TILE_UNIT));

}

void ggj::Dialog::handleInputs(float timeDelta)
{
    if(m_inputManager.keyPressed(KeyboardKey::Tab))
    {
        m_draw = false;
        m_dialogId = {};
        m_currentPlayer->setHasFinishedDialog();
    }
    if(m_inputManager.keyPressed(KeyboardKey::S))
    {
        int choices = m_dialogManager.getDialog(m_dialogId)->choices.size();
        if (m_currentChoice < choices - 1)
        {
            ++m_currentChoice;
        }
    }
    if(m_inputManager.keyPressed(KeyboardKey::W))
    {
        if (m_currentChoice > 0)
        {
            --m_currentChoice;
        }
    }
    if(m_inputManager.keyPressed(KeyboardKey::Space))
    {
        if (m_dialogManager.getDialog(m_dialogId)->choices.size() == 0)
            return;

        m_dialogId = m_dialogManager.getDialog(m_dialogId)->choices[m_currentChoice].nextDialogId;
        m_currentChoice = 0;
    }
}
