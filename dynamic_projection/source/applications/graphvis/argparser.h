#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>

#include "../../common/MersenneTwister.h"


class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-not_full_screen")) {
	full_screen = false;
      } else if (std::string(argv[i]) == std::string("-i") ||
		 std::string(argv[i]) == std::string("-filename")) {
	i++; assert (i < argc);
	filename = argv[i];
      } else if (std::string(argv[i]) == std::string("-start_complete")) {
	start_complete = true;
      } else {
	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	assert (0);
      }
    }
  }
  

  void DefaultValues() {
    width = 200;
    height = 200;
    full_screen = true;
    
    rows = 3;
    cols = 5;
    start_complete = false;

    //animals_filename = "animals.txt";
    filename = "test.txt";
  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  MTRand mtrand;

  int width;
  int height;
  bool full_screen;
  //  std::string image_filename;

  int rows;
  int cols;
  bool start_complete;

  std::string filename;

};



// ========================================================

#endif
