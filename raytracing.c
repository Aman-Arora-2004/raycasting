#include "raytracing.h"
#include <math.h>

// moving light source that emits rays
Circle light_source;

// moving circle that blocks rays and casts a shadow
Circle shadow_circle;
// array of all rays emitted from light source each frame
Ray rays[RAYS_NUMBER];
// heap allocated pixel buffer for light simulation, dimensions LIGHT_W x LIGHT_H
Uint32 *light_buffer = NULL;

// velocity of light source in pixels per frame
double light_dx, light_dy;
// velocity of shadow circle in pixels per frame
double shadow_dx, shadow_dy;

// returns a pointer to the light buffer so other systems can read it
Uint32 *GetLightBuffer(void)
{
    	return light_buffer;
}

// generates RAYS_NUMBER rays evenly spread in all 
// directions from the given circles center
void GenerateRays(Circle circle, Ray rays[RAYS_NUMBER])
{
    	for (int i = 0; i < RAYS_NUMBER; i++) {
    		// evenly distribute ray angles across a full 360 degrees
        	double angle = ((double)i / RAYS_NUMBER) * 2 * M_PI;
        	rays[i] = (Ray){circle.x, circle.y, angle};
    }
}

void UpdateLightBuffer()
{
	// fill the entire buffer with the background light color
    	for (int i = 0; i < LIGHT_W * LIGHT_H; i++)
        	light_buffer[i] = 0xffe16941;
	
	// precompute squared radius to avoid sqrt in the inner loop
    	double draw_r2 = (shadow_circle.radius - 1) * (shadow_circle.radius - 1);
    	double radius_squared = shadow_circle.radius * shadow_circle.radius;

	// march each ray forward one step at a time until
	// it hits the shadow circle or goes out of bounds
    	for (int i = 0; i < RAYS_NUMBER; i++) {
        	Ray ray = rays[i];
        	double x = ray.x_start, y = ray.y_start;
        	while (1) {
        		// advance the ray by RAY_STEP_SIZE pixels in its direction
            		x += cos(ray.angle) * RAY_STEP_SIZE;
			y += sin(ray.angle) * RAY_STEP_SIZE;
			// stop if the ray has left the buffer bounds
			if (x < 0 || x >= LIGHT_W || y < 0 || y >= LIGHT_H) break;
			// stop if the ray has entered the shadow circle
	 		double dx = x - shadow_circle.x, dy = y - shadow_circle.y;
			if (dx*dx + dy*dy < radius_squared) break;
				int bx = (int)x, by = (int)y;
			// draw a thick dot at the ray's current position using RAY_THICKNESS
		    	for (int ty = -(int)RAY_THICKNESS; ty <= (int)RAY_THICKNESS; ty++) {
		        	for (int tx = -(int)RAY_THICKNESS; tx <= (int)RAY_THICKNESS; tx++) {
		          		int px = bx + tx, py = by + ty;
		            		if (px >= 0 && px < LIGHT_W && py >= 0 && py < LIGHT_H)
		                		light_buffer[py * LIGHT_W + px] = COLOR_RAY;
                			}
            			}
        	}
    	}
	// draw the shadow circle as a filled black circle
    	for (int y = (int)(shadow_circle.y - shadow_circle.radius); y < (int)(shadow_circle.y + shadow_circle.radius); y++) {
        	for (int x = (int)(shadow_circle.x - shadow_circle.radius); x < (int)(shadow_circle.x + shadow_circle.radius); x++) {
            		if (x < 0 || x >= LIGHT_W || y < 0 || y >= LIGHT_H) continue;
            		double dx = x - shadow_circle.x, dy = y - shadow_circle.y;
            		if (dx*dx + dy*dy < draw_r2)
                		light_buffer[y * LIGHT_W + x] = 0xff000000;
        	}
    	}
	
	// draw the light source as a small filled circle in the background color
	// drawn last so it always appears on top of the rays
    	double src_r2 = 15.0 * 15.0;
    	for (int y = (int)(light_source.y - 15); y < (int)(light_source.y + 15); y++) {
        	for (int x = (int)(light_source.x - 15); x < (int)(light_source.x + 15); x++) {
            		if (x < 0 || x >= LIGHT_W || y < 0 || y >= LIGHT_H) continue;
            		double dx = x - light_source.x, dy = y - light_source.y;
            		if (dx*dx + dy*dy < src_r2)
                		light_buffer[y * LIGHT_W + x] = 0xffe16941;
        	}
    	}
}

void UpdateLightSim()
{
	// move the light source and bounce it off the edges of the buffer
    	light_source.x += light_dx;
    	light_source.y += light_dy;
    	if (light_source.x < LIGHT_MARGIN) { light_source.x = LIGHT_MARGIN; light_dx = fabs(light_dx); }
    	if (light_source.x > LIGHT_W - LIGHT_MARGIN) { light_source.x = LIGHT_W - LIGHT_MARGIN; light_dx = -fabs(light_dx); }
    	if (light_source.y < LIGHT_MARGIN) { light_source.y = LIGHT_MARGIN; light_dy = fabs(light_dy); }
    	if (light_source.y > LIGHT_H - LIGHT_MARGIN) { light_source.y = LIGHT_H - LIGHT_MARGIN; light_dy = -fabs(light_dy); }
	
	// move the shadow circle and bounce it off the edges of the buffer
    	shadow_circle.x += shadow_dx;
    	shadow_circle.y += shadow_dy;
    	if (shadow_circle.x - shadow_circle.radius < 0 || shadow_circle.x + shadow_circle.radius > LIGHT_W) shadow_dx = -shadow_dx;
    	if (shadow_circle.y - shadow_circle.radius < 0 || shadow_circle.y + shadow_circle.radius > LIGHT_H) shadow_dy = -shadow_dy;
	
	// regenerate rays from the new light source position and redraw the buffer
    	GenerateRays(light_source, rays);
    	UpdateLightBuffer();
}

void InitLightSim(int w, int h)
{
	// set velocities as a fraction of the buffer dimensions so speed scales with resolution
    	light_dx = w * 0.002;
    	light_dy = h * 0.004;
    	shadow_dx = w * 0.006;
    	shadow_dy = h * 0.005;
	
	// use the average of width and height to scale the shadow circle radius
    	double avg = (w + h) * 0.5;
    	
    	// allocate the pixel buffer for the light simulation
    	light_buffer = (Uint32 *)malloc(w * h * sizeof(Uint32));
    	
    	// place the light source near the top left
    	light_source = (Circle){w * 0.2, h * 0.1, 1};
    	
    	// place the shadow circle in the center, capped at radius 80
    	shadow_circle = (Circle){w * 0.4, h * 0.5, fmin(avg * 0.15, 80)};
	
	// do an initial render so the buffer is ready before the first frame
    	GenerateRays(light_source, rays);
    	UpdateLightBuffer();
}
