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
      } else if (std::string(argv[i]) == std::string("-tiled_display")) {
	tiled_display.read(i,argc,argv);
      } else {
	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	assert (0);
      }
    }
  }
  

  void DefaultValues() {
  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  TiledDisplay tiled_display;
};


// ========================================================

#endif
