#ifndef _COLOR_H_
#define _COLOR_H_

#include "../common/Image.h"
#include "point.h"


#define NUM_COLORS 10

#define BLACK_IDX    0
#define RED_IDX      1
#define ORANGE_IDX   2
#define BROWN_IDX    3
#define YELLOW_IDX   4
#define LIME_IDX     5
#define GREEN_IDX    6
#define CYAN_IDX     7
#define BLUE_IDX     8
#define MAGENTA_IDX  9

extern const std::string COLOR_KEYWORD[NUM_COLORS];
extern const sRGB COLOR_sRGB[NUM_COLORS];

// north arrow is this color
#define ARROW_COLOR_IDX ORANGE_IDX

// wall heights by color
extern const double RED_HEIGHT;
extern const double BLUE_HEIGHT;
extern const double GREEN_HEIGHT;

// wall tip extensions
extern const double wall_tips;
extern const double curved_wall_tips;

// color token markers
extern const int SPECIFIC_WALL_TOKEN_IDX;
extern const int GLOBAL_WALL_TOKEN_IDX;
extern const int FLOOR_TOKEN_IDX;
extern const int FLOOR_TOKEN_IDX2;
extern const int CEILING_TOKEN_IDX;






#endif
