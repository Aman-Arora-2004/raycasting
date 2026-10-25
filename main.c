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
    {1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0, 0,0,0,1,1,0,0,0,0,0, 0,0,0,1,1,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,1,1,1,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,1,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,1,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0, 0,0,1,0,0,0,0,1,0,0, 0,0,0,0,0,1,1,0,0,1},
    {1,0,1,0,0,0,0,0,0,0, 0,0,1,0,0,0,0,1,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,1,1,0, 0,0,0,0,0,0,0,0,0,0, 0,1,1,1,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,0,0,0,0,0, 0,0,0,0,1,1,0,0,0,0, 0,0,0,0,0,1,1,0,0,1},
    {1,0,0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,1,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,1,0, 0,0,0,0,0,0,0,0,0,0, 0,1,1,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,0,0,0,0, 0,0,0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,1,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,1,1,1,0,0,0,0,0,0, 0,0,1,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,1,1,0,0, 0,0,0,0,1,1,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0, 0,0,0,1,0,0,0,0,0,0, 0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1}
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
    
    
    // get every active remote player once per frame
    // used for the 3D sprites and for the minimap dots
    
    RemotePlayerState remote_states[MAX_REMOTE_PLAYERS];
    int remote_count = Network_GetRemotePlayers(
        remote_states,
        MAX_REMOTE_PLAYERS
    );

    // draw each remote player as a sprite in the 3D view
    for (int i = 0; i < remote_count; i++)
    {
        Player remote_player =
        {
            remote_states[i].x,
            remote_states[i].y,
            remote_states[i].angle
        };

        DrawRemotePlayerSprite(
            psurface,
            player,
            remote_player,
            remote_states[i].player_id
        );
    }

    // Draw the map and local player.
    DrawMiniMap(psurface);

    // draw each remote player on the minimap.
    for (int i = 0; i < remote_count; i++)
    {
        Player remote_player =
        {
            remote_states[i].x,
            remote_states[i].y,
            remote_states[i].angle
        };


        DrawRemotePlayerOnMiniMap(
            psurface,
            remote_player,
            remote_states[i].player_id
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
