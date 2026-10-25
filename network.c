#include "network.h"

#include <stdio.h>
#include <string.h>

#include <emscripten.h>
#include <emscripten/websocket.h>

/*
 * Handle for the WebSocket connection.
 *
 * A value of 0 means we currently do not
 * have a WebSocket object.
 */
static EMSCRIPTEN_WEBSOCKET_T socket_handle = 0;

/*
 * Whether the WebSocket connection is currently open.
 */
static bool connected = false;

/*
 * Player ID assigned by the server.
 *
 * -1 means we have not received our ID yet.
 */
static int local_player_id = -1;

/*
 * Latest state received for the other player.
 *
 * For now our project is testing two players,
 * so storing one remote player is enough.
 */
static RemotePlayerState remote_player =
{
    .player_id = -1,
    .x = 0.0f,
    .y = 0.0f,
    .angle = 0.0f,
    .active = false
};

/*
 * Runs when the WebSocket successfully connects.
 */
static EM_BOOL OnOpen(
    int event_type,
    const EmscriptenWebSocketOpenEvent *event,
    void *user_data
)
{
    connected = true;

    printf(
        "[Network] Connected to server.\n"
    );

    return EM_TRUE;
}

/*
 * Runs whenever the server sends a message
 * to this client.
 */
static EM_BOOL OnMessage(
    int event_type,
    const EmscriptenWebSocketMessageEvent *event,
    void *user_data
)
{
    /*
     * Our server currently sends text messages only.
     */
    if (!event->isText)
    {
        return EM_TRUE;
    }

    /*
     * Copy the received WebSocket message into
     * a normal null-terminated C string.
     *
     * This makes functions such as sscanf()
     * safe to use on the message.
     */
    char message[256];

    size_t copy_length = event->numBytes;

    if (copy_length >= sizeof(message))
    {
        copy_length = sizeof(message) - 1;
    }

    memcpy(
        message,
        event->data,
        copy_length
    );

    message[copy_length] = '\0';

    /*
     * -------------------------------------------------
     * MESSAGE TYPE 1:
     *
     * WELCOME|playerId
     *
     * Example:
     *
     * WELCOME|2
     *
     * This tells this browser which player ID
     * the server assigned to it.
     * -------------------------------------------------
     */
    if (strncmp(message, "WELCOME|", 8) == 0)
    {
        int received_id;

        /*
         * Read the number after WELCOME|.
         */
        if (
            sscanf(
                message,
                "WELCOME|%d",
                &received_id
            ) == 1
        )
        {
            local_player_id = received_id;

            printf(
                "[Network] Assigned Player ID: %d\n",
                local_player_id
            );
        }

        return EM_TRUE;
    }

    /*
     * -------------------------------------------------
     * MESSAGE TYPE 2:
     *
     * PLAYER|playerId|x|y|angle
     *
     * Example:
     *
     * PLAYER|2|850.50|420.25|1.5708
     *
     * This contains another player's latest
     * position and facing direction.
     * -------------------------------------------------
     */
    if (strncmp(message, "PLAYER|", 7) == 0)
    {
        int sender_id;

        float x;
        float y;
        float angle;

        /*
         * Break the message into its individual values.
         */
        int values_read = sscanf(
            message,
            "PLAYER|%d|%f|%f|%f",
            &sender_id,
            &x,
            &y,
            &angle
        );

        /*
         * sscanf should successfully read
         * exactly four values.
         */
        if (values_read == 4)
        {
            /*
             * Save the remote player's latest state.
             */
            remote_player.player_id = sender_id;

            remote_player.x = x;
            remote_player.y = y;
            remote_player.angle = angle;

            remote_player.active = true;

            /*
             * Temporary debug output.
             *
             * Later we can remove this once the
             * remote player is actually being rendered.
             */
            printf(
                "[Network] Player %d -> X: %.2f Y: %.2f Angle: %.4f\n",
                remote_player.player_id,
                remote_player.x,
                remote_player.y,
                remote_player.angle
            );
        }
        else
        {
            printf(
                "[Network] Invalid PLAYER message: %s\n",
                message
            );
        }

        return EM_TRUE;
    }

    /*
     * If we get here, the server sent a message
     * type that this client does not understand yet.
     */
    printf(
        "[Network] Unknown message: %s\n",
        message
    );

    return EM_TRUE;
}

/*
 * Runs if the WebSocket encounters an error.
 */
static EM_BOOL OnError(
    int event_type,
    const EmscriptenWebSocketErrorEvent *event,
    void *user_data
)
{
    printf(
        "[Network] WebSocket error.\n"
    );

    return EM_TRUE;
}

/*
 * Runs when the WebSocket connection closes.
 */
static EM_BOOL OnClose(
    int event_type,
    const EmscriptenWebSocketCloseEvent *event,
    void *user_data
)
{
    connected = false;

    local_player_id = -1;

    remote_player.active = false;

    printf(
        "[Network] Connection closed. Code: %d\n",
        event->code
    );

    /*
     * Delete the WebSocket object now that
     * the connection has closed.
     */
    if (socket_handle > 0)
    {
        emscripten_websocket_delete(
            socket_handle
        );

        socket_handle = 0;
    }

    return EM_TRUE;
}

/*
 * Create the WebSocket connection.
 */
void Network_Init(
    const char *server_url
)
{
    /*
     * Reset the previous network state.
     */
    connected = false;

    local_player_id = -1;

    remote_player.active = false;

    /*
     * Make sure this browser supports WebSockets.
     */
    if (!emscripten_websocket_is_supported())
    {
        printf(
            "[Network] WebSockets are not supported.\n"
        );

        return;
    }

    /*
     * Configure the WebSocket connection.
     */
    EmscriptenWebSocketCreateAttributes attributes;

    emscripten_websocket_init_create_attributes(
        &attributes
    );

    attributes.url = server_url;

    attributes.protocols = NULL;

    attributes.createOnMainThread = EM_TRUE;

    /*
     * Create the actual WebSocket.
     */
    socket_handle =
        emscripten_websocket_new(
            &attributes
        );

    if (socket_handle <= 0)
    {
        printf(
            "[Network] Failed to create WebSocket.\n"
        );

        socket_handle = 0;

        return;
    }

    /*
     * Register our callback functions.
     */
    emscripten_websocket_set_onopen_callback(
        socket_handle,
        NULL,
        OnOpen
    );

    emscripten_websocket_set_onmessage_callback(
        socket_handle,
        NULL,
        OnMessage
    );

    emscripten_websocket_set_onerror_callback(
        socket_handle,
        NULL,
        OnError
    );

    emscripten_websocket_set_onclose_callback(
        socket_handle,
        NULL,
        OnClose
    );

    printf(
        "[Network] Attempting connection to %s...\n",
        server_url
    );
}

/*
 * Send our local player's current position
 * and facing direction to the server.
 */
void Network_SendPlayerState(
    float x,
    float y,
    float angle
)
{
    /*
     * Don't attempt to send anything if
     * the WebSocket isn't connected.
     */
    if (
        !connected ||
        socket_handle <= 0
    )
    {
        return;
    }

    /*
     * Build our state message.
     *
     * Example:
     *
     * STATE|520.00|360.00|1.5708
     */
    char message[128];

    snprintf(
        message,
        sizeof(message),
        "STATE|%.2f|%.2f|%.4f",
        x,
        y,
        angle
    );

    /*
     * Send the message through the WebSocket.
     */
    EMSCRIPTEN_RESULT result =
        emscripten_websocket_send_utf8_text(
            socket_handle,
            message
        );

    if (result != EMSCRIPTEN_RESULT_SUCCESS)
    {
        printf(
            "[Network] Failed to send player state.\n"
        );
    }
}

/*
 * Return whether we're connected.
 */
bool Network_IsConnected(void)
{
    return connected;
}

/*
 * Return the ID assigned to this browser/client.
 */
int Network_GetLocalPlayerId(void)
{
    return local_player_id;
}

/*
 * Give the rest of the game access to
 * the latest received remote-player state.
 */
bool Network_GetRemotePlayerState(
    RemotePlayerState *out_state
)
{
    if (
        !remote_player.active ||
        out_state == NULL
    )
    {
        return false;
    }

    *out_state = remote_player;

    return true;
}

/*
 * Close the WebSocket connection.
 */
void Network_Shutdown(void)
{
    if (socket_handle > 0)
    {
        emscripten_websocket_close(
            socket_handle,
            1000,
            "Client shutting down"
        );
    }
}
