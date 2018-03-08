#ifndef __PAINT_ARG_PARSER_H__
#define __PAINT_ARG_PARSER_H__

#include "base_argparser.h"

class PaintArgParser : public ArgParser {

public:
  //  PaintArgParser() : ArgParser() { DefaultValues(); }

  virtual void dummyfunc() {}

  PaintArgParser(int argc, char *argv[]) : ArgParser(argc, argv) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-not_full_screen")) {
	tiled_display.full_screen = false;
      } else if (std::string(argv[i]) == std::string("-human_paintbrush")) {
	human_paintbrush = true;
	//full_screen = false;
	//width = 900;
	//height = 900;
      } else if (std::string(argv[i]) == std::string("-tiled_display")) {
	tiled_display.read(i,argc,argv);
      } else {
	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	assert (0);
      }
    }
  }
  

  void DefaultValues() {
    //width = 200;
    //height = 200;
    //full_screen = true;
    human_paintbrush = false;
  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  TiledDisplay tiled_display;

  //int width;
  //int height;
  //bool full_screen;
  bool human_paintbrush;
};


// ========================================================

#endif
