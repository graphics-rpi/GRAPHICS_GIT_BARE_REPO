#ifndef __BASE_ARG_PARSER_H__
#define __BASE_ARG_PARSER_H__

#include <cassert>
#include "../../calibration/planar_interpolation_calibration/tiled_display.h"
#include "MagnifierCursor.h"

// ============================================
// THIS IS THE ONE BASE CLASS OF ALL ARG PARSER
// ============================================

typedef std::map<int, MagnifierCursor> mag_cursor_map_t;
typedef std::map<int, MagnifierCursor>::iterator mag_cursor_itr_t;

class ArgParser {

public:
  //ArgParser() { DefaultValues(); }
  virtual ~ArgParser() {}

  // force this to be an abstract class!
  virtual void dummyfunc() = 0;

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-not_full_screen")) {
	tiled_display.full_screen = false;
      } else if (std::string(argv[i]) == std::string("-no_lasers")) {
	lasers_enabled = false; 
        //tiled_display.full_screen = false;
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
      } else if( std::string(argv[i]) == std::string("-ignore_primary_mouse") ) {
        ignore_primary_mouse = true;
      } else {

        // should be rethought
        // skip unknown args

	//std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;
	//assert (0);
      }
    }

    //tiled_display.print();
  }
  

  void DefaultValues() {
    multimice_enabled = true;
    lasers_enabled = true;
    mag_cursors[201] = MagnifierCursor();

    inner_radius = 100;
    outer_radius = 200;
  }


  // ==============
  // Representation
  // all public! (no accessors)
  int inner_radius; 
  int outer_radius;

  bool multimice_enabled;
  bool lasers_enabled;
  TiledDisplay tiled_display;
  std::map<int, MagnifierCursor> mag_cursors;
  
  std::string m_shader_filepath;

  // Variable to set whether we want to ignore PRIMARY MOUSE or not
  //  This is useful if the system architecture doesn't allow the
  //  the renaming of USB mouse devices
  bool ignore_primary_mouse;

};


// ========================================================

#endif
