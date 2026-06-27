//
// Created by robin on 08.11.24.
//

#ifndef GALDER_GAME_JAM_2024_PROJECT_ICLIENT_H
#define GALDER_GAME_JAM_2024_PROJECT_ICLIENT_H

#include "../../enums/ConnectionStatus.hpp"
#include "../../data/ServerHostInfo.hpp"
#include <cstdint>
#include <queue>
namespace ggj
{
    template <class TClientData, class TServerData> class IClient
    {
        public:
        virtual ~IClient() noexcept = default;
        virtual bool initialize() = 0;
        virtual void connect(uint16_t port, std::string ipAddress) = 0;

        /*!
             * Non-blocking counterpart to connect(): start connecting without entering the
             * blocking loop.
             */
        virtual void beginConnect(uint16_t port, std::string ipAddress) = 0;
        /*!
             * Performs a single non-blocking iteration of the client program (drain incoming
             * messages + run connection callbacks). Call once per frame from the game loop.
             */
        virtual void poll() = 0;
        /*! Current connection status (None until connected, Connected once the handshake completes). */
        [[nodiscard]] virtual ConnectionStatus status() const = 0;

        [[nodiscard]] virtual ServerHostInfo getServerInfo() const = 0;
        virtual void ping() const = 0;
        virtual void disconnect() const = 0;
        virtual void queueData(const TClientData& data) { m_queue.push(data); };

        protected:
        virtual bool send(const TClientData& data) = 0;
        virtual TServerData receive() = 0;
        virtual void clientProgram() = 0;
        std::queue<TClientData> m_queue;
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2024_PROJECT_ICLIENT_H
