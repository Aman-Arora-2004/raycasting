/*
 * Multiplayer WebSocket server for our CSC 466 project.
 *
 * Current responsibilities:
 * 1. Accept browser clients.
 * 2. Give every connected player a unique ID.
 * 3. Receive that player's x, y, and angle.
 * 4. Forward the state to the OTHER connected clients.
 *
 * Client sends:
 * STATE|x|y|angle
 *
 * Server forwards:
 * PLAYER|playerId|x|y|angle
 * 
 * Server sends:    WELCOME|playerId   (to a new player)
 *                  LEAVE|playerId     (to everyone when a player leaves)
 */

const WebSocket = require("ws");

const PORT = 8081;
const MAX_PLAYERS = 5;

// set to false to stop printing every position update
const LOG_STATE = true;

const server = new WebSocket.Server({
    port: PORT
});

let nextPlayerId = 1;

console.log(
    `[Server] WebSocket server running on ws://localhost:${PORT}`
);

server.on("connection", (socket) => {
    // new socket is counted in server.clients, more than MAX_PLAYERS mean its full
    if (server.clients.size > MAX_PLAYERS)
    {
        console.log("[Server] Game full, rejecting connection.");

        socket.rejected = true;
        socket.close(1013, "Server full");
        return;
    }
    const playerId = nextPlayerId++;

    socket.playerId = playerId;

    console.log(
        `[Server] Player ${playerId} connected.`
    );

    /*
     * Tell this client which player ID it was assigned.
     */
    socket.send(
        `WELCOME|${playerId}`
    );

    socket.on("message", (data) => {

        const message = data.toString();

        /*
         * Expected:
         * STATE|x|y|angle
         */
        if (!message.startsWith("STATE|"))
        {
            console.log(
                `[Server] Unknown message from Player ${playerId}: ${message}`
            );

            return;
        }

        const parts = message.split("|");

        if (parts.length !== 4)
        {
            console.log(
                `[Server] Invalid state from Player ${playerId}: ${message}`
            );

            return;
        }

        const x = Number(parts[1]);
        const y = Number(parts[2]);
        const angle = Number(parts[3]);

        if (
            !Number.isFinite(x) ||
            !Number.isFinite(y) ||
            !Number.isFinite(angle)
        )
        {
            console.log(
                `[Server] Invalid coordinates from Player ${playerId}.`
            );

            return;
        }

        //print what player sent
        if (LOG_STATE)
        {
            const degrees = (angle * 180 / Math.PI).toFixed(1);

            console.log(
                `[Server] Player ${playerId}: x=${x} y=${y} angle=${angle} (${degrees} deg)`
            );
        }

        const outgoingMessage =
            `PLAYER|${playerId}|${x}|${y}|${angle}`;

        /*
         * Forward this player's state to every OTHER client.
         */
        server.clients.forEach((client) => {

            if (
                client !== socket &&
                client.readyState === WebSocket.OPEN
            )
            {
                client.send(outgoingMessage);
            }
        });
    });

    socket.on("close", () => {

        console.log(
            `[Server] Player ${playerId} disconnected.`
        );

        // tell every other client player left
        server.clients.forEach((client) => {

            if (client.readyState === WebSocket.OPEN)
            {
                client.send(`LEAVE|${playerId}`);
            }
        });
    });

    socket.on("error", (error) => {

        console.error(
            `[Server] Player ${playerId} error:`,
            error.message
        );
    });
});
