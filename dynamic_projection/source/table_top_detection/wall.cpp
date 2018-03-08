#include "wall.h"
#include "window.h"
#include "line.h"

// window keywords by color
//const char *CYAN_KEYWORD = "cyan";
//const char *MAGENTA_KEYWORD = "magenta";
//onst char *YELLOW_KEYWORD = "yellow";

#include "argparser.h"
extern ArgParser *args;


Wall::Wall(std::vector<Point> &edge_points, Line centerline,
           std::vector<Point> &points, Image<byte> &component_image) {
    this->centerline = centerline;
    n_points = points.size();

    //std::cout << "TRY TO MAKE A WALL" << std::endl;

    color = sRGB (255,255,0);

    // fit lines to both sides
    std::vector<Point> side1_points;
    std::vector<Point> side2_points;
    for (unsigned i=0; i<edge_points.size(); i++){
      if (centerline.side(edge_points[i])){
        side1_points.push_back(edge_points[i]);
      } else {
        side2_points.push_back(edge_points[i]);
      }
    }
    double inlier_thresh = 2.0;
    line1 = Line(side1_points, inlier_thresh);
    line2 = Line(side2_points, inlier_thresh);

    // estimate wall thickness
    double side1_thickness = 0.;
    for (unsigned i=0; i<side1_points.size(); i++){
      side1_thickness += fabs(centerline.b * side1_points[i].row + 
			      centerline.a * side1_points[i].col +
			      centerline.c);
    }
    side1_thickness /= double(side1_points.size());

    double side2_thickness = 0.;
    for (unsigned i=0; i<side2_points.size(); i++){
      side2_thickness += fabs(centerline.b * side2_points[i].row + 
			      centerline.a * side2_points[i].col +
			      centerline.c);
    }
    side2_thickness /= double(side2_points.size());

    double thickness = side1_thickness + side2_thickness;

    // check for reasonable wall thickness
    // this is measured in pixels, approximately


    
    if (thickness < 5. || thickness > 19.){
      (*args->output) << "wall too thick = " << thickness << std::endl;
      throw -1;
      //valid = false;
      //return;
    }
    
    // check for reasonable parallelism; reject "wedge"-shaped walls
    double dot = line1.a * line2.a + line1.b * line2.b;
    double parallel_dot_thresh = 0.95;
    if (fabs(dot) < parallel_dot_thresh){
      (*args->output) << "error with parallel" << std::endl;
      throw -1;
      //valid = false;
      //return;
    }
    
    // find extents of points - determine orthogonal cap lines
    double min_d = std::numeric_limits<double>::max();
    double max_d = -std::numeric_limits<double>::max();

    for (unsigned i=0; i<points.size(); i++){
      double d = centerline.a*points[i].row - 
        centerline.b*points[i].col;
      if (d > max_d) max_d = d;
      if (d < min_d) min_d = d;
    }

    // find dominant color at each point along centerline
    int min_idx = int(min_d) - 1;
    int max_idx = int(max_d) + 1;
    int nhist = max_idx - min_idx + 1;
    std::vector<Histogram> hist(nhist, Histogram());
    Histogram global_hist;
    for (unsigned i=0; i<points.size(); i++){
      byte color = component_image(int(points[i].row),
                                   int(points[i].col));
      double d = centerline.a*points[i].row - 
        centerline.b*points[i].col;
      int idx = int(d) - min_idx;
      assert(idx >= 0);
      assert(idx < nhist);
      global_hist[color]++;
      hist[idx][color]++;
    }

    // find dominant "wall" color over entire length
    int wall_idx = RED_IDX;
    int max_count = global_hist[RED_IDX];
    if (global_hist[GREEN_IDX] > max_count){
      wall_idx = GREEN_IDX;
      max_count = global_hist[GREEN_IDX];
    }
    if (global_hist[BLUE_IDX] > max_count){
      wall_idx = BLUE_IDX;
      max_count = global_hist[BLUE_IDX];
    }

    double wall_height = wall_idx_to_height(wall_idx);
    y = wall_height;

    // classify each bin along length by dominant color
    std::vector<int> dominant_color(nhist);
    for (int i=0; i<nhist; i++){
      int best_idx = 0;
      int max_count = hist[i][0];
      for (int j=1; j<NUM_COLORS; j++){
        if (hist[i][j] > max_count){
          max_count = hist[i][j];
          best_idx = j;
        }
      }
      dominant_color[i] = best_idx;
    }

    // filter dominant colors to remove noisy "short runs"
    // !!! todo - use running historgram to make this faster
    std::vector<int> filtered_dominant_color(nhist);
    filtered_dominant_color =dominant_color;
    int filter_width = 13; // must be odd
    int half_width = (filter_width-1)/2;
    for (int i=half_width; i<nhist-half_width; i++){
      Histogram filter_hist;
      for (int j=-half_width; j<=half_width; j++){
	filter_hist[dominant_color[i+j]]++;
      }
      
      // find dominant color over filter length
      // either the wall color, or a window color
      int filtered_idx = wall_idx;
      int max_count = filter_hist[filtered_idx];
      if (filter_hist[CYAN_IDX] > max_count){
	filtered_idx = CYAN_IDX;
	max_count = filter_hist[CYAN_IDX];
      }
      if (filter_hist[MAGENTA_IDX] > max_count){
	filtered_idx = MAGENTA_IDX;
	max_count = filter_hist[MAGENTA_IDX];
      }
      if (filter_hist[YELLOW_IDX] > max_count){
	filtered_idx = YELLOW_IDX;
      }
      filtered_dominant_color[i] = filtered_idx;
    }
    
    dominant_color = filtered_dominant_color;

    // cut into sections by dominant color (detect windows)
    // ignore color runs less than this number of pixels long
    // DOLCEA: took all this out
    const int MIN_COLOR_RUN_LENGTH = 5;
    int current_color = dominant_color[0];
    int run_start = 0;
    for (int i=1; i<nhist; i++){
      if (dominant_color[i] != current_color){
        int run_length = i - run_start;
        bool valid_color = false;
        if (run_length > MIN_COLOR_RUN_LENGTH){
          std::string window_keyword;
          switch (current_color){
          case CYAN_IDX:
            window_keyword = COLOR_KEYWORD[CYAN_IDX];
            valid_color = true;
            break;
          case MAGENTA_IDX:
            window_keyword = COLOR_KEYWORD[MAGENTA_IDX];
            valid_color = true;
            break;
          case YELLOW_IDX:
            window_keyword = COLOR_KEYWORD[YELLOW_IDX];
            valid_color = true;
            break;
          default:
            break;
          }
          if (valid_color){
            // found a window
            
            // find cap lines
            Line cap1(centerline.b, -centerline.a, double(run_start+min_idx));
            Line cap2(centerline.b, -centerline.a, double(i-1+min_idx));
            Rectangle rect(line1, cap1, line2, cap2, centerline);
            Window window(rect, window_keyword);
            windows.push_back(window);
          }
        }
        current_color = dominant_color[i];
        run_start = i;
      }
    }


    // create cap lines
    cap1 = Line(centerline.b, -centerline.a, min_d);
    cap2 = Line(centerline.b, -centerline.a, max_d);

    // find endpoints on center line
    Linepair pair1(cap1, centerline);
    end_point1 = pair1.intersection;
    Linepair pair2(cap2, centerline);
    end_point2 = pair2.intersection;

    rectangle = Rectangle(line1, cap1, line2, cap2, centerline);





    /*

    // check if this should be merged with any of the previous walls
    for (unsigned i=0; i<previous_walls.size(); i++){
      Wall w = previous_walls[i];
      if (color_idx != w.color_idx) continue; // must be same color
      double dot = fabs(w.centerline.a * centerline.a +
			w.centerline.b * centerline.b);

      // how "parallel" walls must be to merge
      const double dot_thresh = 0.5;
      if (dot > dot_thresh){
	double min_dist = end_point1.distance(w.end_point1);
	Point new_end1 = end_point2;
	Point new_end2 = w.end_point2;
	Line new_cap1 = cap2;
	Line new_cap2 = w.cap2;
	double dist = end_point1.distance(w.end_point2);
	if (dist < min_dist){
	  min_dist = dist;
	  new_end1 = end_point2;
	  new_end2 = w.end_point1;
	  new_cap1 = cap2;
	  new_cap2 = w.cap1;
	}
	dist = end_point2.distance(w.end_point1);
	if (dist < min_dist){
	  min_dist = dist;
	  new_end1 = end_point1;
	  new_end2 = w.end_point2;
	  new_cap1 = cap1;
	  new_cap2 = w.cap2;
	}
	dist = end_point2.distance(w.end_point2);
	if (dist < min_dist){
	  min_dist = dist;
	  new_end1 = end_point1;
	  new_end2 = w.end_point1;
	  new_cap1 = cap1;
	  new_cap2 = w.cap1;
	}

	// how close wall endpoints must be to merge
	const double dist_thresh = 10.0;
	if (min_dist < dist_thresh){
	  // merge this wall into old one
	  printf ("merging walls\n");
	  throw -1;
	  //valid = false;
	  double old_weight = double(w.n_points)/double(n_points+w.n_points);
	  double weight = 1. - old_weight;
	  // merge centerline
	  previous_walls[i].centerline.a = 
	    ( old_weight * previous_walls[i].centerline.a +
	      weight * centerline.a);
	  previous_walls[i].centerline.b = 
	    ( old_weight * previous_walls[i].centerline.b +
	      weight * centerline.b);
	  previous_walls[i].centerline.c = 
	    ( old_weight * previous_walls[i].centerline.c +
	      weight * centerline.c);
	  // merge line1
	  previous_walls[i].line1.a = 
	    ( old_weight * previous_walls[i].line1.a +
	      weight * line1.a);
	  previous_walls[i].line1.b = 
	    ( old_weight * previous_walls[i].line1.b +
	      weight * line1.b);
	  previous_walls[i].line1.c = 
	    ( old_weight * previous_walls[i].line1.c +
	      weight * line1.c);
	  // merge line2
	  previous_walls[i].line2.a = 
	    ( old_weight * previous_walls[i].line2.a +
	      weight * line2.a);
	  previous_walls[i].line2.b = 
	    ( old_weight * previous_walls[i].line2.b +
	      weight * line2.b);
	  previous_walls[i].line2.c = 
	    ( old_weight * previous_walls[i].line2.c +
	      weight * line2.c);

	  // merge endpoints
	  previous_walls[i].end_point1 = new_end1;	 
 	  previous_walls[i].end_point2 = new_end2;	  
	  // merge caps
	  previous_walls[i].cap1 = new_cap1;	 
 	  previous_walls[i].cap2 = new_cap2;	  
	  // merge n_points
	  previous_walls[i].n_points += n_points;
	  // caculate new rectangle
	  previous_walls[i].rectangle = 
	    Rectangle(previous_walls[i].line1,
		      previous_walls[i].cap1,
		      previous_walls[i].line2,
		      previous_walls[i].cap2,
		      previous_walls[i].centerline);

	  // merge windows (DOLCEA: took this out)
	  previous_walls[i].windows.insert(previous_walls[i].windows.end(),
	  				   windows.begin(), windows.end());
	  // note that this fails if middle section of wall is processed last
	  return;
	}
	}
	}

        */
    //std::cout << "FOUND WALL" << std::endl;
}
