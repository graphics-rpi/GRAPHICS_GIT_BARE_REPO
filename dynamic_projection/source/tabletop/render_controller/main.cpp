#include "render_controller.h"
#include "../argparser.h"
ArgParser *ARGS;

int curr_frame=4;


int main(int argc, char **argv) {
  RenderController rc;
  sleep(7);
  
  ARGS = new ArgParser(argc,argv);
  
  rc.run(); 
}
