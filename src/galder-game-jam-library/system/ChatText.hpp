//
// Created by robin on 14.11.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_CHATTEXT_HPP
#define GALDER_GAME_JAM_2026_PROJECT_CHATTEXT_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ggj::ChatText
{
    static constexpr int32_t FIRST_ACCEPTED_CODEPOINT = 32; ///< Space; everything below is a control character.
    static constexpr int32_t LAST_ACCEPTED_CODEPOINT = 281; ///< The game font bakes 250 glyphs starting at 32.

    /*!
     * Whether a typed codepoint can go into a chat line. Limited to what the font can draw, which
     * also keeps every accepted codepoint at 2 UTF-8 bytes or fewer (see ChatMessage::MAX_TEXT_BYTES).
     *
     * @param codepoint Unicode codepoint
     * @return True if it is printable with the game font
     */
    [[nodiscard]] static constexpr bool IsAcceptedCodepoint(int32_t codepoint)
    {
        return codepoint >= FIRST_ACCEPTED_CODEPOINT && codepoint <= LAST_ACCEPTED_CODEPOINT;
    }

    /*!
     * Appends a codepoint to a string, UTF-8 encoded.
     *
     * @param destination String to append to
     * @param codepoint Unicode codepoint; invalid values are ignored
     */
    inline void AppendUtf8(std::string& destination, int32_t codepoint)
    {
        if (codepoint < 0 || codepoint > 0x10FFFF)
            return;

        if (codepoint < 0x80)
        {
            destination.push_back(static_cast<char>(codepoint));
        }
        else if (codepoint < 0x800)
        {
            destination.push_back(static_cast<char>(0xC0 | (codepoint >> 6)));
            destination.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else if (codepoint < 0x10000)
        {
            destination.push_back(static_cast<char>(0xE0 | (codepoint >> 12)));
            destination.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            destination.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
        else
        {
            destination.push_back(static_cast<char>(0xF0 | (codepoint >> 18)));
            destination.push_back(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F)));
            destination.push_back(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F)));
            destination.push_back(static_cast<char>(0x80 | (codepoint & 0x3F)));
        }
    }

    /*!
     * Copies UTF-8 text into a fixed buffer, cutting it short on a codepoint boundary so a
     * multi-byte character is never split. The result is always null-terminated.
     *
     * @param destination Buffer to write to
     * @param destinationSize Size of the buffer in bytes, including room for the terminator
     * @param source UTF-8 text
     */
    inline void CopyUtf8Truncated(char* destination, size_t destinationSize, const std::string& source)
    {
        if (destination == nullptr || destinationSize == 0)
            return;

        size_t length = std::min(source.size(), destinationSize - 1);
        // Back off continuation bytes (10xxxxxx) when the cut would land inside a character.
        if (length < source.size())
        {
            while (length > 0 && (static_cast<unsigned char>(source[length]) & 0xC0) == 0x80)
                --length;
        }

        source.copy(destination, length);
        destination[length] = '\0';
    }

    /*!
     * Greedy word wrap. Words wider than a whole line are broken up character by character.
     * Spaces at a line break are dropped.
     *
     * @param text UTF-8 text to wrap
     * @param firstLineWidth Room on the first line (it may follow a prefix such as the sender name)
     * @param lineWidth Room on every following line
     * @param measure Returns the drawn width of a piece of text
     * @return The lines; at least one (possibly empty)
     */
    inline std::vector<std::string> WrapText(const std::string& text, float firstLineWidth, float lineWidth, const std::function<float(const std::string&)>& measure)
    {
        // Split into UTF-8 characters first so hard breaks never cut a multi-byte sequence.
        std::vector<std::string> characters;
        for (size_t i = 0; i < text.size();)
        {
            size_t length = 1;
            while (i + length < text.size() && (static_cast<unsigned char>(text[i + length]) & 0xC0) == 0x80)
                ++length;
            characters.push_back(text.substr(i, length));
            i += length;
        }

        std::vector<std::string> lines { std::string {} };
        float available = firstLineWidth;

        const auto startNewLine = [&]()
        {
            while (!lines.back().empty() && lines.back().back() == ' ')
                lines.back().pop_back();
            lines.emplace_back();
            available = lineWidth;
        };

        size_t index = 0;
        while (index < characters.size())
        {
            if (characters[index] == " ")
            {
                if (!lines.back().empty())
                    lines.back() += ' ';
                ++index;
                continue;
            }

            std::string word;
            size_t end = index;
            while (end < characters.size() && characters[end] != " ")
                word += characters[end++];

            if (measure(lines.back() + word) <= available)
            {
                lines.back() += word;
                index = end;
                continue;
            }

            // Doesn't fit: move to a fresh (full width) line if that helps, otherwise break the word up.
            if (measure(word) <= lineWidth && (!lines.back().empty() || available < lineWidth))
            {
                startNewLine();
                continue;
            }

            if (!lines.back().empty() && measure(lines.back() + characters[index]) > available)
                startNewLine();

            while (index < end && measure(lines.back() + characters[index]) <= available)
                lines.back() += characters[index++];

            // A single character wider than the line still has to go somewhere.
            if (lines.back().empty() && index < end)
                lines.back() += characters[index++];

            if (index < end)
                startNewLine();
        }

        return lines;
    }
} // namespace ggj::ChatText

#endif // GALDER_GAME_JAM_2026_PROJECT_CHATTEXT_HPP
