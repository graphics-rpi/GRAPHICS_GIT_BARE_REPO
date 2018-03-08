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

	    tiled_display.full_display_width = 1200;
	    tiled_display.full_display_height = 1200;
	    tiled_display.my_width = 1200;
	    tiled_display.my_height = 1200;

      } else if (std::string(argv[i]) == std::string("-i") ||
		std::string(argv[i]) == std::string("-animals_filename")) {
	    i++; assert (i < argc);
	    animals_filename = argv[i];
      } else if (std::string(argv[i]) == std::string("-no_lasers")) {
	    lasers = false;
      } else if (std::string(argv[i]) == std::string("-move_mice")) {
	randomly_move_multimice = true;
      } else if (std::string(argv[i]) == std::string("-crop_zoom")) {
	crop_zoom = true;
      } else if (std::string(argv[i]) == std::string("-white_background")) {
	background_color = Vec3f(1,1,1);
      } else if (std::string(argv[i]) == std::string("-draw_grid")) {
	draw_grid = true;
      } else if (std::string(argv[i]) == std::string("-images")) {
	animal_example = false;
        while (i+1 < argc && std::string(argv[i+1]).size() > 0 && argv[i+1][0] != '-') {
          i++; assert (i < argc);
          image_collection_classes.push_back(argv[i]);
        }
        assert (image_collection_classes.size() >= 1);
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

    animal_example = true;
    randomly_move_multimice = false;
    crop_zoom = false;

    draw_grid = false;

    pause = false;
    draw_trails_seconds = 2.0;

    background_color = Vec3f(0,0,0);

    // The relative location of the more_animals.txt file
    animals_filename = "../source/applications/graph_interaction/more_animals.txt";

    // the mir image collection
    image_collection_directory = "../mir_images/mirflickr";


  }


  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  MTRand mtrand;
  TiledDisplay tiled_display;
  bool start_complete;
  bool lasers;

  bool animal_example;

  bool pause;

  double draw_trails_seconds;

  bool randomly_move_multimice;
  bool crop_zoom;
  bool draw_grid;
  Vec3f background_color;
  std::string animals_filename;
  std::vector<std::string> image_collection_classes;
  std::string image_collection_directory;

};



// ========================================================

#endif
