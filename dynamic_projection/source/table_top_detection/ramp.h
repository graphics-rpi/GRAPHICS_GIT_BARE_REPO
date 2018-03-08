#ifndef _RAMP_H_
#define _RAMP_H_

#include <vector>
#include "line.h"
#include "point.h"
#include "quad.h"
#include "color.h"
#include "object.h"
#include "argparser.h"

extern ArgParser *args;

// ======================================================================
// RAMP
// ======================================================================

class Ramp : public Object {
public:
  Point corner_points[4];
  Line edges[4];
  //  bool valid;
  v3d world_points[4];

  Ramp(Image<byte> &component_image,
       const Quad& q, std::vector<Point> &points){

    if (!args->find_army_terrain) { throw -1; }

    std::vector<double> lengths;
    for (int i = 0; i < 4; i++) {
      lengths.push_back(q.edge_length(i));
    }

    std::sort(lengths.begin(),lengths.end());
    double ratio = lengths[0] / lengths[3];
    // ratio should be 3/5 = 0.6
    if (ratio < 0.4 || ratio > 0.8) {
      throw -1;
    } 
    
    for(int i = 0; i < 4; i++){
      corner_points[i] = q.corner_points[i];
      //edges[i] = q.edges[i];
    }
    
    // first recompute edge lines based on corners
    // (redundant, fix later)
    std::vector<Point> endpts(2);
    for(int i = 0; i < 3; i++){
      endpts[0] = corner_points[i];
      endpts[1] = corner_points[i+1];
      edges[i] = Line(endpts);
    }
    endpts[0] = corner_points[3];
    endpts[1] = corner_points[0];
    edges[3] = Line(endpts);

    // assign cyan points to each line based on minimal error
    std::vector<Point> edge_inliers[4];

    // for each point, find error for each edge and assign to edge with minimal error
    for(unsigned int i = 0; i < points.size(); i++){
      // for now only consider cyan points
      if(component_image(int(points[i].row), int(points[i].col)) != CYAN_IDX){
	continue;
      }
      double min_error = edges[0].point_error(points[i]);
      int assigned_edge = 0;
      for(int j = 1; j < 4; j++){
	double assignment_error = edges[j].point_error(points[i]);
	if(assignment_error < min_error){
	  min_error = assignment_error;
	  assigned_edge = j;
	}
      }
      // assign point to this edge
      edge_inliers[assigned_edge].push_back(points[i]);
    }
    
    // determine if one of the edges can be considered a ground edge (does it have enough cyan inliers?)
    int ground_edge_id = -1;
    int most_inliers = 0;
    for(int i = 0; i < 4; i++){
      if((int)edge_inliers[i].size() > most_inliers){
	ground_edge_id = i;
	most_inliers = edge_inliers[i].size();
      }
    }
    if(ground_edge_id == -1){
      // no sufficient ground edge found, reject
      throw -1; //valid = false;
    }
    else{
      // reorder edges and corner points based on orientation of ground edge
      // first create temp copies of the corners and edges
      
      Point temp_corners[4];
      Line temp_edges[4];
      for(int i = 0; i < 4; i++){
	temp_corners[i] = corner_points[i];
	temp_edges[i] = edges[i];
      }
      for(int i = 0, j = ground_edge_id; i < 4; i++, j = (j+3)%4){
	corner_points[3-j] = temp_corners[i];
	edges[3-j] = temp_edges[i];
      }
      
      // check for approximate aspect ratio 5:3 (add this later)

      //      valid = true;
    }

    (*args->output) << "FOUND RAMP" << std::endl;

  }

  void project(CalibratedCamera &camera){
    // short end height : 0.00635 meters = 0.25 inches
    // tall end height : 0.0508 meters = 2 inches
    // points are oriented in "short, tall, tall, short" order    
    //double heights[4] = {0.0508, 0.0508, 0.0508, 0.0508};
    double heights[4] = {0.00635, 0.0508, 0.0508, 0.00635};
    v3d unpadded_points[4];
    for (int i=0; i<4; i++){
      unpadded_points[i] = camera.PointFromPixel(corner_points[i].row,
						 corner_points[i].col,
						 heights[i]);
      // zero out the y value now that we've accurately computed x and z
      // from the correct height (we only care about its dimensions in the xz plane)
      unpadded_points[i].y() = 0;
    }

    // add padding
    const double ramp_padding = 0.00635; // 0.00635 meters = 0.25 inches
    for (int i=0; i<4; i++){
      v3d v1 = unpadded_points[i] - unpadded_points[(i+1)%4];
      v3d v2 = unpadded_points[i] - unpadded_points[(i+3)%4];
      v1.normalize();
      v2.normalize();
      world_points[i] = unpadded_points[i] + (v1 + v2) * ramp_padding;
    }

    //    (*args->output) << "ramp ";
    for (int i=0; i<4; i++){
      
      // was unused, commented out ??
      //v3d v1 = world_points[i] - world_points[(i+1)%4];
      

      //(*args->output) << "  " << v1.length() * 39.3700787;
      //printf("distance = %f\n", v1.length() * 39.3700787);
    }
    //    (*args->output) << "\n";
  }

  void write(FILE *fp){
    fprintf(fp, "ramp ");
    for (int i=0; i<4; i++){
      fprintf(fp, "%+8.4f %+8.4f ",
	      world_points[i].x(),  
	      world_points[i].z());
    }
    fprintf(fp, "\t+0.0508\n");
  }


  void draw(Image<sRGB> &image){
    //PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));

    for(int i = -5; i < 5; i++){
      image(corner_points[0].row + i, corner_points[0].col) = sRGB(255, 0, 0);
      image(corner_points[0].row, corner_points[0].col + i) = sRGB(255, 0, 0);
    }
    for(int i = -5; i < 5; i++){
      image(corner_points[1].row + i, corner_points[1].col) = sRGB(0, 255, 0);
      image(corner_points[1].row, corner_points[1].col + i) = sRGB(0, 255, 0);
    }
    for(int i = -5; i < 5; i++){
      image(corner_points[2].row + i, corner_points[2].col) = sRGB(0, 0, 255);
      image(corner_points[2].row, corner_points[2].col + i) = sRGB(0, 0, 255);
    }
    for(int i = -5; i < 5; i++){
      image(corner_points[3].row + i, corner_points[3].col) = sRGB(255, 255, 0);
      image(corner_points[3].row, corner_points[3].col + i) = sRGB(255, 255, 0);
    }

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
	//        image.applyFunctorOnLine(setPixel, 
        image.setLinePixels(
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
  Ramp(){}
};

#endif
