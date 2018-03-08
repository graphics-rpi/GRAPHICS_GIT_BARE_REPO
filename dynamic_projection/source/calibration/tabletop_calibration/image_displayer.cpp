#include <Image.h>
#include <ImageOps.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include "util.h"
#include "Vector3.h"
#include "Image.h"
#include <SDL/SDL.h>
#include <fstream>
#define GL_GLEXT_PROTOTYPES
#include <SDL/SDL_opengl.h>
#define GLX_GLXEXT_PROTOTYPES
#include <GL/glx.h>
#include <GL/glxext.h>
#ifdef FIREWIRE_CAMERA
#include <PointGreyCamera.hpp>
#endif  // #ifdef FIREWIRE_CAMERA

#include <X11/Xlib.h>


void cleanup(){
  SDL_Quit();
}

void draw_8bit_image(SDL_Surface *screen, Image<byte> image, int level, int rows, int cols){
  if (screen->format->BytesPerPixel != 4){
    FATAL_ERROR("wrong screen format\n");
  }
  if ( SDL_MUSTLOCK(screen) ) {
    if ( SDL_LockSurface(screen) < 0 ) {
      FATAL_ERROR("can't lock screen\n");
      return;
    }
  }
  
  for (int row=0; row<image.getRows()&& row<rows; row++){
    for (int col=0; col<image.getCols()&&col< cols; col++){
      Uint32 *bufp = (Uint32 *)screen->pixels + row*screen->pitch/4 + col;
      //byte v = int((image(row, col)?level:0) * (1-0.3*row/image.getRows()));
      byte v = int((image(row, col)?level:0) /** (1-0.3*row/image.getRows())*/);
      // partally correct for varying projector distance
      *bufp= SDL_MapRGB(screen->format, v,v, v);
    }
  } 

  if ( SDL_MUSTLOCK(screen) ) {
    SDL_UnlockSurface(screen);
  }
 
  //  SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
  SDL_Flip(screen);
}

void getRootWindowSize(const char *name, int &rows, int &cols){
  Display *disp = XOpenDisplay(name);
  int screen_number = XDefaultScreen(disp);
  rows = DisplayHeight(disp, screen_number); 
  cols = DisplayWidth(disp, screen_number); 
}

int main(int argc, char **argv){
  bool load=false;
  setenv("__GL_SYNC_TO_VBLANK", "1", 1);
  if (argc != 3){
    fprintf(stderr,
	    "\nstructured light correspondence generator \n"
	    "%s <Xdisplay> <output file> <camera_calibration.dat>\n"
	    "example:\n"
	    "  %s :1.0 output.dat camera_images/camera_calibration.dat\n",
	    "  EMPAC_mask.pbm projector_name plane\n",
	     argv[0]);
    return -1;
  }
  else if(argc == 8)
  load=true;
  setenv("DISPLAY", argv[1], 1);
  char *filename = argv[2];
  
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    exit(1);
  }
  atexit(cleanup);


  // get root window size
  int rows, cols;
  getRootWindowSize(NULL, rows, cols);
  printf("Screen size = %d, %d\n", rows, cols);

  SDL_Surface *screen = NULL;

  if (NULL == (screen = SDL_SetVideoMode(cols, rows, 0, 
                                         SDL_HWSURFACE|
                                         SDL_FULLSCREEN|
                                         SDL_DOUBLEBUF))){
    printf("Can't set OpenGL mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  // turn off mouse/keyboard grabbing
  SDL_WM_GrabInput(SDL_GRAB_OFF);

  // hide mouse cursor
  SDL_ShowCursor(0);

  //printf("Image size = %d, %d\n", image_rows, image_cols);

  int level=255;

  
  fprintf(stderr, "level = %d\n", level);
  
  Image<byte> in_image(argv[2]);

  draw_8bit_image(screen, in_image, level, rows, cols);
  
  SDL_Delay(1600);

  return 0;
}
