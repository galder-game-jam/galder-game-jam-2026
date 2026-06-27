//
// Created by robin on 27.06.2026.
//

#ifndef GALDER_GAME_JAM_2026_PROJECT_NETWORKROLE_H
#define GALDER_GAME_JAM_2026_PROJECT_NETWORKROLE_H

namespace ggj
{
    /*! Whether this instance is running offline, hosting the authoritative server, or a client. */
    enum class NetworkRole
    {
        None = 0, ///< Single player / not networked.
        Server,   ///< Authoritative host (also a local player).
        Client    ///< Connected to a remote host.
    };
} // namespace ggj

#endif //GALDER_GAME_JAM_2026_PROJECT_NETWORKROLE_H
