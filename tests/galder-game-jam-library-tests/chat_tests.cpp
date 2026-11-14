//
// Created by robin on 14.11.2026.
//

#include "catch.hpp"
#include "fakeit.hpp"
#include "../../src/galder-game-jam-library/GalderGameJamLibrary.h"

#include <set>

using namespace fakeit;
using namespace ggj;

namespace
{
    constexpr int32_t O_SLASH = 0xF8; // 'ø', 2 bytes in UTF-8

    /*! Fake width: 10 px per byte, so test expectations are easy to work out by hand. */
    [[nodiscard]] float measureTenPerByte(const std::string& text)
    {
        return static_cast<float>(text.size()) * 10.f;
    }

    /*! Fakes the keyboard for a ChatBox: which keys are pressed this frame and what was typed. */
    struct FakeKeyboard
    {
        Mock<IInputManager<ggj::KeyboardKey>> input;
        std::set<ggj::KeyboardKey> pressed {};
        std::vector<int32_t> typed {};
        bool isCaptured { false };

        FakeKeyboard()
        {
            When(Method(input, textKeyPressed)).AlwaysDo([this](ggj::KeyboardKey key) { return pressed.count(key) > 0; });
            When(Method(input, takeCharsPressed)).AlwaysDo([this]() { return std::exchange(typed, {}); });
            When(Method(input, setKeyboardCaptured)).AlwaysDo([this](bool captured) { isCaptured = captured; });
        }

        /*! Runs one frame of input with only the given key pressed. */
        void press(ChatBox& chat, ggj::KeyboardKey key)
        {
            pressed = { key };
            chat.handleInput();
            pressed.clear();
        }

        void type(ChatBox& chat, const std::string& text)
        {
            for (const char c : text)
                typed.push_back(static_cast<unsigned char>(c));
            chat.handleInput();
        }
    };
} // namespace

TEST_CASE("Chat text - UTF-8 encoding", "[chat]")
{
    std::string text;
    ChatText::AppendUtf8(text, 'A');
    ChatText::AppendUtf8(text, O_SLASH);
    REQUIRE(text == "A\xC3\xB8");
}

TEST_CASE("Chat text - truncating copy never splits a character", "[chat]")
{
    char buffer[4] {};
    ChatText::CopyUtf8Truncated(buffer, sizeof(buffer), "ab\xC3\xB8"); // 4 bytes, 3 fit
    REQUIRE(std::string(buffer) == "ab");

    ChatText::CopyUtf8Truncated(buffer, sizeof(buffer), "abc");
    REQUIRE(std::string(buffer) == "abc");

    ChatText::CopyUtf8Truncated(buffer, sizeof(buffer), "abcdef");
    REQUIRE(std::string(buffer) == "abc");
}

TEST_CASE("Chat text - word wrap", "[chat]")
{
    SECTION("Fits on one line")
    {
        const std::vector<std::string> lines = ChatText::WrapText("hi there", 100.f, 100.f, measureTenPerByte);
        REQUIRE(lines == std::vector<std::string> { "hi there" });
    }

    SECTION("Wraps on spaces and drops the space at the break")
    {
        const std::vector<std::string> lines = ChatText::WrapText("aaa bbb ccc", 70.f, 70.f, measureTenPerByte);
        REQUIRE(lines == std::vector<std::string> { "aaa bbb", "ccc" });
    }

    SECTION("A narrower first line (after the name prefix) wraps sooner")
    {
        const std::vector<std::string> lines = ChatText::WrapText("aaa bbb ccc", 30.f, 70.f, measureTenPerByte);
        REQUIRE(lines == std::vector<std::string> { "aaa", "bbb ccc" });
    }

    SECTION("A word that would fit a full line moves off a short first line")
    {
        const std::vector<std::string> lines = ChatText::WrapText("aaaaa", 30.f, 70.f, measureTenPerByte);
        REQUIRE(lines == std::vector<std::string> { "", "aaaaa" });
    }

    SECTION("Words longer than a line are broken up")
    {
        const std::vector<std::string> lines = ChatText::WrapText("aaaaaaaaaa", 40.f, 40.f, measureTenPerByte);
        REQUIRE(lines == std::vector<std::string> { "aaaa", "aaaa", "aa" });
    }

    SECTION("Hard breaks keep multi-byte characters whole")
    {
        const std::vector<std::string> lines = ChatText::WrapText("\xC3\xB8\xC3\xB8\xC3\xB8", 40.f, 40.f, measureTenPerByte);
        REQUIRE(lines == std::vector<std::string> { "\xC3\xB8\xC3\xB8", "\xC3\xB8" });
    }
}

TEST_CASE("Chat log - draft is limited to 300 characters", "[chat]")
{
    ChatLog log;
    for (size_t i = 0; i < ChatLog::MAX_DRAFT_LENGTH; ++i)
        REQUIRE(log.appendCodepoint(O_SLASH));

    REQUIRE_FALSE(log.appendCodepoint('x'));
    REQUIRE(log.draftLength() == 300);
    REQUIRE(log.draftUtf8().size() < ChatMessage::MAX_TEXT_BYTES); // the worst case still fits on the wire
}

TEST_CASE("Chat log - rejects characters the font can't draw", "[chat]")
{
    ChatLog log;
    REQUIRE_FALSE(log.appendCodepoint('\t'));
    REQUIRE_FALSE(log.appendCodepoint(0x4E2D)); // CJK
    REQUIRE(log.draftLength() == 0);
}

TEST_CASE("Chat log - backspace removes a whole character", "[chat]")
{
    ChatLog log;
    log.appendCodepoint('a');
    log.appendCodepoint(O_SLASH);
    log.eraseLast();
    REQUIRE(log.draftUtf8() == "a");
    log.eraseLast();
    log.eraseLast(); // on an empty draft
    REQUIRE(log.draftLength() == 0);
}

TEST_CASE("Chat log - taking the draft trims it and clears it", "[chat]")
{
    ChatLog log;
    for (const char c : std::string("  hello  "))
        log.appendCodepoint(c);

    const std::optional<std::string> text = log.takeDraft();
    REQUIRE(text.has_value());
    REQUIRE(*text == "hello");
    REQUIRE(log.draftLength() == 0);

    log.appendCodepoint(' ');
    REQUIRE_FALSE(log.takeDraft().has_value());
}

TEST_CASE("Chat log - history is capped, oldest dropped", "[chat]")
{
    ChatLog log;
    for (size_t i = 0; i < ChatLog::MAX_ENTRIES + 5; ++i)
        log.addEntry(ChatEntry { "p", RED, std::to_string(i) });

    REQUIRE(log.entries().size() == ChatLog::MAX_ENTRIES);
    REQUIRE(log.entries().front().text == "5");
}

TEST_CASE("Chat log - lines fade out after a while", "[chat]")
{
    REQUIRE(ChatLog::FadeAlpha(0.f) == Approx(1.f));
    REQUIRE(ChatLog::FadeAlpha(ChatLog::VISIBLE_SECONDS - ChatLog::FADE_SECONDS) == Approx(1.f));
    REQUIRE(ChatLog::FadeAlpha(ChatLog::VISIBLE_SECONDS - ChatLog::FADE_SECONDS / 2) == Approx(0.5f));
    REQUIRE(ChatLog::FadeAlpha(ChatLog::VISIBLE_SECONDS) == Approx(0.f));
    REQUIRE(ChatLog::FadeAlpha(100.f) == Approx(0.f));
}

TEST_CASE("Chat box - Tab toggles the chat and captures the keyboard", "[chat]")
{
    FakeKeyboard keyboard;
    Mock<INetworkManager> network;
    ChatBox chat(keyboard.input.get(), network.get());

    keyboard.press(chat, ggj::KeyboardKey::Tab);
    REQUIRE(chat.isOpen());
    REQUIRE(keyboard.isCaptured);

    keyboard.press(chat, ggj::KeyboardKey::Tab);
    REQUIRE_FALSE(chat.isOpen());
    REQUIRE_FALSE(keyboard.isCaptured);
}

TEST_CASE("Chat box - typing while closed is ignored", "[chat]")
{
    FakeKeyboard keyboard;
    Mock<INetworkManager> network;
    ChatBox chat(keyboard.input.get(), network.get());

    keyboard.type(chat, "wasd");
    keyboard.press(chat, ggj::KeyboardKey::Tab);
    REQUIRE(chat.log().draftLength() == 0);
}

TEST_CASE("Chat box - Escape clears the draft and keeps the chat open", "[chat]")
{
    FakeKeyboard keyboard;
    Mock<INetworkManager> network;
    ChatBox chat(keyboard.input.get(), network.get());

    keyboard.press(chat, ggj::KeyboardKey::Tab);
    keyboard.type(chat, "oops");
    REQUIRE(chat.log().draftUtf8() == "oops");

    keyboard.press(chat, ggj::KeyboardKey::Escape);
    REQUIRE(chat.log().draftLength() == 0);
    REQUIRE(chat.isOpen());
    REQUIRE(keyboard.isCaptured);
}

TEST_CASE("Chat box - Enter sends the draft and keeps the chat open", "[chat]")
{
    FakeKeyboard keyboard;
    Mock<INetworkManager> network;
    // Copied in the stub: FakeIt keeps reference arguments by reference, and the sent string is a temporary.
    std::vector<std::string> sent;
    When(Method(network, sendChatMessage)).AlwaysDo([&sent](const std::string& text) { sent.push_back(text); });
    ChatBox chat(keyboard.input.get(), network.get());

    keyboard.press(chat, ggj::KeyboardKey::Tab);
    keyboard.press(chat, ggj::KeyboardKey::Enter); // empty draft: nothing to send
    REQUIRE(sent.empty());

    keyboard.type(chat, "hi");
    keyboard.press(chat, ggj::KeyboardKey::Enter);
    REQUIRE(sent == std::vector<std::string> { "hi" });
    REQUIRE(chat.isOpen());
    REQUIRE(chat.log().draftLength() == 0);
}

TEST_CASE("Chat box - closing keeps the draft, reset forgets it", "[chat]")
{
    FakeKeyboard keyboard;
    Mock<INetworkManager> network;
    ChatBox chat(keyboard.input.get(), network.get());

    keyboard.press(chat, ggj::KeyboardKey::Tab);
    keyboard.type(chat, "later");
    keyboard.press(chat, ggj::KeyboardKey::Tab);
    keyboard.press(chat, ggj::KeyboardKey::Tab);
    REQUIRE(chat.log().draftUtf8() == "later");

    chat.reset();
    REQUIRE_FALSE(chat.isOpen());
    REQUIRE_FALSE(keyboard.isCaptured);
    REQUIRE(chat.log().draftLength() == 0);
}
