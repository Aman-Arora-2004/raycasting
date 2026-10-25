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
 */

const WebSocket = require("ws");

const PORT = 8081;

const server = new WebSocket.Server({
    port: PORT
});

let nextPlayerId = 1;

console.log(
    `[Server] WebSocket server running on ws://localhost:${PORT}`
);

server.on("connection", (socket) => {

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
    });

    socket.on("error", (error) => {

        console.error(
            `[Server] Player ${playerId} error:`,
            error.message
        );
    });
});
