#include "raycasting.h"
#include "raytracing.h"
#include <math.h>

void DrawMiniMap(SDL_Surface *psurface)
{ 
	//scale factor for the minimap, each map cell is 10x10 pixels
	int minimap_scale = 10;
	
	// offset from the bottom left corner of the screen
	int offset_x = 10;
	int offset_y = HEIGHT - (MAP_HEIGHT * minimap_scale) - 10;
	
	// draw a dark background behind the minimap with 2px padding
	SDL_Rect bg = {offset_x - 2,
		offset_y - 2,
		MAP_WIDTH * minimap_scale + 4, 
		MAP_HEIGHT * minimap_scale + 4
	};
    	SDL_FillRect(psurface, &bg, 0x222222);
    	
	// draw each map cell, white for walls, dark gray for empty
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for(int x = 0; x < MAP_WIDTH; x++)
		{
			SDL_Rect cell = {
				offset_x + x * minimap_scale, 
				offset_y + y * minimap_scale,
				minimap_scale - 1,
				minimap_scale -1,
			};
			Uint32 color = (map[y][x] == 1) ? 0xffffff : 0x333333;
			SDL_FillRect(psurface, &cell, color);	
		}
	}
	
	// draw the player as a small green dot, scaled to minimap coordinates
	SDL_Rect player_dot = {
		offset_x + (int)(player.x / CELL_SIZE * minimap_scale) - 2,
        	offset_y + (int)(player.y / CELL_SIZE * minimap_scale) - 2,
        	4, 4
	};
	SDL_FillRect(psurface, &player_dot, COLOR_PLAYER);
}

// gets player distance from walls using angle
double GetDistance(Player player, double angle)
{
	double ray_dir_x = cos(angle);
	double ray_dir_y = sin(angle);
	
	// which cell player is in	
	int cell_x = (int)(player.x / CELL_SIZE);
    	int cell_y = (int)(player.y / CELL_SIZE);	
	
	// how far along the ray to cross one full cell in x or y
    	double delta_dist_x = fabs(1.0 / ray_dir_x);
    	double delta_dist_y = fabs(1.0 / ray_dir_y);
	
	// direction to step in x and y (+1 or -1)
    	int step_x, step_y;

    	// distance to the first x and y grid line
    	double side_dist_x, side_dist_y;
	
	// if ray points left, step left and measure distance to the left cell boundary
	if (ray_dir_x < 0)
    	{
        	step_x = -1;
        	side_dist_x = (player.x / CELL_SIZE - cell_x) * delta_dist_x * CELL_SIZE;
    	}
    	else
    	// if ray points right, step right and measure distance to the right cell boundary
    	{
        	step_x = 1;
        	side_dist_x = (cell_x + 1.0 - player.x / CELL_SIZE) * delta_dist_x * CELL_SIZE;
    	}
	// if ray points up, step up and measure distance to the top cell boundary
    	if (ray_dir_y < 0)
    	{
        	step_y = -1;
        	side_dist_y = (player.y / CELL_SIZE - cell_y) * delta_dist_y * CELL_SIZE;
    	}
    	else
    	// if ray points down, step down and measure distance to the bottom cell boundary
    	{
        	step_y = 1;
        	side_dist_y = (cell_y + 1.0 - player.y / CELL_SIZE) * delta_dist_y * CELL_SIZE;
    	}
    	// step until we hit a wall
    	int hit_side; // what side of wall is hit 0 = x side, 1 = y side
    	while (1)
    	{
    		// advance to the nearest grid boundary, x or y	
        	if (side_dist_x < side_dist_y)
        	{
            		side_dist_x += delta_dist_x * CELL_SIZE;
            		cell_x += step_x;
            		hit_side = 0;
        	}
        	else
        	{
            		side_dist_y += delta_dist_y * CELL_SIZE;
            		cell_y += step_y;
            		hit_side = 1;
        	}
		
		// if the ray has gone out of bounds, return -1 (no wall hit)
        	if (cell_x < 0 || cell_y < 0 || cell_x >= MAP_WIDTH || cell_y >= MAP_HEIGHT)
            	return -1;

        	if (map[cell_y][cell_x] == 1)
        	{
            	// compute exact distance to the wall using DDA step count
            	double dist;
            	if (hit_side == 0)
                	dist =(cell_x-player.x / CELL_SIZE + (1-step_x) / 2.0)/ray_dir_x *CELL_SIZE;
            	else
                	dist =(cell_y-player.y / CELL_SIZE + (1-step_y) / 2.0)/ray_dir_y *CELL_SIZE;
            	return dist;
        }
    }
}

void DrawVerticalLine(SDL_Surface *psurface, double height, double x, Uint32 color, double column_width)
// draws a single vertical line on the surface (called from DrawFOV)
{
	// center the wall slice vertically on the screen
	SDL_Rect vertical_rect = {(int)x, 
		(HEIGHT / 2) - ((int)height / 2), 
		(int)column_width + 1, 
		(int)height
	};
	SDL_FillRect(psurface,&vertical_rect, color);
}

Uint32 GetShadedColor(double distance)
// calculates wall shading based on distance from player
{
	// brightness from 255 to 0. distance / render_distance normalizes it to 0.0 - 1.0
	// fmin clamps so it never goes over 1 (brightness cant be negative)
    	int brightness = (int)(255.0 * (1.0 - fmin(distance / RENDER_DISTANCE, 1.0)));
    	
    	//pack brightness equally int r, g,b channels
    	// gray shade because all values are the same in eery channel
    	return (brightness << 16) | (brightness << 8) | brightness;
}

void DrawFOV(SDL_Surface *surface, Player player)
{
	// fov in radians
	double fov_rad = PLAYER_FOV * (M_PI / 180);
	double half_fov = fov_rad / 2.0;
	
	double num_columns = WIDTH / 2; 
	// get the angle step by dividing the fov by the width of screen_window
	double angle_step = fov_rad / num_columns;
	// get the width of each column
	double column_width = (double)WIDTH / num_columns;

	for (int i = 0; i < num_columns; i++)
	{
		// how much to offset the ray angle
		// when rendering 3d column, 0 is on the far left of screen, to render from left to right 
		// must subtract half_fov to make the position the leftmost val of fov
		// as loop increments i * angle_step ray sweeps across center and ends at +half_fov
		double ray_angle_offset = -half_fov + (i * angle_step);
		// calculate the absolute angle by taking consideration of offset
		double absolute_ray_angle = player.angle + ray_angle_offset;
		
		// get the player distance from particular wall using absolute_ray_angle
		double raw_distance = GetDistance(player, absolute_ray_angle);
		if (raw_distance <= 0) continue;
		
		// get the corrected distance by adjusting raw distance with ray_angle_offset
		// gets rid of fisheye perspective
		double corrected_distance = raw_distance * cos(ray_angle_offset);
		if (corrected_distance < 1.0) corrected_distance = 1.0;
		
		// calculate the height of the wall based on cell size, map height 
		//and the corrected distance from particular column
		double wall_height = (CELL_SIZE * HEIGHT) /corrected_distance;
		
		//gets the width of the vertical line to draw
		double screen_x = i * column_width;
		
		// gets the wall color based on corrected distance
		Uint32 color = GetShadedColor(corrected_distance);
		DrawVerticalLine(surface, wall_height, screen_x, color, column_width);
	}		
}

void DrawCeiling(SDL_Surface *surface)
{	
	SDL_LockSurface(surface);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    // get the light simulation buffer from the raytracer
    Uint32 *light_buffer = GetLightBuffer();
    // scale the light buffer to fill the top half of the screen
    for (int y = 0; y < HEIGHT / 2; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int lx = (x * LIGHT_W) / WIDTH;
            int ly = (y * LIGHT_H) / (HEIGHT / 2);
            pixels[y * (surface->pitch / 4) + x] = light_buffer[ly * LIGHT_W + lx];
        }
    }
    SDL_UnlockSurface(surface);
}
