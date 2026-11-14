//
// Created by robin on 14.11.2026.
//

#include "ChatLog.h"
#include "ChatText.hpp"

#include <algorithm>

namespace ggj
{
    bool ChatLog::appendCodepoint(int32_t codepoint)
    {
        if (!ChatText::IsAcceptedCodepoint(codepoint) || m_draft.size() >= MAX_DRAFT_LENGTH)
            return false;

        m_draft.push_back(codepoint);
        return true;
    }

    void ChatLog::eraseLast()
    {
        if (!m_draft.empty())
            m_draft.pop_back();
    }

    void ChatLog::clearDraft()
    {
        m_draft.clear();
    }

    std::optional<std::string> ChatLog::takeDraft()
    {
        const std::string text = draftUtf8();
        m_draft.clear();

        const size_t first = text.find_first_not_of(' ');
        if (first == std::string::npos)
            return std::nullopt;

        const size_t last = text.find_last_not_of(' ');
        return text.substr(first, last - first + 1);
    }

    std::string ChatLog::draftUtf8() const
    {
        std::string text;
        for (const int32_t codepoint : m_draft)
            ChatText::AppendUtf8(text, codepoint);
        return text;
    }

    void ChatLog::addEntry(const ChatEntry& entry)
    {
        m_entries.push_back(entry);
        while (m_entries.size() > MAX_ENTRIES)
            m_entries.pop_front();
    }

    void ChatLog::update(float timeDelta)
    {
        for (ChatEntry& entry : m_entries)
            entry.age += timeDelta;
    }

    float ChatLog::FadeAlpha(float age)
    {
        const float fadeStart = VISIBLE_SECONDS - FADE_SECONDS;
        if (age <= fadeStart)
            return 1.f;
        return std::clamp((VISIBLE_SECONDS - age) / FADE_SECONDS, 0.f, 1.f);
    }

    void ChatLog::reset()
    {
        m_draft.clear();
        m_entries.clear();
    }
} // namespace ggj
