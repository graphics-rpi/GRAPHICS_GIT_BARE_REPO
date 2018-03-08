#ifndef _ARROW_H_
#define _ARROW_H_

#include "color.h"
#include "point.h"
#include "line.h"
#include "object.h"
#include "argparser.h"

extern ArgParser *args;

// ======================================================================
// NORTH ARROW
// ======================================================================

struct ArrowPoint {
  ArrowPoint (double d, double dist){
    this->d = d;
    this->dist = dist;
  }
  double d, dist;
};


inline bool ArrowPointBydist(const ArrowPoint &a, const ArrowPoint &b){
  return a.dist > b.dist;
}


class Arrow : public Object {
 public:
  Arrow(Line centerline, std::vector<Point> &edge_points, Histogram &histogram) {

    //std::cout << "trying to make an arrow" << std::endl;
    confidence = 0.9;

    this->centerline = centerline;
    
    int total = histogram.total();
    std::vector<std::pair<int,double> > percents = histogram.calculate_percents();

    if (!args->find_architectural_design) { throw -1; }

    if (percents[ORANGE_IDX].second + percents[RED_IDX].second < 0.8) { 
      //(*args->output) << "not orange enough" << std::endl;
      throw -1; 
    }

    if (percents[ORANGE_IDX].second < 0.3 * percents[RED_IDX].second) { 
      //(*args->output) << "not orange enough" << std::endl;
      throw -1; 
    }

#if 0
    (*args->output) << "-----------------------" << std::endl;
    (*args->output) << "NORTH ARROW CANDIDATE" << std::endl;
    (*args->output) << "get total " << total << std::endl; 
    (*args->output) << "orange reds " << percents[ORANGE_IDX].second << " " <<
      percents[RED_IDX].second << " " << std::endl;
    (*args->output) << "-----------------------" << std::endl;
#endif
    
    // smaller camera
    //    const int total_north_target = 1425;

    // higher res camera
    const int total_north_target = 3413;

    if (total < 0.9 * total_north_target || 
		total > 1.1 * total_north_target) {

      //std::cerr << total << " " << 0.9 * total_north_target<< " " << 1.1 * total_north_target << std::endl;

      //(*args->output) << "north arrow wrong size total" << std::endl;
      throw -1; 
    }

    // has to have enough edge points
    if (edge_points.size() < 20){
      //(*args->output) << "north arrow edge points wrong" << std::endl;
      throw -1;
    }

    // collect the distance along the line and distance from line
    double min_d = std::numeric_limits<double>::max();
    double max_d = -std::numeric_limits<double>::max();
    std::vector<ArrowPoint>arrow_points;
    for (unsigned i=0; i<edge_points.size(); i++){
      double d = centerline.a*edge_points[i].row - 
        centerline.b*edge_points[i].col;
      double dist = fabs(centerline.a*edge_points[i].col +
                         centerline.b*edge_points[i].row + centerline.c);
      arrow_points.push_back(ArrowPoint(d,dist));
      if (d < min_d) min_d = d;
      if (d > max_d) max_d = d;
    }
    std::sort(arrow_points.begin(), arrow_points.end(), ArrowPointBydist);

    // let the top N widest points vote on which side is the head
    unsigned N = 11; //note must be smaller than the 20 above
    unsigned votes = 0;
    double mid_d = (min_d + max_d)/2.;
    for (unsigned i=0; i<N; i++){
      if (arrow_points[i].d > mid_d) votes++;
    }
    
    // find the mean with of the M-N narrowest points
    double mean_dist = 0.;
    for (unsigned i=N; i<arrow_points.size(); i++){
      mean_dist += arrow_points[i].dist;
    }
    mean_dist /= (arrow_points.size()-N);

    // check widest points; must be this wide to be an arrow
    double wide_thresh = mean_dist * 1.8;
    for (unsigned i=0; i<N; i++){
      if (arrow_points[i].dist < wide_thresh){
        //(*args->output) << "north arrow check widest points" << std::endl;
	throw -1;
	//valid = false;
	//return;
      }
    }    
    
    // find points for debug drawing
    Linepair pair1(centerline, Line(centerline.b, -centerline.a, min_d));
    Linepair pair2(centerline, Line(centerline.b, -centerline.a, mid_d));
    Linepair pair3(centerline, Line(centerline.b, -centerline.a, max_d));
    // compare center of mass to geometrical center

    if (votes > N/2){
      p1 = pair2.intersection;
      p2 = pair3.intersection;
    } else {
      p1 = pair2.intersection;
      p2 = pair1.intersection;
    }
    north = atan2(p1.row - p2.row,
		  p1.col - p2.col);

    //static int num_north_arrows = 0;
    //num_north_arrows++;
    //if (num_north_arrows > 1) { std::cerr << "WHOOPS! found more than one north arrow!" << std::endl; }    

    //    printf("original_north = %f\n", north);
  }

  void project(CalibratedCamera &camera) { }
  void write(FILE *fp) {}

  void draw(Image<sRGB> &image){
    sRGB tmp;
    tmp = COLOR_sRGB[ORANGE_IDX];
    // PixelSetter<sRGB> setPixel(tmp); //sRGB(255, 255, 255));
    //if (valid){
    if (p1.row >=0 && p1.row < image.getRows() &&
	p2.row >=0 && p2.row < image.getRows() &&
	p1.col >=0 && p1.col < image.getCols() &&
	p2.col >=0 && p2.col < image.getCols()){
      image.setLinePixels(
			  //image.applyFunctorOnLine(setPixel, 
			  int(p1.row),
			  int(p1.col),
			  int(p2.row),
			  int(p2.col),
			  tmp);
      }
      //}
  }


  Point p1, p2;
  Line centerline;
  double north;
  //  bool valid;
};

#endif 
