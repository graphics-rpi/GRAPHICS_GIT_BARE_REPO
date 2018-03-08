#ifdef FIREWIRE_CAMERA
#include <PointGreyCamera.hpp>
#endif // #ifdef FIREWIRE_CAMERA

#ifdef GIGE_CAMERA
#include "GigEVisionCamera.hpp"
#endif // #ifdef GIGE_CAMERA

#include <stdio.h>
#include <SDL/SDL.h>
#include "CameraAPI.hpp"
#include <X11/Xlib.h>

void getRootWindowSize(const char *name, int &rows, int &cols)
{
  Display *disp = XOpenDisplay(name);
  int screen_number = XDefaultScreen(disp);
  rows = DisplayHeight(disp, screen_number); 
  cols = DisplayWidth(disp, screen_number); 
}


void draw_rgb_image(SDL_Surface *screen, Image<sRGB> image)
{
  if (screen->format->BytesPerPixel != 4)
  {
    FATAL_ERROR("wrong screen format\n");
  }
  if ( SDL_MUSTLOCK(screen) ) 
  {
    if ( SDL_LockSurface(screen) < 0 ) 
    {
      FATAL_ERROR("can'tlocl screen\n");
      return;
    }
  }
  
  for (int row=0; row<image.getRows(); row++)
  {
    for (int col=0; col<image.getCols(); col++)
    {
      Uint32 *bufp = (Uint32 *)screen->pixels + row*screen->pitch/4 + col;
      *bufp= SDL_MapRGB(screen->format, image(row,col).r(),
			image(row,col).g(), image(row,col).b());
    }
  } 

  if ( SDL_MUSTLOCK(screen) ) {
    SDL_UnlockSurface(screen);
  }
 
  //  SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
  SDL_Flip(screen);
}

void cleanup()
{
  SDL_Quit();
}

bool file_exists(const char *filename)
{
  FILE *fp = fopen(filename, "rt");
  if (NULL == fp){
    return false;
  } 
  else 
  {
    fclose(fp);
    return(true);
  }
}


int main(int argc, char **argv)
{

#ifdef GIGE_CAMERA
  GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER8> camera;

  //#define CAM_TESTING
#ifdef CAM_TESTING
  int shutter = 500000;
  int gain = 0;
#else
  //int shutter = 13000;
int shutter = 60000;
  int gain = 0;
#endif

  //int cam_rows = 960;
  //int cam_cols = 960;
  //int row_offset = 0;
  //int col_offset = 145;
//If you just want the table
//  int cam_rows = 1456;
//  int cam_cols = 1600;
//  int row_offset = 0;
//  int col_offset = 300;
  int cam_rows = 1456;
  int cam_cols = 1936;
  int row_offset = 0;
  int col_offset = 0;
  camera.SetExposure(shutter);
  camera.SetGain(gain);
  camera.SetWhiteBalance(105,400);
  camera.SetBinning(1, 1);
  camera.SetROI(row_offset, col_offset, cam_rows, cam_cols);
  camera.InitContinuousCapture();
#else //Vimba camera
  std::cout<<"Using vimba"<<std::endl;
  VimbaCamera<sRGB> camera;
  long long w, h;
  camera.getSensorWidthAndHeight(w,h);
  camera.setOffset(00,00);
  camera.setSize(1936,1456);
#endif // #ifdef GIGE_CAMERA

  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) 
  {
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
                                         SDL_NOFRAME|
                                         SDL_DOUBLEBUF)))
  {
    printf("Can't set SDL mode: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }
  

  // turn off mouse/keyboard grabbing
  SDL_WM_GrabInput(SDL_GRAB_ON);

  // hide mouse cursor
  SDL_ShowCursor(0);
      int scaledownFactor=1;
  while (1)
  {

    // preview mode
    while (1)
    {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
	      switch (event.type) 
        {
	        case SDL_QUIT:
	          exit(0);
	          break;
	        case SDL_KEYDOWN:
	          switch(event.key.keysym.sym)
            {
	            case SDLK_ESCAPE: case 'q':
	              exit(0);
	              break;
	            default:
	              goto PREVIEW_DONE;
	          }
	      }
      }

      // paint captured image into centered rectangle on screen
      Image<sRGB> cap = camera.GetNextFrame();
      Image<sRGB> frame(rows, cols);
      for (int row=0; row<rows; row++){
	      for (int col=0; col<cols; col++){
	        frame(row, col) = sRGB(127);
	      }
      }

      if(frame.getRows() - cap.getRows() >= 0||frame.getRows() - cap.getRows() >= 0)
      {
        scaledownFactor=1;
      }
      else //Scales down the image to display by a factor of 2 if it doesn't fit on screen
      {
        scaledownFactor=2;
        assert(frame.getRows() - cap.getRows()/2 >= 0);
        assert(frame.getRows() - cap.getRows()/2 >= 0);
      }
      int roff = (frame.getRows() - cap.getRows()/scaledownFactor)/2;
      int coff = (frame.getCols() - cap.getCols()/scaledownFactor)/2;
      //assert(frame.getRows() - cap.getRows() >= 0);
      //assert(frame.getRows() - cap.getRows() >= 0);
      std::cout<<"frame size: "<<frame.getCols()<<" "<<frame.getRows()<<std::endl
               <<"cap   size: "<<cap.getCols()<<" " <<cap.getRows()<<std::endl;
      for (int row=0; row*scaledownFactor<cap.getRows(); row++)
      {
	      for (int col=0; col*scaledownFactor<cap.getCols(); col++)
        {
	        frame(roff+row, coff+col) = cap(row*scaledownFactor, col*scaledownFactor);
	      }
      }
      
      draw_rgb_image(screen, frame);
    }
    
  PREVIEW_DONE:

    int data_count = 0;
    char filename[1024];
    do {
      snprintf(filename, 1024, "camera_images/image%03d.ppm", data_count++);
    } while (file_exists(filename));

    
    Image <sRGB> image = camera.GetNextFrame();
    
    if (argc == 2)
    {
      image.write(argv[1]);
      exit(0);
    } 
    else 
    {
      image.write(filename);
    }

    // "blink" border to indicate that frame has been captured
    Image<sRGB> frame(rows, cols);
    for (int row=0; row<rows; row++)
    {
      for (int col=0; col<cols; col++)
      {
	      frame(row, col) = sRGB(0, 0, 0);
      }
    }
    int roff = (frame.getRows() - image.getRows()/scaledownFactor)/2;
    int coff = (frame.getCols() - image.getCols()/scaledownFactor)/2;
    for (int row=0; row*scaledownFactor<image.getRows(); row++)
    {
      for (int col=0; col*scaledownFactor<image.getCols(); col++)
      {
	      frame(roff+row, coff+col) = image(row*scaledownFactor, col*scaledownFactor);
      }
    }
    
    draw_rgb_image(screen, frame);

  }

  return 0;
}
