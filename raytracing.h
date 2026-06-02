#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "types.h"
#include <SDL2/SDL.h>

// moving light source that emits rays
extern Circle light_source;

// moving circle that blocks rays and casts a shadow
extern Circle shadow_circle;

// array of all rays emitted from light source each frame
extern Ray rays[RAYS_NUMBER];

// heap allocated pixel for light simulation
extern Uint32 *light_buffer;



// returns a direct memory pointer to buffer holding pre-calculated light mapping data
//
// exposes memory address of internal 'light_buffer' array directly to caller
// allows rendering functions to read light map values without exposing variable globally
Uint32 *GetLightBuffer(void);


// populates an array of Ray structures originating from the center of a given circle
// uniformly distributes the rays across a full 360-degree 
// circle to simulate an omnidirectional light or sensor source
//

// iterates through the rays array up to RAYS_NUMBER
// to initialize every individual ray container

// divides the loop index by the total ray count and multiplies 
// by 2 * M_PI to calculate evenly spaced angles in radians

// spawns each Ray struct with its origin anchored directly to 
// the circle's center coordinates (circle.x, circle.y)

// directs the newly created ray object into the matching index of 
// the rays array to pass the completed set back to the engine
void GenerateRays(Circle circle, Ray rays[RAYS_NUMBER]);


// updates the lighting buffer by casting rays 
// outward to trace light and shadow boundaries

// fills the map with a background color, projects thick light rays, 
// blocks them using a shadow circle, and draws light source centers
//
// 
// fills the entire light_buffer with a default orange background 
// color to establish the initial illumination state

// steps each ray forward by RAY_STEP_SIZE using cosine and sine
// breaking out of the loop if it leaves the grid boundaries

// computes the distance from the rays tip to the shadow circles center
// halting the ray instantly if it enters the radius

// iterates through local pixel offsets to apply a multi-pixel brush 
// (RAY_THICKNESS), drawing thick light beams onto the buffer

// loops over a bounding box around the shadow circle
// rendering a solid black void anywhere inside its inner radius squared

// bounds a 15-pixel area around the light source coordinates
// locking down a clean circle filled back in with the original orange tone
void UpdateLightBuffer(void);


// updates physics and movement simulation for both light source and shadow-casting circle
// moves objects, handles boundary bounces, updates ray directions, and refreshes dynamic lighting buffer
//
// 
// displaces the light source position by adding the 
// velocity variables (light_dx, light_dy) to its coordinates

// checks if the light source crosses the LIGHT_MARGIN boundaries
// forcing a bounce by using fabs() to reverse velocity signs

// advances the position of the shadow-casting circle by injecting 
// its respective velocity vectors (shadow_dx, shadow_dy)

// checks if the circle's outer edge touches the buffer frame
// flipping the velocity variables (-shadow_dx) to create a rebound

// calls GenerateRays from the updated light source position 
// and triggers UpdateLightBuffer to redraw the scene
void UpdateLightSim(void);


// initializes lighting simulation parameters, object speeds, positions, and memory buffers
// sets up light source and shadow objects relative to screen size before running the initial light generation
//
// 
// multiplies the input dimensions (w, h) by decimal percentages
// to calculate responsive movement speeds for both objects

// dynamically allocates the light_buffer byte array using 
// malloc based on the calculated pixel area (w * h)

// sets up the initial light source at the upper-left area and 
// centers the shadow circle using screen percentage thresholds

// calculates a size value (15% of the average screen size) 
// and employs fmin to cap the shadow circle at a maximum of 80 pixels

// triggers GenerateRays and UpdateLightBuffer to project the starting 
// frame paths into the memory buffer immediately
void InitLightSim(int w, int h);

#endif
