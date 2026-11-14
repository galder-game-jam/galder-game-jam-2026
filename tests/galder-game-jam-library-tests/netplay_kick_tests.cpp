//
// Loopback test for kicking a netplay client. Needs real sockets, so it is hidden from the default
// run: execute it with `galder-game-jam-library-tests "[.network]"`.
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
    constexpr uint16_t TEST_PORT = 23339;

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

TEST_CASE("Host can kick a connected client", "[.network][netplay]")
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

    REQUIRE(pollUntil(host, client, [&host]() { return host.connectedPlayers().size() == 2; }));

    SECTION("The host itself and unknown ids cannot be kicked")
    {
        REQUIRE_FALSE(host.kickPlayer(INetworkManager::HOST_PLAYER_ID));
        REQUIRE_FALSE(host.kickPlayer(4242));
        REQUIRE(host.connectedPlayers().size() == 2);
    }

    SECTION("A client cannot kick anyone")
    {
        REQUIRE_FALSE(client.kickPlayer(INetworkManager::HOST_PLAYER_ID + 1));
        REQUIRE_FALSE(client.wasKicked());
    }

    SECTION("A kicked client is told, and the host forgets them")
    {
        uint32_t guestId = INetworkManager::NO_PLAYER_ID;
        for (const ConnectedPlayer& player : host.connectedPlayers())
        {
            if (!player.isHost)
                guestId = player.playerId;
        }
        REQUIRE(guestId != INetworkManager::NO_PLAYER_ID);

        REQUIRE_FALSE(client.wasKicked());
        REQUIRE(host.kickPlayer(guestId));
        REQUIRE(host.connectedPlayers().size() == 1);
        REQUIRE(host.remotePlayers().empty());
        REQUIRE_FALSE(host.kickPlayer(guestId)); // already gone

        REQUIRE(pollUntil(host, client, [&client]() { return client.wasKicked(); }));
        REQUIRE(host.connectedPlayers().size() == 1);

        client.shutdown();
        REQUIRE_FALSE(client.wasKicked()); // cleared for the next session
    }

    client.shutdown();
    host.shutdown();
}
