#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>
#include <fstream>
#include "../common/MersenneTwister.h"


class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ~ArgParser() {
    if (output != &std::cout)
      delete output;
  }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
  
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == "-noblending") {
	blending = false;
      } else if (std::string(argv[i]) == "-army") {
	army = true;
      } else if (std::string(argv[i]) == "-multidisplay") {
	multidisplay = true;
      } else if (std::string(argv[i]) == "-verbose") {
	verbose = true; 
      } else if (std::string(argv[i]) == "-no_pic") {
	take_pic = false; 
      } else if (std::string(argv[i]) == "-quiet") {
	verbose = false;
      } else if (std::string(argv[i]) == "-use_stored_textures") {
	use_stored_textures = true;
      } else {
	std::cerr << "UNKNOWN COMMAND LINE ARGUMENT: " << argv[i] << std::endl;
	usage(argc,argv); 
	exit(0);
      }
    }
    
    if (verbose) {
      output = &std::cout;
    } else {
      output = new std::ofstream("/dev/null");
    }
  }

  void DefaultValues() {
    blending = true;
    army = false;
    multidisplay = false;
    verbose = true;
    take_pic = true;
    use_stored_textures = false;
  }

  void usage (int argc, char ** argv) {
    exit(0);
  }

  // ==============
  // REPRESENTATION
  // all public! (no accessors)


  bool blending;
  bool army;
  bool multidisplay;
  bool take_pic;
  bool verbose;
  bool use_stored_textures;
  std::ostream *output;

};



// ========================================================

#endif
