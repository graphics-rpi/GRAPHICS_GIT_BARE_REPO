#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>
#include "../../common/MersenneTwister.h"

#include "../../calibration/planar_interpolation_calibration/tiled_display.h"

class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-not_full_screen")) {
	tiled_display.full_screen = false;
      } else if (std::string(argv[i]) == std::string("-i") ||
		 std::string(argv[i]) == std::string("-animals_filename")) {
	i++; assert (i < argc);
	animals_filename = argv[i];
      } else if (std::string(argv[i]) == std::string("-no_lasers")) {
	lasers = false;
      } else if (std::string(argv[i]) == std::string("-start_complete")) {
	start_complete = true;
      } else if (std::string(argv[i]) == std::string("-tiled_display")) {
	tiled_display.read(i,argc,argv);
      } else {
	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	assert (0);
      }
    }
  }
  

  void DefaultValues() {
    start_complete = false;
    lasers = true;

    //animals_filename = "animals.txt";
    // The relative location of the more_animals.txt file
    animals_filename = "../source/applications/multi_surface_graph/more_animals.txt";
  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  MTRand mtrand;
  TiledDisplay tiled_display;
  bool start_complete;
  bool lasers;

  std::string animals_filename;

};



// ========================================================

#endif
