#ifndef NETWORK_H
#define NETWORK_H

#include <stdbool.h>

/*
 * Stores the latest known state of another player.
 *
 * For our current two-player MVP, we only need to store
 * one remote player.
 *
 * If we later support 3+ players, this can be changed
 * into an array of RemotePlayerState objects.
 */
typedef struct
{
    int player_id;

    float x;
    float y;
    float angle;

    /*
     * false = we have not received a remote player yet
     * true  = this structure contains valid player data
     */
    bool active;

} RemotePlayerState;

/*
 * Connect to the multiplayer WebSocket server.
 */
void Network_Init(const char *server_url);

/*
 * Send the local player's current game state.
 */
void Network_SendPlayerState(
    float x,
    float y,
    float angle
);

/*
 * Returns true if the WebSocket connection
 * to the server is currently open.
 */
bool Network_IsConnected(void);

/*
 * Returns the player ID assigned by the server.
 *
 * Returns -1 if the server has not assigned one yet.
 */
int Network_GetLocalPlayerId(void);

/*
 * Copies the latest remote player state into out_state.
 *
 * Returns:
 *
 * true  = remote player data is available
 * false = no remote player data has been received yet
 */
bool Network_GetRemotePlayerState(
    RemotePlayerState *out_state
);

/*
 * Close the WebSocket connection.
 */
void Network_Shutdown(void);

#endif
