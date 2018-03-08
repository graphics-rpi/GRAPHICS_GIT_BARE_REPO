#ifndef __TILED_DISPLAY_H__
#define __TILED_DISPLAY_H__
// Included files for OpenGL Rendering
#include "../../applications/paint/gl_includes.h"

#include <string>

#include "planar_calibration.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TiledDisplay {

  TiledDisplay() {
    is_tiled_display = false;
    is_master = true;
    full_screen = true;
    full_display_width = 1000;
    full_display_height = 800;
    my_width = full_display_width;
    my_height = full_display_height;
    my_left = 0;
    my_bottom = 0;
  }
  
  void read(int &i, int &argc, char* argv[]) {
    is_tiled_display = true;
    std::string masterstring;
    i++;	assert (i < argc);      masterstring = std::string(argv[i]);
    if (masterstring == "master") {
      is_master = true;
    } else {
      assert (masterstring == "not_master");
      is_master = false;
    }
    i++;	assert (i < argc);	full_display_width = atoi(argv[i]);
    i++;	assert (i < argc);	full_display_height = atoi(argv[i]);
    i++;	assert (i < argc);	my_width = atoi(argv[i]);
    i++;	assert (i < argc);	my_height = atoi(argv[i]);
    i++;	assert (i < argc);	my_left = atoi(argv[i]);
    i++;	assert (i < argc);	my_bottom = atoi(argv[i]);  
    sanity_check_dimensions();
  }
  void reshape(int w, int h) {
    my_width = w;
    my_height = h;     
    if (!is_tiled_display) {
      full_display_width = w;
      full_display_height = h;         
    }
    sanity_check_dimensions();
  }
  void ORTHO() {
    gluOrtho2D(my_left,
	       my_left+my_width, 
	       my_bottom, 
	       my_bottom+my_height);
  }

  void ORTHO2() {
    gluOrtho2D(my_left,
	       my_left+my_width, 
	       my_bottom+my_height,
	       my_bottom);
  }

  glm::mat4 getOrtho(){
    return glm::ortho( (float)my_left, 
            (float)(my_left + my_width), 
            (float)my_bottom, 
            (float)(my_bottom + my_height), 
            -1.0f, 
            1.0f);
  }

  glm::mat4 getOrtho2(){
    return glm::ortho( (float)my_left, 
            (float)(my_left + my_width), 
            (float)(my_bottom + my_height), 
            (float)my_bottom, 
            -1.0f, 
            1.0f);
  }

  void set_from_calibration_data(PlanarCalibration *pc) {

    if (full_screen) {
      
      full_display_width = pc->getWidth();
      full_display_height = pc->getHeight();
      
      if (!is_tiled_display) {
        my_width = pc->getWidth();
        my_height = pc->getHeight();
      }

    }
    sanity_check_dimensions();
  }


  void sanity_check_dimensions() {
    if (full_display_width == 0) {
      print();
      full_display_width = 500;
      full_display_height = 500;
      my_width = 500;
      my_height = 500;
      my_left = 0;
      my_bottom = 0;
    }
    assert (full_display_width > 0);
    assert (full_display_height > 0);
    assert (my_width > 0);
    assert (my_width <= full_display_width);
    assert (my_height > 0);
    assert (my_height <= full_display_height);
    assert (my_left >= 0 && my_left+my_width <= full_display_width);
    assert (my_bottom >= 0 && my_bottom+my_height <= full_display_height);
  }

  void print() {
    std::cout << "TILED DISPLAY" << std::endl;
    std::cout << "is_tiled_display " << is_tiled_display << std::endl;
    std::cout << "full_screen " << full_screen << std::endl;
    std::cout << "full_display_width " << full_display_width << std::endl;
    std::cout << "full_display_height " << full_display_height << std::endl;
    std::cout << "my_width " << my_width << std::endl;
    std::cout << "my_height " << my_height << std::endl;
    std::cout << "my_left " << my_left << std::endl;
    std::cout << "my_bottom " << my_bottom << std::endl;
  }


  // representation
  bool is_tiled_display;
  bool is_master;
  bool full_screen;
  int full_display_width;
  int full_display_height;
  int my_width;
  int my_height;
  int my_left;
  int my_bottom;
};

#endif
