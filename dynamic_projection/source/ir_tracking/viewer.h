#ifndef __VIEWER_H__
#define __VIEWER_H__

#include "../common/Image.h"


#if 1
#define IMAGE_WIDTH 1280
#define IMAGE_HEIGHT 960
#else
#define IMAGE_WIDTH 512
#define IMAGE_HEIGHT 512
#endif


int viewer_start(int argc, char** argv);
int viewer_loop();
void my_idle();
void image_to_texture(const Image<byte> &image);
void image_to_texture(const Image<sRGB> &image);


#endif 
