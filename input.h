#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include <SDL2/SDL.h>
#include <emscripten.h>
#include <emscripten/html5.h>

extern float touch_forward;
extern float touch_strafe;

EMSCRIPTEN_KEEPALIVE void set_touch_move(float forward, float strafe);
EMSCRIPTEN_KEEPALIVE void set_touch_rotation(float delta);

// checks if given world position is a valid position for player to move to
// returns false if position is out of bounds or inside a wall cell
//

// divides absolute float coordinates (x, y) by CELL_SIZE and 
// casts them to integers to identify targeted map cell

// evaluates whether the calculated cell indices fall outside the grid bounds 
// (less than zero or exceeding MAP_WIDTH/MAP_HEIGHT)

// returns false immediately if the coordinates point outside the map 
// array to prevent illegal memory reads or out-of-bounds errors

// reads the 2D map matrix at the target position, returning true 
// if the space is empty (0) and false if blocked by a wall
bool CanMoveTo(float x, float y);


// reads the current keyboard state and moves the player accordingly
// WASD moves relative to the players current facing angle
// collision is checked before each move via CanMoveTo

// fetches a pointer to  global SDL keyboard state array to 
// inspect active key states simultaneously

// a key shifts the player's current angle backwards by 90 degrees 
// (-M_PI / 2) to calculate and apply a leftward movement vector

// d key shifts the player's angle forwards by 90 degrees (+M_PI / 2) 
// to determine and execute a rightward movement velocity vector

// w or s keys follows the direct player.angle vector to step forwards (+)
// or subtracts the vector (-) to walk directly backward

// calls CanMoveTo for every input condition, safely committing 
// the coordinates to player.x and player.y only if the path is free
void HandleInput(void);

// emscripten mouse move callback, called by browser on every mouse movement
// rotates player based on horizontal mouse delta
// registered via emscripten_set_mousemove_callback in main

//
// reads movementX property from the mouse event struct to 
// capture horizontal pixel shifts independent of screen bounds

// multiplies the pixel delta by a small decimal scalar (0.004) 
// to smoothly update the player.angle float accumulator

// returns true to signal to the underlying platform 
// wrapper that the event was processed successfully
bool mouse_move_callback(int eventType, const EmscriptenMouseEvent *e, void *userData);

#endif
