//
// Created by robin on 14.11.2026.
//

#include "ChatBox.h"
#include "../system/ChatText.hpp"

#include <cmath>
#include <optional>

namespace ggj
{
    void ChatBox::setFonts(raylib::Font* bold, raylib::Font* regular)
    {
        m_boldFont = bold;
        m_regularFont = regular;
    }

    void ChatBox::handleInput()
    {
        // Always drained, so whatever was typed while the chat was closed never ends up in it.
        const std::vector<int32_t> typed = m_input.takeCharsPressed();

        if (m_input.textKeyPressed(TOGGLE_KEY))
        {
            if (m_isOpen)
                close();
            else
                open();
            return;
        }

        if (!m_isOpen)
            return;

        for (const int32_t codepoint : typed)
            m_log.appendCodepoint(codepoint);

        if (m_input.textKeyPressed(KeyboardKey::Backspace))
            m_log.eraseLast();

        if (m_input.textKeyPressed(KeyboardKey::Escape))
            m_log.clearDraft();

        if (m_input.textKeyPressed(KeyboardKey::Enter) || m_input.textKeyPressed(KeyboardKey::KpEnter))
        {
            if (const std::optional<std::string> text = m_log.takeDraft(); text.has_value())
                m_network.sendChatMessage(*text);
        }
    }

    void ChatBox::update(float timeDelta)
    {
        for (const ChatEntry& entry : m_network.takeChatMessages())
            m_log.addEntry(entry);
        m_log.update(timeDelta);
        m_caretTimer += timeDelta;
    }

    void ChatBox::open()
    {
        m_isOpen = true;
        m_caretTimer = 0.f;
        m_input.setKeyboardCaptured(true);
    }

    void ChatBox::close()
    {
        if (!m_isOpen)
            return;
        m_isOpen = false;
        m_input.setKeyboardCaptured(false);
    }

    void ChatBox::reset()
    {
        close();
        m_log.reset();
    }

    std::vector<ChatBox::VisualLine> ChatBox::layout(const std::string& prefix, const std::string& text, const raylib::Color& color) const
    {
        const float prefixWidth = m_boldFont->MeasureText(prefix, FONT_SIZE, 0).x;
        const float lineWidth = PANEL_WIDTH - PADDING * 2;
        const std::vector<std::string> wrapped =
            ChatText::WrapText(text, lineWidth - prefixWidth, lineWidth, [this](const std::string& piece) { return m_regularFont->MeasureText(piece, FONT_SIZE, 0).x; });

        std::vector<VisualLine> lines;
        lines.reserve(wrapped.size());
        for (size_t i = 0; i < wrapped.size(); ++i)
            lines.push_back(VisualLine { i == 0 ? prefix : std::string {}, wrapped[i], color });
        return lines;
    }

    void ChatBox::drawLine(const VisualLine& line, float y, float alpha) const
    {
        const raylib::Color color = line.color.Fade(alpha);
        // A drop shadow keeps dark name colours readable on any background.
        const raylib::Color shadow = raylib::Color::Black().Fade(alpha * 0.8f);

        float x = PANEL_X + PADDING;
        if (!line.prefix.empty())
        {
            m_boldFont->DrawText(line.prefix, raylib::Vector2 { x + 1, y + 1 }, FONT_SIZE, 0, shadow);
            m_boldFont->DrawText(line.prefix, raylib::Vector2 { x, y }, FONT_SIZE, 0, color);
            x += m_boldFont->MeasureText(line.prefix, FONT_SIZE, 0).x;
        }
        m_regularFont->DrawText(line.text, raylib::Vector2 { x + 1, y + 1 }, FONT_SIZE, 0, shadow);
        m_regularFont->DrawText(line.text, raylib::Vector2 { x, y }, FONT_SIZE, 0, color);
    }

    void ChatBox::draw() const
    {
        if (m_boldFont == nullptr || m_regularFont == nullptr)
            return;

        // Wrapped history, oldest first; only what fits is drawn, anchored to the bottom.
        std::vector<VisualLine> history;
        std::vector<float> alphas;
        for (const ChatEntry& entry : m_log.entries())
        {
            const float alpha = m_isOpen ? 1.f : ChatLog::FadeAlpha(entry.age);
            if (alpha <= 0.f)
                continue;
            for (const VisualLine& line : layout(entry.name + ": ", entry.text, entry.nameColor))
            {
                history.push_back(line);
                alphas.push_back(alpha);
            }
        }

        const size_t maxLines = m_isOpen ? MAX_OPEN_LINES : MAX_CLOSED_LINES;
        const size_t firstLine = history.size() > maxLines ? history.size() - maxLines : 0;
        const size_t shownLines = history.size() - firstLine;

        float bottom = PANEL_BOTTOM;
        if (m_isOpen)
        {
            const raylib::Color nameColor = m_network.localPlayerNameColor();
            const bool isCaretVisible = std::fmod(m_caretTimer, 1.f) < 0.5f;
            std::vector<VisualLine> input = layout(m_network.localPlayerName() + ": ", m_log.draftUtf8() + (isCaretVisible ? "_" : " "), nameColor);

            const float inputHeight = static_cast<float>(input.size()) * LINE_HEIGHT + PADDING * 2;
            const float inputTop = bottom - inputHeight;
            DrawRectangle(static_cast<int>(PANEL_X), static_cast<int>(inputTop), static_cast<int>(PANEL_WIDTH), static_cast<int>(inputHeight), Color { 0, 0, 0, 190 });
            DrawRectangleLines(static_cast<int>(PANEL_X), static_cast<int>(inputTop), static_cast<int>(PANEL_WIDTH), static_cast<int>(inputHeight), Color { 255, 255, 255, 120 });
            for (size_t i = 0; i < input.size(); ++i)
                drawLine(input[i], inputTop + PADDING + static_cast<float>(i) * LINE_HEIGHT, 1.f);

            const std::string counter = std::to_string(m_log.draftLength()) + "/" + std::to_string(ChatLog::MAX_DRAFT_LENGTH);
            const float counterWidth = m_regularFont->MeasureText(counter, SMALL_FONT_SIZE, 0).x;
            m_regularFont->DrawText(counter, raylib::Vector2 { PANEL_X + PANEL_WIDTH - counterWidth - PADDING, inputTop - SMALL_FONT_SIZE - 2 }, SMALL_FONT_SIZE, 0,
                                    Color { 200, 200, 200, 220 });

            bottom = inputTop - SMALL_FONT_SIZE - 4;
            if (shownLines > 0)
            {
                const float historyHeight = static_cast<float>(shownLines) * LINE_HEIGHT + PADDING * 2;
                DrawRectangle(static_cast<int>(PANEL_X), static_cast<int>(bottom - historyHeight), static_cast<int>(PANEL_WIDTH), static_cast<int>(historyHeight),
                              Color { 0, 0, 0, 140 });
            }
            bottom -= PADDING;
        }

        float y = bottom - static_cast<float>(shownLines) * LINE_HEIGHT;
        for (size_t i = firstLine; i < history.size(); ++i)
        {
            drawLine(history[i], y, alphas[i]);
            y += LINE_HEIGHT;
        }
    }
} // namespace ggj
