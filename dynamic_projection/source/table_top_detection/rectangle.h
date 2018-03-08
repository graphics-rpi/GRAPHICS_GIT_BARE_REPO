#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include <cmath>
#include "line.h"
#include "../common/Image.h"
#include "../common/CalibratedCamera.h"


// ======================================================================
// RECTANGLE
// ======================================================================

class Rectangle {
public:
  Rectangle(){}

  Rectangle(Line line1, Line cap1, Line line2, Line cap2, Line centerline){
    this->line1 = line1;
    this->line2 = line2;
    this->centerline = centerline;
    this->cap1 = cap1;
    this->cap2 = cap2;

    for (int i=0; i<4; i++){
      point_order[i] = i;
    }

    // create intersection points
    std::vector<Linepair> pairs;
    pairs.push_back(Linepair(line1, cap1));
    pairs.push_back(Linepair(line1, cap2));
    pairs.push_back(Linepair(line2, cap1));
    pairs.push_back(Linepair(line2, cap2));

    // calculate center of 4 corners
    double xc = 0.;
    double yc = 0.;
    std::vector<Point> corners;
    for (unsigned i=0; i<4; i++){
      corners.push_back(pairs[i].intersection);
      xc += pairs[i].intersection.col;
      yc += pairs[i].intersection.row;
    }
    xc /= 4.;
    yc /= 4.;

    // sort points into angular order around the center
    // N^2 sort (but 4^2 = 16)
    for (int i=0; i<3; i++){
      double ang1 = atan2(corners[i].row - yc, corners[i].col - xc);
      if (ang1 < 0.){
	ang1 += 2*M_PI;
      }    
      int closest = 0;
      double min_dist = 2*M_PI;
      for (int j=i+1; j<4; j++){
	double ang2 = atan2(corners[j].row - yc, corners[j].col - xc);
	if (ang2 < 0.){
	  ang2 += 2*M_PI;
	}
	// find clockwise angular distance between points
	double dist = ang2 - ang1;
	if (dist < 0.){
	  dist += 2*M_PI;
	}
	if (dist < min_dist){
	  min_dist = dist;
	  closest = j;
	}
      }
      Point temp = corners[i+1];
      corners[i+1] = corners[closest];
      corners[closest] = temp;
      int temp2 = point_order[i+1];
      point_order[i+1] = point_order[closest];
      point_order[closest] = temp2;
    }
    
    for (int i=0; i<4; i++){
      corner_points[i] = corners[i];
    }
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
	image.setLinePixels( int(corner_points[i].row),
		       int(corner_points[i].col),
		       int(corner_points[j].row),
		       int(corner_points[j].col),
		       sRGB(255,255,255) );

	/*        image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col));
	*/
      }
    }
  }


  void project(CalibratedCamera &camera, double height){
    for (int i=0; i<4; i++){
      world_points[i] = camera.PointFromPixel(corner_points[i].row,
                                              corner_points[i].col,
                                              height);
    }    
  }


  void project_and_extend(CalibratedCamera &camera, double height,
			  double extension){
   
    // find intersection of caps with centerline
    Linepair lp1(line1, cap1);
    Linepair lp2(line1, cap2);

    v3d wp[4];

    // project into 3D
    wp[0] = camera.PointFromPixel(lp1.intersection.row,
                                  lp1.intersection.col,
                                  height);
    wp[1] = camera.PointFromPixel(lp2.intersection.row,
                                  lp2.intersection.col,
                                  height);
    // extend points
    v3d dir = (wp[1] - wp[0]);
    dir.normalize();
    wp[0] = wp[0] - extension * dir;
    wp[1] = wp[1] + extension * dir;

    Linepair lp3(line2, cap1);
    Linepair lp4(line2, cap2);

    // project into 3D
    wp[2] = camera.PointFromPixel(lp3.intersection.row,
                                  lp3.intersection.col,
                                  height);
    wp[3] = camera.PointFromPixel(lp4.intersection.row,
                                  lp4.intersection.col,
                                  height);
    // extend points
    dir = (wp[3] - wp[2]);
    dir.normalize();
    wp[2] = wp[2] - extension * dir;
    wp[3] = wp[3] + extension * dir;

    for (int i=0; i<4; i++){
      world_points[i] = wp[point_order[i]];
    }
  }

  void write(FILE *fp){
    for (int i=0; i<4; i++){
      fprintf(fp, "%+8.4f %+8.4f ",
              world_points[i].x(),  
              world_points[i].z());
    }
  }

  //private:
  Line centerline;
  Line line1;
  Line line2;
  Line cap1;
  Line cap2;
  v3d world_points[4];
  Point corner_points[4];
  int point_order[4];
};

#endif
