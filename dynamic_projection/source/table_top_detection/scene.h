#ifndef _SCENE_H_
#define _SCENE_H_

#include "arrow.h"
#include "column.h"
#include "table.h"
#include "curved_wall.h"
#include "unidentified.h"
#include "colortoken.h"
#include "wall.h"

#include "argparser.h"
extern ArgParser *args;

// ======================================================================
// SCENE
// ======================================================================

class Scene {
public:
  Scene(){
    north = 0.0;
    IMAGE_original = NULL;
    IMAGE_undistorted = NULL;
    IMAGE_white_balance = NULL;
    IMAGE_all_components = NULL;
    IMAGE_selected_components = NULL;
    IMAGE_raw_labels = NULL;
    IMAGE_labels_visualization = NULL;
    IMAGE_colors = NULL;
    IMAGE_enhanced_colors = NULL;
    IMAGE_table_mask = NULL;
    table = NULL;
  }
  
  ~Scene(){
    delete IMAGE_original;
    delete IMAGE_undistorted;
    delete IMAGE_white_balance;
    delete IMAGE_all_components;
    delete IMAGE_selected_components;
    delete IMAGE_raw_labels;
    delete IMAGE_labels_visualization;
    delete IMAGE_colors;
    delete IMAGE_enhanced_colors;
    delete IMAGE_table_mask;
    for (unsigned int i = 0; i < objects.size(); i++) {
      delete objects[i];
    }
    objects.clear();
    delete table;
  }

  void assign_room_colors();


  void write(const char *filename);
  void draw(Image<sRGB> &image);

 public:
  std::vector<Object*> objects;

 public:
  std::vector<Blob> blobs;

  Image<sRGB> *IMAGE_original;
  Image<sRGB> *IMAGE_undistorted;
  Image<sRGB> *IMAGE_white_balance;

  Image<byte> *IMAGE_all_components;
  Image<byte> *IMAGE_selected_components;

  Image<int>  *IMAGE_raw_labels;
  Image<sRGB> *IMAGE_labels_visualization;

  Image<sRGB> *IMAGE_colors;
  Image<sRGB> *IMAGE_enhanced_colors;

  Image<byte> *IMAGE_table_mask;

  double north;
  sRGB floor_color;
  sRGB ceiling_color;
  sRGB default_wall_color;
  CalibratedCamera camera;
  Table *table;

};

#endif
