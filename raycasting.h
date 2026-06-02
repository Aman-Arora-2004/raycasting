#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "types.h"
#include <SDL2/SDL.h>

// renders a scaled minimap on SDL surface
// shows wall cells, empty cells and the players current pos

// scales the map down(10x) and offsets it to the bottom-left 
// corner of the window also draws a dark gray border/background rectangle

// iterates through 2d map array row-by-row and column-by-column 
// to render individual grid squares (cells)

// draws wall cells (value 1) as solid white and empty spaces as dark charcoal
// leaving a 1 pixel gap between cells for a visible grid outline

// converts players continuous, large scale coordinates into small minimap 
// coordinates drawing player as a custom 4x4 square dot
void DrawMiniMap(SDL_Surface *psurface);

// casts a single ray from the player at 
// specific angle into a grid based map using DDA

// first converts the angle into a directional vector (ray_dir) 
// and finds the players initial grid cell

// calculates delta_dist (how far the ray must travel to cross
// exactly one full horizontal or vertical grid unit)

// determines if the ray shoots left/right (step_x) and up/down (step_y)
// then computes the initial distance to the very first grid lines (side_dist)

// increments through the grid one cell at a time
// It always hops to the closest next grid line 
// (comparing side_dist_x and side_dist_y) until it 
// encounters a wall (value 1) or leaves the map boundaries.

// upon collision it uses perpendicular ray length to compute and return 
// the exact distance to the wall surface which prevents fish-eye distortion
double GetDistance(Player player, double angle);


// renders single vertical strip on screen to construct 3D raycasted view
// stretches the column symmetrically around vertical screen center to simulate wall height
//
// uses the calculated x coordinate to set the horizontal position of the wall slice

// subtracts half of the line's height from the screens vertical center 
// (HEIGHT / 2) to anchor the column top correctly

// adds a 1 pixel padding to the column width selection to 
// eliminate gaps between neighboring vertical wall slices

// calls SDL_FillRect to push the solid color 
//rectangle onto the target rendering surface
void DrawVerticalLine(SDL_Surface *psurface, double height, double x, Uint32 color, double column_width);


// calculates wall shading based on  playerss distance to create depth
// normalizes distance against max render limit to generate clean grayscale value
//
// 
// calculates brightness from 255 down to 0 by dividing the 
// current distance by the RENDER_DISTANCE to create a 0.0 to 1.0 ratio

// uses fmin to clamp the ratio so it never goes over 1.0
// ensuring the final brightness value cannot drop below zero

// replicates the final brightness integer across the 
// red, green, and blue channels equally since identical values produce gray

// shifts and combines the channel values using bitwise OR operations into a single packed Uint32 color structure.
Uint32 GetShadedColor(double distance);


// function projects multiple rays across the players field of 
// view to render a 3D scene from left to right

// converts 2D distance measurements into scaled vertical 
// wall columns to construct the pseudo-3D perspective
//
// converts the players field of view into radians (fov_rad)
// divides it in half and calculates both the angle step and individual column width

// loops through each screen column, subtracting half_fov so  
// sweep starts at leftmost edge and increments across to rightmost edge

// calls GetDistance using  absolute ray angle 
// then multiplies the result by cos(ray_angle_offset) 
// to remove the fisheye distortion

// computes the 3d wall slice height using the cell size, screen 
// height, corrected distance, then grabs a shaded color based on depth

// identifies the precise horizontal screen placement (screen_x) 
// and passes the parameters to DrawVerticalLine to draw the individual strip
void DrawFOV(SDL_Surface *surface, Player player);


// renders  top half of the screen by drawing the ceiling with custom 2d raytracing
// maps surface pixels to pre-calculated light buffer to apply lighting across the ceiling area
//
// 
// invokes SDL_LockSurface to safely secure direct pixel-level memory access before writing data

// iterates vertically only up to half the screen height (HEIGHT / 2) 
// to restrict drawing strictly to the upper ceiling zone

// scales the current screen (x, y) coordinates down to match 
// the smaller dimensions of the light buffer (LIGHT_W, LIGHT_H)

// copies the color values from the light buffer directly into the
// surfaces raw pixel array, using pitch to calculate precise row offsets
void DrawCeiling(SDL_Surface *surface);

#endif
