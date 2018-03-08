#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>
#include "../../calibration/planar_interpolation_calibration/tiled_display.h"

class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-not_full_screen")) {
	tiled_display.full_screen = false;
      } else if (std::string(argv[i]) == std::string("-image_filename") ||
		 std::string(argv[i]) == std::string("-i")) {
	i++;
	assert (i < argc);
	image_filename = argv[i];
      } else if (std::string(argv[i]) == std::string("-num_pieces") || 
		 std::string(argv[i]) == std::string("-n")) {
	i++;
	assert (i < argc);
	cols = atoi(argv[i]);
	i++;
	assert (i < argc);
	rows = atoi(argv[i]);
	assert (rows > 0 && cols > 0);
      } else if (std::string(argv[i]) == std::string("-demo1")) {
	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/WEF_2012.ppm";
	cols = 5;
	rows = 4;
      } else if (std::string(argv[i]) == std::string("-demo1b")) {
	//image_filename = "images/EMPAC_LOBBY_smaller.ppm";
	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/empac_ext_night_hull_smaller.ppm";
	cols = 4;
	rows = 3;
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/tarragona.ppm";	
	//	cols = 10;
	//	rows = 8;
      } else if (std::string(argv[i]) == std::string("-demo2")) {
	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/tarragona.ppm";	
	cols = 5;
	rows = 4;
	//image_filename = "images/EMPAC_concert_hall_small.ppm";
	//image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/EPCOT_MONORAIL.ppm";
	//cols = 8;
	//rows = 6;
      } else if (std::string(argv[i]) == std::string("-demo3")) {
	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/french_abbey.ppm";
	//image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/EMPAC_LOBBY_smaller.ppm";
	cols = 6;
	rows = 4;
      } else if (std::string(argv[i]) == std::string("-demo4")) {
	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/empac_ext_night_hull_smaller.ppm";
	//image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/waikiki.ppm";	
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/EMPAC_concert_hall_small.ppm";
	cols = 6;
	rows = 4;
      } else if (std::string(argv[i]) == std::string("-tiled_display")) {
	tiled_display.read(i,argc,argv);

      } else if (std::string(argv[i]) == std::string("-size")) {
        tiled_display.full_screen = false;        
	i++;
	assert (i < argc);	
	int width = atoi(argv[i]);
	i++;
	assert (i < argc);	
	int height = atoi(argv[i]);
	tiled_display.reshape(width,height);

      } else {
	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	assert (0);
      }
    }

    tiled_display.print();
  }
  

  void DefaultValues() {
    rows = 3;
    cols = 5;
    image_filename = "../source/applications/puzzle/images/waikiki.ppm";

    multimice_enabled = true;
    lasers_enabled = true;
  }


  // ==============
  // Representation
  // all public! (no accessors)

  std::string image_filename;
  int rows;
  int cols;

  bool multimice_enabled;
  bool lasers_enabled;

  TiledDisplay tiled_display;

};


// ========================================================

#endif
