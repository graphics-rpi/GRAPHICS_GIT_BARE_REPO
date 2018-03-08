#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "quad.h"
#include "object.h"
#include "argparser.h"

extern ArgParser *args;

// ======================================================================
// PLATFORM
// ======================================================================

class Platform : public Object {
public:
  Point corner_points[4];
  //line edges[4];
  //  bool valid;
  v3d world_points[4];

  Platform(const Quad& q){

    if (!args->find_army_terrain) { throw -1; }

    std::vector<double> lengths;
    for (int i = 0; i < 4; i++) {
      lengths.push_back(q.edge_length(i));
    }

    std::sort(lengths.begin(),lengths.end());
    double ratio = lengths[0] / lengths[3];
    // ratio should be 1
    if (ratio < 0.8 || ratio > 1.2) {
      throw -1;
    }
    for(int i = 0; i < 4; i++){
      corner_points[i] = q.corner_points[i];
      //edges[i] = q.edges[i];
    }
    std::cout << "FOUND PLATFORM" << std::endl;
  }

  void project(CalibratedCamera &camera){
    double height = 0.0508; // 0.0508 meters = 2 inches
    v3d unpadded_points[4];
    for (int i=0; i<4; i++){
      unpadded_points[i] = camera.PointFromPixel(corner_points[i].row,
						 corner_points[i].col,
						 height);
    }

    // add padding
    const double platform_padding = 0.00635; // 0.00635 meters = 0.25 inches
    for (int i=0; i<4; i++){
      v3d v1 = unpadded_points[i] - unpadded_points[(i+1)%4];
      v3d v2 = unpadded_points[i] - unpadded_points[(i+3)%4];
      v1.normalize();
      v2.normalize();
      world_points[i] = unpadded_points[i] + (v1 + v2) * platform_padding;
    }

    //    std::cout << "platform ";
    for (int i=0; i<4; i++){

      //      v3d v1 = world_points[i] - world_points[(i+1)%4];

      //std::cout << "  " << v1.length() * 39.3700787;
      //printf("distance = %f\n", v1.length() * 39.3700787);
    }
    //    std::cout << "\n";


  }

  void write(FILE *fp){
    fprintf(fp, "platform ");
    for (int i=0; i<4; i++){
      fprintf(fp, "%+8.4f %+8.4f ",
	      world_points[i].x(),  
	      world_points[i].z());
    }
    fprintf(fp, "\t+0.0508\n");
  }

  void draw(Image<sRGB> &image){
    //PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));

    for (int i=0; i<4; i++){
      int j = (i+1) % 4;
      if (corner_points[i].row >= 0 &&
          corner_points[i].row < image.getRows() &&
          corner_points[i].col >= 0 &&
          corner_points[i].col < image.getCols() &&
          corner_points[j].row >= 0 &&
          corner_points[j].row < image.getRows() &&
          corner_points[j].col >= 0 &&
          corner_points[j].col < image.getCols()){
	image.setLinePixels(
	// image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col),
				 sRGB(255,255,255)
			    );
      }
    }
  }

private:
  Platform(){}
};

#endif
