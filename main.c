#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <emscripten.h>
#include <emscripten/html5.h>

#include "types.h"
#include "raytracing.h"
#include "raycasting.h"
#include "input.h"
#include "network.h"

// map layout
int WIDTH  = 1600;
int HEIGHT = 800;
int LIGHT_W;
int LIGHT_H;

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// initialize player and app state
Player player = {CELL_SIZE * 6.5f, CELL_SIZE * 4.5f, 0.0f * (M_PI / 180.0f)};
SDL_Window *pwindow = NULL;
SDL_Surface *psurface = NULL;
bool app_running = 1;

/*
 * Time when we last sent the local player's state
 * to the multiplayer server.
 */
Uint32 last_network_send = 0;

void main_loop()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                app_running = 0;
                break;
        }
    }

    HandleInput();

    /*
     * Send the local player's position and angle at
     * approximately 20 updates per second (every 50 ms).
     */
    if (Network_IsConnected())
    {
        Uint32 current_time = SDL_GetTicks();

        if (current_time - last_network_send >= 50)
        {
            Network_SendPlayerState(
                player.x,
                player.y,
                player.angle
            );

            last_network_send = current_time;
        }
    }

    UpdateLightSim();
    SDL_FillRect(psurface, NULL, 0x00000000);

    DrawCeiling(psurface);

    SDL_Rect floor_rect = {0, HEIGHT / 2, WIDTH, HEIGHT / 2};
    SDL_FillRect(psurface, &floor_rect, 0x2d2d2d);

    DrawFOV(psurface, player);

    // Draw the map and local player.
    DrawMiniMap(psurface);

    /*
     * Get the latest remote-player state received from
     * the server and draw that player on the minimap.
     */
    RemotePlayerState remote_state;

    if (Network_GetRemotePlayerState(&remote_state))
    {
        Player remote_player =
        {
            remote_state.x,
            remote_state.y,
            remote_state.angle
        };

        DrawRemotePlayerOnMiniMap(
            psurface,
            remote_player
        );
    }

    SDL_UpdateWindowSurface(pwindow);
}

int main(int argc, char *argv[])
{
    // Read actual browser canvas size
    WIDTH = EM_ASM_INT({
        return document.getElementById('canvas').width;
    });

    HEIGHT = EM_ASM_INT({
        return document.getElementById('canvas').height;
    });

    if (WIDTH <= 0 || HEIGHT <= 0)
    {
        WIDTH = 1600;
        HEIGHT = 800;
    }

    LIGHT_W = WIDTH;
    LIGHT_H = HEIGHT / 2;

    InitLightSim(LIGHT_W, LIGHT_H);

    SDL_Init(SDL_INIT_VIDEO);

    pwindow = SDL_CreateWindow(
        "Aman Arora Raycaster",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0
    );

    psurface = SDL_GetWindowSurface(pwindow);

    /*
     * Connect this browser client to the local
     * multiplayer WebSocket server.
     */
    Network_Init("ws://localhost:8081");

    emscripten_set_mousemove_callback(
        "#canvas",
        NULL,
        true,
        mouse_move_callback
    );

    emscripten_set_main_loop(main_loop, 0, 1);

    return 0;
}
