#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include "../paint/base_argparser.h"

class PuzzleArgParser : public ArgParser {

public:
  //ArgParser() { DefaultValues(); }

  virtual void dummyfunc() {}
  
  PuzzleArgParser(int argc, char *argv[]) : ArgParser(argc,argv) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-image_filename") ||
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

      } else if (std::string(argv[i]) == std::string("-petascale")) {
        opening_image_filename = "../source/applications/puzzle/images/PetascaleTITLE.jpg";
	image_filename = "../source/applications/puzzle/images/version6.jpg";
	cols = 5;
	rows = 3;

#if 1
	opening_screen_mode = true;
	puzzle_mode = false;
	solved_puzzle = false;
#endif
	
      } else if (std::string(argv[i]) == std::string("-version1")) {
	image_filename = "../source/applications/puzzle/images/version1.jpg";
	cols = 6;
	rows = 5;
      } else if (std::string(argv[i]) == std::string("-version2")) {
	image_filename = "../source/applications/puzzle/images/version2.jpg";
	cols = 6;
	rows = 5;
      } else if (std::string(argv[i]) == std::string("-version3")) {
	image_filename = "../source/applications/puzzle/images/version3.jpg";
	cols = 6;
	rows = 5;
      } else if (std::string(argv[i]) == std::string("-version4")) {
	image_filename = "../source/applications/puzzle/images/version4.png";
	cols = 6;
	rows = 5;
      } else if (std::string(argv[i]) == std::string("-version5")) {
	opening_image_filename = "../source/applications/puzzle/images/ccni.jpg";
	image_filename = "../source/applications/puzzle/images/version5.png";
	cols = 6;
	rows = 5;

      } else if (std::string(argv[i]) == std::string("-ccni")) {
	image_filename = "../source/applications/puzzle/images/ccni.png";
	cols = 6;
	rows = 5;


      } else if (std::string(argv[i]) == std::string("-demo1")) {
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/WEF_2012.ppm";
	image_filename = "../source/applications/puzzle/images/rpi_ariel_vcc.jpg";
	cols = 4;
	rows = 3;
      } else if (std::string(argv[i]) == std::string("-demo1b")) {
	//image_filename = "images/EMPAC_LOBBY_smaller.ppm";
	image_filename = "../source/applications/puzzle/images/empac_ext_night_hull_smaller.jpg";//ppm";
	cols = 4;
	rows = 3;
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/tarragona.ppm";	
	//	cols = 10;
	//	rows = 8;
      } else if (std::string(argv[i]) == std::string("-demo2")) {
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/tarragona.ppm";	
	image_filename = "../source/applications/puzzle/images/brooklyn_bridge_cables.ppm";
	cols = 5;
	rows = 4;
	//image_filename = "images/EMPAC_concert_hall_small.ppm";
	//image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/EPCOT_MONORAIL.ppm";
	//cols = 8;
	//rows = 6;
      } else if (std::string(argv[i]) == std::string("-demo3")) {
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/french_abbey.ppm";
	image_filename = "../source/applications/puzzle/images/Parc_Jacques-Cartier_cabin.ppm";
	//image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/EMPAC_LOBBY_smaller.ppm";
	cols = 6;
	rows = 4;
      } else if (std::string(argv[i]) == std::string("-demo4")) {
	image_filename = "../source/applications/puzzle/images/BrooklynBridge_sunset.ppm";
	//image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/waikiki.ppm";	
	//	image_filename = "/home/grfx/GRAPHICS_GIT_WORKING_CHECKOUT/dynamic_projection/source/applications/puzzle/images/EMPAC_concert_hall_small.ppm";
	cols = 8;
	rows = 5;
      } else {

        // should be rethought
        // skip unknown args

	//std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	//assert (0);
      }
    }

    tiled_display.print();
  }
  

  void DefaultValues() {
    opening_image_filename = "../source/applications/puzzle/images/rpi_ariel_vcc.jpg";
    rows = 3;
    cols = 5;
    image_filename = "../source/applications/puzzle/images/empac_ext_night_hull_smaller.jpg";
    opening_screen_mode = false;
    puzzle_mode = true;
    solved_puzzle = false;
    m_shader_filepath = "../source/applications/paint/shaders/";
  }


  // ==============
  // Representation
  // all public! (no accessors)

  std::string opening_image_filename;
  std::string image_filename;
  int rows;
  int cols;
  bool opening_screen_mode;
  bool puzzle_mode;
  bool solved_puzzle;
};


// ========================================================

#endif
