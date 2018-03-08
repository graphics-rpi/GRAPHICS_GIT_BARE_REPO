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
      } else 
      if (std::string(argv[i]) == std::string("-ip")) {
      	osc_ip_address = argv[i+1];
      	i++;
      } else if (std::string(argv[i]) == std::string("-port")) {
      	osc_port = atoi(argv[i+1]);
      	i++;
      } else {
	    	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	    	assert (0);
      }
    }
  }
  

  void DefaultValues() {
    osc_ip_address = (char*)"127.0.0.1";
    osc_port = 7000;
    include_lasers = false;
  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  TiledDisplay tiled_display;
  bool include_lasers;
  char* osc_ip_address;
  int osc_port;
};


// ========================================================

#endif
