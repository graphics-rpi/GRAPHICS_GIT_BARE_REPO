#include <iostream>
#include <limits>
#include <vector>
#include <algorithm>
#include <map>
#include <iomanip>

#include "../common/Image.h"
#include "../common/ImageOps.h"
#include "../common/Vector3.h"
#include "../common/Matrix3.h"
#include "../common/CalibratedCamera.h"
#include "../common/directory_locking.h"

#include "argparser.h"
#include "point.h"
#include "color.h"
#include "table.h"
#include "histogram.h"
#include "scene.h"
#include "object.h"

#define IMAGE_FORMAT ".png"
//#define IMAGE_FORMAT ".jpg"
//#define IMAGE_FORMAT ".ppm"

// ======================================================================
// GLOBAL CONSTANTS

#define CAMERA_CALIBRATION_DAT "../state/table_top_calibration/camera_calibration.dat"
#define TABLE_CAL_DAT "../state/table_top_calibration/table_cal.dat"
//#define TABLE_CAL_DAT "../state/table_top_calibration/table_cal3.dat"
#define CAMERA_IMAGE_DIRECTORY "../state/table_top_image/"
#define WALL_FILE_DIRECTORY "../state/table_top_wallfile/"

const std::string COLOR_KEYWORD[NUM_COLORS] = { "white/black", "red", "orange", "brown", "yellow", "lime", "green", "cyan", "blue", "magenta" };

const sRGB        COLOR_sRGB[NUM_COLORS]    = { sRGB(  0,   0,   0),
						sRGB(255,   0,   0),
						sRGB(255, 128,   0),
						sRGB(128, 128,   0),
						sRGB(255, 255,   0),
						sRGB( 50, 255,  50),
						sRGB(  0, 100,   0),
						sRGB(  0, 255, 255),
						sRGB(  0,   0, 255),
						sRGB(255,   0, 255) };


// wall heights by color
const double RED_HEIGHT   = 0.254;   // 10"
const double BLUE_HEIGHT  = 0.2032;  // 8"
const double GREEN_HEIGHT = 0.127;   // 5"

// wall tip extensions
const double wall_tips        = 0.01143;// 0.45"
//const double wall_tips        = 0.02; // hack: ensure closed cornell box
const double curved_wall_tips = 0.0127; // 0.5"

// color token markers
const int SPECIFIC_WALL_TOKEN_IDX = RED_IDX;
const int GLOBAL_WALL_TOKEN_IDX   = BLUE_IDX;
const int FLOOR_TOKEN_IDX         = GREEN_IDX;
const int FLOOR_TOKEN_IDX2        = CYAN_IDX;
const int CEILING_TOKEN_IDX       = YELLOW_IDX;

#define square(x)((x)*(x))

// ======================================================================
// GLOBAL VARIABLES

ArgParser *args = NULL;

// ======================================================================

template <class T>
void Copy(const Image<int> &bounds, const Image<T> &a, Image<T> &b) {
//  std::cout << "image size" << a.getRows() << " " << a.getCols() << std::endl;
  assert (a.getRows() == b.getRows());
  assert (a.getCols() == b.getCols());
  if (bounds.getRows() != a.getRows()) {
    std::cout << "ERROR bounds: " << bounds.getRows() << " images " << a.getRows() << "x" << a.getCols() << std::endl;
    exit(1);
  }
  assert (bounds.getCols()==2);
  for (int row=0; row<a.getRows(); row++){
    //std::cout << "compare " << bounds(row,0) << " " << a.getCols() << std::endl;
    assert (bounds(row,0) >= 0 && bounds(row,0) <= a.getCols());
    //std::cout << "compare " << bounds(row,1) << " " << a.getCols() << std::endl;
    assert (bounds(row,1) >= 0 && bounds(row,1) <= a.getCols());
    for (int col=bounds(row,0); col<=bounds(row,1); col++){
      b(row,col) = a(row,col);
    }
  }
}


template <class T>
void Swap(Image<T> *&a, Image<T> *&b) {
  Image<T> *c = a;
  a = b;
  b = c;
}


void Enhance(v3d &v, sRGB &norm_color, int &id, sRGB &enh_color) {
  //  v3d orig = v;

  // determine a normalized channel (rgb) difference for this color
  double gap1 = v.max()-v.mid();
  double gap2 = v.mid()-v.min();
  double d = max (gap1,gap2); 
  d *= (1-v.min())/ (v.min()*v.min());
  
  // determine the saturated color (scale between min & max channel intensity)
  v3d u = (v - v3d(v.min())) / v3d(v.max());
  
  double d_thresh = 0.5;
  
  // if we are above a difference threshold...
  v = u * double(d > d_thresh);
  v.normalize();
  norm_color = sRGB(255.*u);
  
  id = 0;

  int r = norm_color.r(); 
  int g = norm_color.g(); 
  int b = norm_color.b(); 

  //  else if (r < 10 && g > 40 && b < 1.2*g) {
  if (r < 0.6*g &&  g > 40 && b < 1.2*g) {
    if (r > b) { 
      id = LIME_IDX; 
    } else {
      id = GREEN_IDX; 
    }
  }

  else if (r > 50 && b < 0.1*r) {    
    if (r < 100 || g < 0.2*r) {
      if (g < 0.5*r) {
	id = RED_IDX;
      } else if (g < 0.9*r) {
	id = BROWN_IDX;
      } else {
	id = YELLOW_IDX; 
      }
    } else if (g < 0.5*r) {
      id = ORANGE_IDX; 
    } else {
      id = YELLOW_IDX; 
    }
  }

  //else if (r < 10 && g < 45 && b > 110) {
  //  else if (r < 10 && g < 45 && b > 100) {
  else if (r < 10 && g < 45 && b > 80) {
    id = BLUE_IDX; 
  }
  

  // cyan
  else if (r < 20 && g > 30 && b > 70) {
    id = CYAN_IDX; 
  }

  else {
    id = BLACK_IDX;
  }

  
  enh_color = COLOR_sRGB[id]; //sRGB(red, green, blue);

  assert (id >=0 && id < NUM_COLORS);
}


Image<byte>* find_colors(const Image<sRGB> &in, 
                         Image<sRGB>*& colors,
                         Image<sRGB>*& enh_colors,
                         const Image<int> &bounds,
                         int grow, int shrink){

  std::cout << "find_colors " << in.getRows() << " " << in.getCols() << std::endl;

  Image<byte> *answer = new Image<byte>(in.getRows(), in.getCols());
  Image<byte> *answer2 = new Image<byte>(in.getRows(), in.getCols());

  for (int i = 0; i < in.getRows(); i++) {
    for (int j = 0; j < in.getCols(); j++) {
      (*answer)(i,j) = 0;
    }
  }

  colors = new Image<sRGB> (in.getRows(), in.getCols());
  enh_colors = new Image<sRGB>(in.getRows(), in.getCols());
  for (int row=0; row<bounds.getRows(); row++){
    for (int col=bounds(row,0); col<=bounds(row,1); col++){
      
      if (col >= in.getCols()) {
        std::cerr << "ack! bounds wrong " <<  col << " " << in.getCols() << std::endl;
        continue;
      }

      // this pixel color
      v3d v = v3d(in(row, col))/255.;

      sRGB enh_color, norm_color;
      int id;
      Enhance(v,norm_color,id,enh_color);
      
      (*colors)(row,col) = norm_color; //sRGB(255.*u);
      (*answer)(row, col) = id;
      (*enh_colors)(row, col) = enh_color; //sRGB(red, green, blue);
    }
  }

  //  Image<byte> *answer2 = new Image<byte>(*answer); 
  Copy(bounds,*answer,*answer2);

  for (int g = 0; g < grow; g++) {
    Swap(answer,answer2);
    //std::cout << "GROW" << std::endl;
    // grow by one pixel
    for (int row=0; row<bounds.getRows(); row++){
      for (int col=bounds(row,0); col<=bounds(row,1); col++){
	for (int i = -1; i <= 1; i++) {
	  for (int j = -1; j <= 1; j++) {
	    if (i == 0 && j == 0) continue;
	    if (fabs(i) == 1 && fabs(j) == 1) continue;
	    if ((*answer)(row,col)==0) continue;
	    if (row+i < 0 || row+i >= bounds.getRows()) continue;
	    if (col+j < bounds(row,0) || col+j >= bounds(row,1)) continue;
	    if ((*answer)(row+i,col+j)!=0) continue;
	    (*answer2)(row+i,col+j) = (*answer)(row,col);
	    (*enh_colors)(row+i,col+j) = (*enh_colors)(row,col);
	  }
	}
      }
    }
    Copy(bounds,*answer2,*answer);
  }


  for (int s = 0; s < shrink; s++) {
    Swap(answer,answer2);
    // shrink by one pixel
    //std::cout << "SHRINK" << std::endl;
    for (int row=0; row<bounds.getRows(); row++){
      for (int col=bounds(row,0); col<=bounds(row,1); col++){
	for (int i = -1; i <= 1; i++) {
	  for (int j = -1; j <= 1; j++) {
	    if (i == 0 && j == 0) continue;
	    if (fabs(i) == 1 && fabs(j) == 1) continue;
	    if ((*answer)(row,col)!=0) continue;
	    if (row+i < 0 || row+i >= bounds.getRows()) continue;
	    if (col+j < bounds(row,0) || col+j >= bounds(row,1)) continue;
	    if ((*answer)(row+i,col+j)==0) continue;
	    (*answer2)(row+i,col+j) = 0; //answer(row,col);
	    (*enh_colors)(row+i,col+j) = v3d(0,0,0); //enh_colors(row,col);
	  }
	}
      }
    }
    Copy(bounds,*answer2,*answer);
  }

  if (args->write_debug_images) colors->write(args->debug_path+"04_colors"+IMAGE_FORMAT);
  if (args->write_debug_images) enh_colors->write(args->debug_path+"05_enhanced"+IMAGE_FORMAT);
  (*answer) = (*answer2);
  
  delete answer2;
  return answer;
}

Image<byte>* toss_irrelevant_colors(const Image<byte> &components, const Image<int> &bounds) {

  Image<byte>* answer = new Image<byte>(components);


  if (args->find_architectural_design) {
    return answer;
  }


  for (int row=0; row<bounds.getRows(); row++){
    for (int col=bounds(row,0); col<=bounds(row,1); col++){
      
      int r = (components(row, col) & 1) >> 0;
      int g = (components(row, col) & 2) >> 1;
      int b = (components(row, col) & 4) >> 2;

      if (!args->find_army_soldiers) {
	// throw out red & green
	if (r && !g && !b) {
	  (*answer)(row,col) = 0;
	}
        if (!r && g && !b) {
          (*answer)(row,col) = 0;
	}
      }
      
      if (!args->find_army_terrain) {
	// throw out yellow & cyan & blue
	if (r && g && !b) {
	  (*answer)(row,col) = 0;
	}
	if (!r && g && b) {
	  (*answer)(row,col) = 0;
	}
	if (!r && !g && b) {
	  (*answer)(row,col) = 0;
	}
      }

      // always throw out purple (?)
      if (r && !g && b) {
	(*answer)(row,col) = 0;
      }
      
    }
  }  
  return answer;
}

// ======================================================================

template <typename pixel_t>
Image<pixel_t>* CorrectRadialDistortion(Image<pixel_t> &in, double u0, double v0,
				       double k1, double k2, double a0){

  //  std::cout << "in correct radial dist" << std::endl;
  Image <pixel_t>* out = new Image<pixel_t>(in.getRows(), in.getCols());
  double denom = a0;
  for (int row=0; row<out->getRows(); row++){

    //    std::cout << ".";
    fflush(stdout);

    double y = (row - v0) / denom;
    for (int col=0; col<out->getCols(); col++){

      //std::cout << "\nmaxes" << in.getRows() << " " << in.getCols();
      double x = (col - u0) / denom;
      double r2 = (x*x + y*y);
      double r4 = r2 * r2;
      double u = col + (col - u0)*(k1 * r2 + k2 * r4);
      double v = row + (row - v0)*(k1 * r2 + k2 * r4);
      //std::cout << "\na"; 
      //std::cout << "row=" << row << "," << "col=" << col; fflush(stdout);
      pixel_t tmp = in.bilinear(v, u);
      //std::cout << "b"; fflush(stdout);
      (*out)(row, col) = tmp;
      //      std::cout << "c"; fflush(stdout);
    }
  }
  //std::cout << "leaving correct radial dist" << std::endl;
  return out;      
}

// ====================================================================
// ====================================================================



bool process_image();

int main(int argc, char **argv){

  std::cout << "=====================================\nTABLE_TOP_DETECT begin!" << std::endl;
  
  args = new ArgParser(argc,argv);

  args->RemoveMessageFile();

  if (args->write_debug_images) {
    int ret = system ("rm -f debug_images/*");
    if (ret != 0) {
      std::cout << "UNUSUAL BEHAVIOR?" << std::endl;
      throw -1;
    }
  }


  int frames_processed = 0;
  while(1) {
    bool success = process_image();
    usleep(500000);
    if (success) {
      frames_processed++;
      (*args->output) << "framesprocessed = " << frames_processed << std::endl;
      if (!args->continuous) break;
      if (args->num_frames > 0) {
        args->num_frames--;
        if (args->num_frames == 0) break;
      }
    }
  }
  return 0;
}



#include "debugging_images.h"




bool process_image() {
  Scene scene;
  DirLock image_directory(CAMERA_IMAGE_DIRECTORY);
  image_directory.Lock();
  scene.IMAGE_original = new Image<sRGB>(args->input_file.c_str());
  image_directory.Unlock();


  
  // ----------------------------------------------------------------
  // CORRECT FOR RADIAL DISTORTION
  // ----------------------------------------------------------------
  if (!scene.camera.loadCalibration(CAMERA_CALIBRATION_DAT)) {
    std::cerr << " could not open calibration file " << std::endl;
    exit(0);
  }
  double a0 = scene.camera.getA0();
  double u0 = scene.camera.getCx();
  double v0 = scene.camera.getCy();
  double k1 = scene.camera.getK1();
  double k2 = scene.camera.getK2();  
  (*args->output) << "correct_radial_distortion..." << std::endl;

  std::cout << "original " << scene.IMAGE_original->getRows() << " " << 
    scene.IMAGE_original->getCols() << std::endl;


  scene.IMAGE_undistorted = CorrectRadialDistortion(*scene.IMAGE_original,u0,v0,k1,k2,a0);
  if (args->write_debug_images) {
    scene.IMAGE_undistorted->write(args->debug_path+"01_undistort"+IMAGE_FORMAT);
  }


  std::cout << "undistorted " << scene.IMAGE_undistorted->getRows() << " " << 
    scene.IMAGE_undistorted->getCols() << std::endl;

  // ----------------------------------------------------------------
  // COLOR BALANCE
  // ----------------------------------------------------------------
  scene.IMAGE_white_balance = new Image<sRGB> (*(scene.IMAGE_undistorted));
  (*args->output) << "color balance..." << std::endl;
  try {
    ColorBalance(*scene.IMAGE_white_balance,0.15,0.7,args->verbose);
  } catch (int) {
    args->WriteMessageFile("[TABLE_TOP_DETECT] ERROR: color balance exposure fail");
    exit(1);
  }
  if (args->write_debug_images) {
    scene.IMAGE_white_balance->write(args->debug_path+"02_w_balance"+IMAGE_FORMAT);
  }


  // ----------------------------------------------------------------
  // TABLE MASK
  // ----------------------------------------------------------------
  try {
    assert (scene.table == NULL);
    scene.table = new Table(&scene.camera, TABLE_CAL_DAT);
  }
  catch (int) {
    assert (0);
  }
  // store min, max active column for each row
  Image<int> bounds = scene.table->getBounds();
  (*args->output) << "table mask..." << std::endl;
  scene.IMAGE_table_mask = scene.table->getMaskImage();
  if (args->write_debug_images) {
    //scene.IMAGE_table_mask->write(args->debug_path+"03_mask"+IMAGE_FORMAT);
  }


  // ----------------------------------------------------------------
  // FIND CONNECTED COMPONENTS
  // ----------------------------------------------------------------
  (*args->output) << "find components..." << std::endl;

  std::cout << "input size " << scene.IMAGE_white_balance->getRows() << " " << 
    scene.IMAGE_white_balance->getCols() << std::endl;

  scene.IMAGE_all_components = find_colors(*scene.IMAGE_white_balance, 
                                           scene.IMAGE_colors,
                                           scene.IMAGE_enhanced_colors,
                                           bounds,2,2); 
  scene.IMAGE_selected_components = toss_irrelevant_colors(*scene.IMAGE_all_components,bounds);
  int numlabels;
  scene.IMAGE_raw_labels = new Image<int>(ConnectedComponents(*scene.IMAGE_selected_components, numlabels));
  (*args->output) << "create labels..." << std::endl;
  if (args->write_debug_images) {
    CreateLabelsImage(*scene.IMAGE_raw_labels,args->debug_path+"06_debug_labels"+IMAGE_FORMAT,args->mtrand);
  }
  scene.blobs.resize(numlabels);
  scene.IMAGE_labels_visualization = new Image<sRGB>(scene.IMAGE_raw_labels->getRows(), scene.IMAGE_raw_labels->getCols());
  for (int row=0; row<scene.IMAGE_labels_visualization->getRows(); row++){
    for (int col=0; col<scene.IMAGE_labels_visualization->getCols(); col++){
      int r = (*scene.IMAGE_undistorted)(row, col).r() * 0.8;
      int g = (*scene.IMAGE_undistorted)(row, col).g() * 0.8;
      int b = (*scene.IMAGE_undistorted)(row, col).b() * 0.8;
      (*scene.IMAGE_labels_visualization)(row, col) = sRGB(r,g,b); 
    }
  }


  // process rest of rows
  for (int row=1; row<bounds.getRows()-1; row++){
    int *min_col = new int[numlabels];
    int *max_col = new int[numlabels];
    for (int i=0; i<numlabels; i++){
      min_col[i] = bounds(row,1);
      //max_col[i] = std::min(bounds(row,0),scene.IMAGE_white_balance->getCols());
      max_col[i] = bounds(row,0);
    }
    for (int col=bounds(row,0)+1; col<=bounds(row,1)-1; col++){
      int label = (*scene.IMAGE_raw_labels)(row,col);
      if (label) {
        assert(label > 0 && label < numlabels);
	// update object's mass and collection of points
	scene.blobs[label].add_point(Point(row,col));
	scene.blobs[label].incr_mass(); 
	min_col[label] = std::min(col,min_col[label]);
	max_col[label] = std::max(col,max_col[label]);
	// edge detection : edge points are pixels that border the edge of the
	// object (4-connected not 8-connected)
	if (((*scene.IMAGE_raw_labels)(row-1,col) == 0 || 
             (*scene.IMAGE_raw_labels)(row,col-1) == 0 ||
	     (*scene.IMAGE_raw_labels)(row+1,col) == 0 ||
             (*scene.IMAGE_raw_labels)(row,col+1) == 0)){
	  scene.blobs[label].add_edge_point(Point(row,col));
	  (*scene.IMAGE_labels_visualization)(row,col) = sRGB(63, 63, 63);
	}
      }
    }
    for (int i=0; i<numlabels; i++){
      if (max_col[i] >= min_col[i]){
	double r;
	r = sqrt(square(row-scene.table->getCenterRow())+square(min_col[i] - scene.table->getCenterCol()));
	scene.blobs[i].adjust_max_radius(r);
	r = sqrt(square(row-scene.table->getCenterRow())+square(max_col[i] - scene.table->getCenterCol()));
	scene.blobs[i].adjust_max_radius(r);
	// add to object's area : note that this may differ from mass, since
	// mass does not count holes in the middle of the object component
	scene.blobs[i].increase_area(max_col[i] - min_col[i] + 1);
      }
    }
    delete [] max_col;
    delete [] min_col;
  }

  // try entropy of color classes in an object as test:
  // real walls should have low entropy (exclude windows?)
  //
  // 

  // maximum object area (pixels) that can touch
  // outside edge of table for valid image - used to reject images
  // with hands 
  int max_area_thresh = 10000; 
  double min_aspect_ratio = 0.2;  // only long-thin objects allowed at tbl edge
  int border_pixels = 50; // from outside of table
  for (int i=1; i<numlabels; i++){
    //double my_area = scene.objects[i].area;
    if ((scene.blobs[i].get_area() > max_area_thresh || 
	 scene.blobs[i].get_aspect() > min_aspect_ratio) &&
	scene.blobs[i].get_max_radius() > (scene.table->getRadiusPixels()-border_pixels)){
      //fprintf(stderr, "Object crossing table boundary; walls not located\n");
      //fprintf(stderr, "area = %d\n", scene.blobs[i].area);
      // return -1;
    }
    scene.blobs[i].compute_gradients((*scene.IMAGE_raw_labels), i);
    Object* obj = scene.blobs[i].classify(&scene);
    if (obj != NULL) {
      //std::cout << "blob is not null" << std::endl;
      scene.objects.push_back(obj);
    } else {
      //std::cout << "blob is null" << std::endl;
    }
  }

  scene.assign_room_colors();
  
  scene.draw(*scene.IMAGE_labels_visualization);
  if (args->write_debug_images) scene.IMAGE_labels_visualization->write(args->debug_path+"07_labels"+IMAGE_FORMAT);

  DirLock wallfile_directory(WALL_FILE_DIRECTORY); 
  wallfile_directory.Lock(); 
  scene.write(args->output_file.c_str());
  wallfile_directory.Unlock(); 
  std::cout << "TABLE_TOP_DETECT finished!\n=====================================" << std::endl;

  args->WriteMessageFile("[TABLE_TOP_DETECT] success");

  return true;

}
