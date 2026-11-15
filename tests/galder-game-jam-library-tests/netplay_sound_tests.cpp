//
// Loopback test for relaying player sound effects to the other peers. Needs real sockets, so it
// is hidden from the default run: execute it with `galder-game-jam-library-tests "[.network]"`.
//

#include "catch.hpp"
#include "fakeit.hpp"
#include "../../src/galder-game-jam-library/GalderGameJamLibrary.h"
#include "../../src/galder-game-jam-library/managers/NetworkManager.h"

#include <chrono>
#include <functional>
#include <thread>

using namespace fakeit;
using namespace ggj;

namespace
{
    constexpr uint16_t TEST_PORT = 23342;

    /*! Polls both peers (~60 Hz) until the condition holds or the timeout passes. */
    [[nodiscard]] bool pollUntil(NetworkManager& host, NetworkManager& client, const std::function<bool()>& condition)
    {
        constexpr float FRAME_SECONDS = 1.f / 60.f;
        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
        while (std::chrono::steady_clock::now() < deadline)
        {
            host.poll(FRAME_SECONDS);
            client.poll(FRAME_SECONDS);
            if (condition())
                return true;
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        return false;
    }
} // namespace

TEST_CASE("Only player action sounds are networked", "[netplay]")
{
    REQUIRE(isNetworkedSound(SoundName::PlayerAttack));
    REQUIRE(isNetworkedSound(SoundName::WizardShoot));
    REQUIRE(isNetworkedSound(SoundName::PlayerHurt));
    REQUIRE(isNetworkedSound(SoundName::PlayerDeath));
    REQUIRE(isNetworkedSound(SoundName::PlayerRespawn));
    REQUIRE_FALSE(isNetworkedSound(SoundName::None));
    REQUIRE_FALSE(isNetworkedSound(SoundName::AbilityNotReady));
    REQUIRE_FALSE(isNetworkedSound(SoundName::DialogMumble));
    REQUIRE_FALSE(isNetworkedSound(static_cast<SoundName>(60000)));
}

TEST_CASE("Sounds one player triggers are heard by the other peers", "[.network][netplay]")
{
    Mock<ILogger> logger;
    Fake(Method(logger, log));
    Mock<IIpAddressResolver> resolver;
    When(Method(resolver, getLocalIpAddress)).AlwaysReturn("127.0.0.1");
    When(Method(resolver, getPublicIpAddress)).AlwaysReturn("127.0.0.1");

    NetworkManager host { logger.get(), resolver.get() };
    NetworkManager client { logger.get(), resolver.get() };
    host.startHost(TEST_PORT, "Host", raylib::Color { 255, 0, 0, 255 }, "Test server", PlayerPickedCharacter::Rogue, CharacterColors {});
    client.startClient("127.0.0.1", TEST_PORT, "Guest", raylib::Color { 0, 255, 0, 255 }, PlayerPickedCharacter::Wizard, CharacterColors {});

    REQUIRE(pollUntil(host, client, [&]() { return !host.remotePlayers().empty() && !client.remotePlayers().empty(); }));

    SECTION("The host hears the guest")
    {
        client.notifyLocalSound(SoundName::PlayerAttack);
        std::vector<SoundEvent> heard;
        REQUIRE(pollUntil(host, client,
                          [&]()
                          {
                              for (const SoundEvent& event : host.takeSoundEvents())
                                  heard.push_back(event);
                              return !heard.empty();
                          }));
        REQUIRE(heard.front().sound == SoundName::PlayerAttack);
        REQUIRE(heard.front().ownerId != INetworkManager::HOST_PLAYER_ID);
    }

    SECTION("The guest hears the host, but not its own sounds, and never sounds that are not networked")
    {
        host.notifyLocalSound(SoundName::AbilityNotReady);
        host.notifyLocalSound(SoundName::PlayerHurt);
        client.notifyLocalSound(SoundName::PlayerDeath);
        std::vector<SoundEvent> heard;
        REQUIRE(pollUntil(host, client,
                          [&]()
                          {
                              for (const SoundEvent& event : client.takeSoundEvents())
                                  heard.push_back(event);
                              return !heard.empty();
                          }));
        // Give any stray extra messages time to arrive before checking nothing else came through.
        (void)pollUntil(host, client, [&]() { return false; });
        for (const SoundEvent& event : client.takeSoundEvents())
            heard.push_back(event);
        REQUIRE(heard.size() == 1);
        REQUIRE(heard.front().sound == SoundName::PlayerHurt);
        REQUIRE(heard.front().ownerId == INetworkManager::HOST_PLAYER_ID);
    }
}
