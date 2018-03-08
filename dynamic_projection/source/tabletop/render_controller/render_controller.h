//#define FILE_LOCATION "../projector_config.txt"
#define MPI_HOSTS_FILE "/home/grfx/hosts2"
#include "RemoteDisplay.h"



class RenderController
{
  public:
    RenderController()
    {
      //blending = true;
      volume_texture_enabled = false;
      puzzle=false;
      red_green_walls=false;
      pong = false;
      pen_demo = false;
      //multidisplay=false;
      space_invaders = false;
      //army = false;
      send_obj = true;
      send_mtl = true;
    
    }
//    void cleanup(){}
    void run();
    //bool blending;
    bool volume_texture_enabled;
    bool puzzle;
    bool red_green_walls;
    bool pong;
    bool pen_demo;
    //bool multidisplay;
    bool space_invaders;
    //bool army;
    bool send_obj;
    bool send_mtl;
    RemoteDisplay *disp;
  private:

};

