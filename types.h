#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>

// pi init
#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.1415926535
#endif

// size of each map cell in pixels
#define CELL_SIZE 80

// map grid dimensions
#define MAP_WIDTH 20
#define MAP_HEIGHT 10

// player render distance in pixels
#define RENDER_DISTANCE 1000

// player field of view in degrees
#define PLAYER_FOV 90

// player rotation and movement speed
#define PLAYER_ROTATION_SPEED 0.1
#define PLAYER_MOVEMENT_SPEED 10

// min distance in pixels the light source stays from the edge
// if it collides with boundary the rays disappear
#define LIGHT_MARGIN 50

// number of rays raytracing
#define RAYS_NUMBER 750

// ray thickness raytracing (keep very very low)
#define RAY_THICKNESS 1

// how many pixels each ray steps
// if 1 then its twice the calculatons needed than if its 2
#define RAY_STEP_SIZE 4

// ray color
#define COLOR_RAY 0xff3bd4ff

#define COLOR_WHITE 0xff0000

// player color
#define COLOR_PLAYER 0x11ff11

// player struct
typedef struct {
    float x;
    float y;
    float angle;
} Player;

// circle struct
typedef struct {
    double x;
    double y;
    double radius;
} Circle;

// ray struct;
typedef struct {
    double x_start, y_start, angle;
} Ray;

extern int WIDTH;		// screen width
extern int HEIGHT;		// screen height
extern int LIGHT_W;		// light width (for raytracing)
extern int LIGHT_H;		// light height (for raytracing)
extern int map[MAP_HEIGHT][MAP_WIDTH];	// 2d map init
extern Player player;		// player init

#endif
