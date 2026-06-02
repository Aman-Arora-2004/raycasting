#include "input.h"
#include <math.h>

float touch_forward = 0.0f;
float touch_strafe  = 0.0f;
float touch_rotation = 0.0f;

EMSCRIPTEN_KEEPALIVE void set_touch_move(float forward, float strafe) {
    touch_forward = forward;
    touch_strafe  = strafe;
    debugDiv.textContent = 'dx:' + dx.toFixed(2);
}

EMSCRIPTEN_KEEPALIVE void set_touch_rotation(float delta) {
    touch_rotation = delta;
}

// checks if the given world position is a valid position for the player to move to
bool CanMoveTo(float x, float y) 
{
	// convert world position to map cell coordinates
	int cell_x = (int)(x / CELL_SIZE);
	int cell_y = (int)(y / CELL_SIZE);
	// reject positions outside the map bounds
	if (cell_x < 0 || cell_y < 0 || cell_x >= MAP_WIDTH || cell_y >= MAP_HEIGHT)
	{
		return false;
	}
	// only allow movement into empty cells
	return map[cell_y][cell_x] == 0;
}

void HandleInput()
{
	// get a snapshot of the current keyboard state
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
 	if (keys[SDL_SCANCODE_A])
 	// strafe left: move perpendicular to facing angle, 90 degrees to left
	{
	 	float new_x = player.x + cos(player.angle - M_PI / 2) * PLAYER_MOVEMENT_SPEED;
	    	float new_y = player.y + sin(player.angle - M_PI / 2) * PLAYER_MOVEMENT_SPEED;
	    	if (CanMoveTo(new_x, new_y))
	    	{
			player.x = new_x;
			player.y = new_y;
	    	}
	}
	if (keys[SDL_SCANCODE_D])
	// strafe right: move perpendicular to facing angle, 90 degrees to right
	{
	    	float new_x = player.x + cos(player.angle + M_PI / 2) * PLAYER_MOVEMENT_SPEED;
	    	float new_y = player.y + sin(player.angle + M_PI / 2) * PLAYER_MOVEMENT_SPEED;
		if (CanMoveTo(new_x, new_y))
    		{
        		player.x = new_x;
        		player.y = new_y;
    		}
	}
	if (keys[SDL_SCANCODE_W])
	// move forward: move in the direction the player is facing
	{
		float new_x = player.x + cos(player.angle) * PLAYER_MOVEMENT_SPEED;
		float new_y = player.y + sin(player.angle) * PLAYER_MOVEMENT_SPEED;
		if (CanMoveTo(new_x, new_y))
		{
			player.x = new_x;
			player.y = new_y;
		}
	}
	if (keys[SDL_SCANCODE_S])
	// move backward: move opposite to the direction the player is facing
	{
		float new_x = player.x - cos(player.angle) * PLAYER_MOVEMENT_SPEED;
		float new_y = player.y - sin(player.angle) * PLAYER_MOVEMENT_SPEED;
		if (CanMoveTo(new_x, new_y))
		{
			player.x = new_x;
			player.y = new_y;
		}
	}
	// touch movement
	if (touch_forward != 0.0f) {
    		float new_x = player.x + cos(player.angle) * touch_forward;
    		float new_y = player.y + sin(player.angle) * touch_forward;
    	if (CanMoveTo(new_x, new_y)) { player.x = new_x; player.y = new_y; }
	}
	if (touch_strafe != 0.0f) {
    		float new_x = player.x + cos(player.angle + M_PI / 2) * touch_strafe;
    		float new_y = player.y + sin(player.angle + M_PI / 2) * touch_strafe;
    		if (CanMoveTo(new_x, new_y)) { player.x = new_x; player.y = new_y; 
    	}
}
// touch rotation
player.angle += touch_rotation;
touch_rotation = 0.0f; // consume it each frame
}

// emscripten mouse move callback, called by the browser on every mouse movement
bool mouse_move_callback(
    	int eventType,
    	const EmscriptenMouseEvent *e,
    	void *userData)
{
	// rotate the player by a small fraction of the horizontal mouse delta
	// 0.004 is the mouse sensitivity, increase to rotate faster
    	player.angle += e->movementX * 0.004f;
    	return true;
}
