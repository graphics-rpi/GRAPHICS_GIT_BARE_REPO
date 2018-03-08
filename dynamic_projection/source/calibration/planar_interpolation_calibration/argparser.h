#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cassert>
#include "planar_calibration.h"
#include "tiled_display.h"

class ArgParser {

public:
  ArgParser() { DefaultValues(); }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    
    for (int i = 1; i < argc; i++) {
      if (std::string(argv[i]) == std::string("-collect_geometry")) {
	collect_geometry = true;
	assert (collect_intensity == false && test_data == false && all_black == false);
      } else if (std::string(argv[i]) == std::string("-collect_intensity")) {
	collect_intensity = true;
	assert (collect_geometry == false && test_data == false && all_black == false);
      } else if (std::string(argv[i]) == std::string("-clear_all_intensities")) {
	clear_all_intensities = true;
      } else if (std::string(argv[i]) == std::string("-clear_laser_intensity")) {
	i++;
	assert (i < argc);
	int which_laser = atoi(argv[i]);
	assert (which_laser > 0 && which_laser <= MAX_LASERS);
	clear_laser_intensity.push_back(which_laser-1);
      } else if (std::string(argv[i]) == std::string("-visualize_intensity")) {
	visualize_intensity = true;
      } else if (std::string(argv[i]) == std::string("-logfile")) {
	i++;
	assert (i < argc);	
	tracking_logfile = argv[i];
      } else if (std::string(argv[i]) == std::string("-test_data")) {
	test_data = true;
	assert (collect_geometry == false && collect_intensity == false && all_black == false);
      } else if (std::string(argv[i]) == std::string("-all_black")) {
	all_black = true;
	assert (collect_geometry == false && collect_intensity == false && test_data == false);
      } else if (std::string(argv[i]) == std::string("-not_full_screen")) {
	tiled_display.full_screen = false;

      } else if (std::string(argv[i]) == std::string("-size")) {
	i++;
	assert (i < argc);	
	int width = atoi(argv[i]);
	i++;
	assert (i < argc);	
	int height = atoi(argv[i]);
	tiled_display.reshape(width,height);

      } else if (std::string(argv[i]) == std::string("-tiled_display")) {
	tiled_display.read(i,argc,argv);
      } else {
	std::cout << "ERROR: unknown command line argument " << argv[i] << std::endl;

	std::cout << " USAGE: " << argv[0] << " -collect_geometry " << std::endl;
	std::cout << " USAGE: " << argv[0] << " -collect_intensity " << std::endl;
	std::cout << " USAGE: " << argv[0] << " -collect_intensity -clear_all_intensities" << std::endl;
	std::cout << " USAGE: " << argv[0] << " -collect_intensity -clear_laser_intensity <LASER#>" << std::endl;
	std::cout << " USAGE: " << argv[0] << " -visualize_intensity" << std::endl;
	std::cout << " USAGE: " << argv[0] << " -test_data" << std::endl;

	assert (0);
      }
    }

    if (all_black + collect_geometry + collect_intensity + test_data == 0) {
      std::cout << "ERROR!  must specify command arg" << std::endl;
      exit(0);
    }

    assert (all_black + collect_geometry + collect_intensity + test_data == 1);
    if (visualize_intensity) {
      assert (test_data == true);
    }

  }
  

  void DefaultValues() {
    collect_geometry = false;
    collect_intensity = false;
    test_data = false;
    all_black = false;

    //full_screen = true;
    //tiled_display = false;
    //width = 700;
    //height = 500;

    TiledDisplay tiled_display;

    /*
    full_display_width = 700;
    full_display_height = 500;
    my_width = full_display_width;
    my_height = full_display_height;
    my_left = 0;
    my_bottom = 0;
    */

    clear_all_intensities = false;
    visualize_intensity = false;
  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  //int width;
  //int height;

  TiledDisplay tiled_display;
  /*
  int full_display_width;
  int full_display_height;
  int my_width;
  int my_height;
  int my_left;
  int my_bottom;
  */


  bool collect_geometry;
  bool collect_intensity;
  bool test_data;
  bool all_black;


  bool clear_all_intensities;
  std::vector<int> clear_laser_intensity;
  bool visualize_intensity;
  //bool full_screen;

  std::string tracking_logfile;
  
};


// ========================================================

#endif
