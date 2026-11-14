//
// Loopback test for syncing the stats shown in the HUD party bars. Needs real sockets, so it is
// hidden from the default run: execute it with `galder-game-jam-library-tests "[.network]"`.
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
    constexpr uint16_t TEST_PORT = 23341;

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

TEST_CASE("Level and resources reach the other peers", "[.network][netplay]")
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

    NetworkPlayerStats hostStats {};
    hostStats.lives = 5;
    hostStats.level = 4;
    hostStats.energy = 55.f;
    hostStats.maxEnergy = 100.f;
    host.setLocalPlayerStats(hostStats);

    NetworkPlayerStats guestStats {};
    guestStats.lives = 3;
    guestStats.level = 7;
    guestStats.mana = 120.f;
    guestStats.maxMana = 200.f;
    client.setLocalPlayerStats(guestStats);

    SECTION("The host sees the guest's wizard resources")
    {
        REQUIRE(pollUntil(host, client, [&host]() {
            const auto& remotes = host.remotePlayers();
            return !remotes.empty() && remotes.begin()->second.level == 7;
        }));
        const RemotePlayer& guest = host.remotePlayers().begin()->second;
        REQUIRE(guest.lives == 3);
        REQUIRE(guest.mana == Approx(120.f));
        REQUIRE(guest.maxMana == Approx(200.f));
        REQUIRE(guest.maxEnergy == Approx(0.f));
    }

    SECTION("The guest sees the host's rogue resources")
    {
        REQUIRE(pollUntil(host, client, [&client]() {
            const auto& remotes = client.remotePlayers();
            return !remotes.empty() && remotes.begin()->second.level == 4;
        }));
        const RemotePlayer& hostPlayer = client.remotePlayers().begin()->second;
        REQUIRE(hostPlayer.lives == 5);
        REQUIRE(hostPlayer.energy == Approx(55.f));
        REQUIRE(hostPlayer.maxEnergy == Approx(100.f));
        REQUIRE(hostPlayer.maxMana == Approx(0.f));
    }
}
