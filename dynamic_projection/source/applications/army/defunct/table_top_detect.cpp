#include <iostream>
#include <limits>

#include <vector>
#include <algorithm>
#include <map>

#include "../../common/Image.h"
#include "../../common/ImageOps.h"
#include "../../common/Vector3.h"
#include "../../common/Matrix3.h"

#include "../../common/CalibratedCamera.h"

// ======================================================================

// GLOBAL VARIABLES
std::string GLOBAL_input_file;
std::string GLOBAL_output_file;
bool GLOBAL_find_army_terrain;
bool GLOBAL_find_army_soldiers;


//#define GIGE_CAMERA
#define DEBUG_LABELS_IMAGE

// ======================================================================

const bool WRITE_DEBUG_IMAGES = true;

const double pi = 3.14159265358979323846;

const sRGB RED     (255,   0,   0);
const sRGB GREEN   (  0, 255,   0);
const sRGB BLUE    (  0,   0, 255);
const sRGB CYAN    (  0, 255, 255);
const sRGB MAGENTA (255,   0, 255);
const sRGB YELLOW  (255, 255,   0);

const int RED_IDX     = 1;
const int GREEN_IDX   = 2;
const int YELLOW_IDX  = 3;
const int BLUE_IDX    = 4;
const int MAGENTA_IDX = 5;
const int CYAN_IDX    = 6;

// north arrow is this color
const int ARROW_COLOR_IDX = RED_IDX;

// wall heights by color
const double RED_HEIGHT   = 0.254;   // 10"
const double BLUE_HEIGHT  = 0.2032;  // 8"
const double GREEN_HEIGHT = 0.127;   // 5"

// wall tip extensions
const double wall_tips        = 0.01143;// 0.45"
//const double wall_tips        = 0.02; // hack: ensure closed cornell box
const double curved_wall_tips = 0.0127; // 0.5"

// window keywords by color
const char *CYAN_KEYWORD = "cyan";
const char *MAGENTA_KEYWORD = "magenta";
const char *YELLOW_KEYWORD = "yellow";

// color token markers
const int SPECIFIC_WALL_TOKEN_IDX = RED_IDX;
const int GLOBAL_WALL_TOKEN_IDX   = BLUE_IDX;
const int FLOOR_TOKEN_IDX         = GREEN_IDX;
const int FLOOR_TOKEN_IDX2        = CYAN_IDX;
const int CEILING_TOKEN_IDX       = YELLOW_IDX;

// ======================================================================

bool isWallColorIdx(int idx){
  switch (idx){
  case RED_IDX:
  case GREEN_IDX:
  case BLUE_IDX:
    return true;
    break;
  default:
    return false;
  }
}

bool isWindowColorIdx(int idx){
  switch (idx){
  case CYAN_IDX:
  case MAGENTA_IDX:
  case YELLOW_IDX:
    return true;
    break;
  default:
    return false;
  }
}

double wall_idx_to_height(int wall_idx){
  switch(wall_idx){
  case RED_IDX:
    return RED_HEIGHT;
    break;
  case GREEN_IDX:
    return GREEN_HEIGHT;
    break;
  case BLUE_IDX:
    return BLUE_HEIGHT;
    break;
  default:
    fprintf(stderr, "invalid wall color index\n");
    return 0.;
  }
}

// ======================================================================

class Table {
public:
  Table(CalibratedCamera *camera, const char *filename){
    this->camera = camera;
    load_file(filename);
  }

  Image<int> getBounds(){
#ifdef FIREWIRE_CAMERA
    const double table_edge_border = 40.0;
#endif
#ifdef GIGE_CAMERA
    const double table_edge_border = 5.0;
    //const double table_edge_border = 0; //5.0;
#endif

    double rad = radius_pixels - table_edge_border;
    
    Image<int> bounds(camera->getRows(), 2);
    double r2 = rad * rad;
    for (int row=0; row<bounds.getRows(); row++){
      double det = r2 - (row-center_row)*(row-center_row);
      if (det > 0.){
	int w =  int(sqrt(r2 - (row-center_row)*(row-center_row)));
	bounds(row, 0) = int(center_col - w);
	bounds(row, 1) = int(center_col + w);
      } else {
	bounds(row, 0) = 0;
	bounds(row, 1) = 0;
      }
    }
    return bounds;
  }

  int getCenterRow(){
    return center_row;
  }

  int getCenterCol(){
    return center_col;
  }

  int getRadiusPixels(){
    return radius_pixels;
  }

  // return the table center in 3D world coordinates
  v3d getCenterWorld(){
    return camera->PointFromPixel(center_row, center_col, 0.);
  }

  // return the radius in meters
  double getRadius(){
    v3d c = getCenterWorld();
    // find a point on the edge of the circle
    v3d p = camera->PointFromPixel(center_row+radius_pixels, center_col, 0.);
    // distance between these points is the table radius
    return (p - c).length();
  }

  Image<byte> getMaskImage(){
    Image<byte> mask(camera->getRows(), camera->getCols());
    double rad2 = radius_pixels * radius_pixels;
    for (int row=0; row<mask.getRows(); row++){
      for (int col=0; col<mask.getCols(); col++){
	double r = (row - center_row) * (row - center_row) +
	  (col - center_col) * (col - center_col);
	if (r < rad2){
	  mask(row, col) = 255;
	} else {
	  mask(row, col) = 0;
	}
      }
    }
    return mask;
  }

private:
  CalibratedCamera *camera;
  double center_row, center_col;
  double radius_pixels;
  int min_row, max_row;
  int min_col, max_col;

  void load_file(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "Unable to open file %s\n", filename);
      exit(-1);
    }
    
    char comment[1024];
    fgets(comment, 1024, fp);
    fgets(comment, 1024, fp);
    fscanf(fp, "%lf %lf", &center_row, &center_col);

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    fscanf(fp, "%lf", &radius_pixels);

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    fgets(comment, 1024, fp);

    fscanf(fp, "%d %d %d %d", &min_row, &max_row, &min_col, &max_col);

    fclose(fp);
  }
};

// ======================================================================

struct point
{
  double row;
  double col;
  point(){}
  point(double row, double col){this->row = row; this->col = col;}
  double distance(point p){
    return sqrt((p.row-row)*(p.row-row)+(p.col-col)*(p.col-col));
  }
  void dump() {printf("%lf %lf\n", row, col);}
};

struct vector2
{
  double x;
  double y;
  vector2(){};
  vector2(double x, double y){this->x = x; this->y = y;};
  void normalize(){
    double len = sqrt(x*x+y*y);
    x /= len;
    y /= len;
  }
};

// ======================================================================

// a b
// c d
struct matrix2x2
{
  double a, b, c, d; 
  matrix2x2(double a, double b, double c, double d)
  {
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
  };
  double trace(){return a+d;};
  double det(){return a*d-b*c;};
  double l1() // eigenvalue (not necessarily leading one !!!)
  {
    return trace()/2 + sqrt(trace()*trace()/4 - det());
  };  
  double l2() // eigenvalue
  {
    return trace()/2 - sqrt(trace()*trace()/4 - det());
  };
  vector2 evect1() // eigenvector for l1()
  {
    if (c != 0){
      return vector2(l1()-d, c);
    } else if (b != 0){
      return vector2(b, l1()-d);
    } else {
      return vector2(0.0, 1.0);
    }
  };
  vector2 evect2() // eigenvector for l2()
  {
    if (c != 0){
      return vector2(l2()-d, c);
    } else if (b != 0){
      return vector2(b, l2()-d);
    } else {
      return vector2(1.0, 0.0);
    }
  };  
  matrix2x2 inverse()
  {
    double ia, ib, ic, id;
    double dt = det();
    ia =  d / dt;
    ib = -b / dt;
    ic = -c / dt;
    id =  a / dt;
    return matrix2x2(ia, ib, ic, id);
  };
  vector2 operator*(const vector2 &v)
  {
    vector2 temp;
    temp.x = a * v.x + b * v.y;
    temp.y = c * v.x + d * v.y;
    return temp;
  };
};

// ======================================================================

// ax + by + c = 0
// note x = col; y = row
struct line
{
  double a;
  double b;
  double c;
  line(){};

  line(double a, double b, double c)
  {
    this->a = a;
    this->b = b;
    this->c = c;
    normalize();
  }

  line(std::vector<point> &points, double inlier_thresh = 0.){
    fit(points, inlier_thresh, false);
    if (inlier_thresh > 0.){
      fit(points, inlier_thresh, true);
    }
  }

  void refit(std::vector<point> &points, double inlier_thresh = 0.){
    fit(points, inlier_thresh, true);
  }

  void fit (std::vector<point> &points, double inlier_thresh = 0., bool 
            init = false){
    vector2 m(0.0, 0.0);
    int n = 0;
    
    std::vector<point>::iterator pt;
    for (pt = points.begin(); pt != points.end(); ++pt){
      if (!init || point_error(*pt) < inlier_thresh){
        m.x += pt->col;
        m.y += pt->row;
        n++;
      }
    }
    m.x /= double(n);
    m.y /= double(n);
    
    double sxx = 0.0;
    double sxy = 0.0;
    double syy = 0.0;
    for (pt = points.begin(); pt != points.end(); ++pt){
      if (!init || point_error(*pt) < inlier_thresh){
        sxx += (pt->col - m.x) * (pt->col - m.x);
        sxy += (pt->col - m.x) * (pt->row - m.y);
        syy += (pt->row - m.y) * (pt->row - m.y);
      }
    }
    
    matrix2x2 cov(sxx/n, sxy/n, sxy/n, syy/n);
    vector2 v;
    if (fabs(cov.l1()) > fabs(cov.l2())){
      v = cov.evect1(); 
    } else {
      v = cov.evect2(); 
    }
    a = -v.y;
    b = v.x;
    c = -(a * m.x + b * m.y); 
    normalize();
    //printf("%u %f %f %f\n", points.size(), a, b, c);
  }

  void normalize()
  {
    double d = sqrt(a*a+b*b);
    a /= d;
    b /= d;
    c /= d;
  }

  bool side(point &p){
    double d = a*p.col + b*p.row + c;
    if (d < 0.){
      return true;
    } else {
      return false;
    }
  }

  // +1 one side
  // -1 other side
  //  0 to close to call (within tolerance of line)
  int side_or_line(point &p, double tolerance = 0.){
    double d = a*p.col + b*p.row + c;
    if (d > tolerance){
      return +1;
    } else if (d < -tolerance){
      return -1;
    }
    return 0;
  }

  double point_error(point &p){
    return  fabs(a * p.col + b * p.row + c);
  }

  double point_error(std::vector<point> &points){
    double err_sum = 0.;
    for (unsigned i=0; i<points.size(); i++){
      err_sum += point_error(points[i]);
    }
    return err_sum / points.size();
  }

  int points_closer_than(std::vector<point> &points, double inlier_thresh){
    int count = 0;
    for (unsigned i=0; i<points.size(); i++){
      if(point_error(points[i]) < inlier_thresh){
        count++;
      }
    }
    return count;
  }

  void dump() {printf("%lf %lf %lf\n", a, b, c);};
};

// ======================================================================

template <class T>
void Copy(const Image<int> &bounds, const Image<T> &a, Image<T> &b) {
  assert (a.getRows() == b.getRows());
  assert (a.getCols() == b.getCols());
  for (int row=0; row<a.getRows(); row++){
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


Image<byte>* find_colors(Image<sRGB> &in, Image<int> &bounds, int grow, int shrink){
#ifndef GIGE_CAMERA
  assert(0);
  exit(0);
#endif

  Image<byte> *ids = new Image<byte>(in.getRows(), in.getCols());
  Image<sRGB> colors(in.getRows(), in.getCols());
  Image<sRGB> enh_colors(in.getRows(), in.getCols());
  for (int row=0; row<bounds.getRows(); row++){
    for (int col=bounds(row,0); col<=bounds(row,1); col++){
      
      // this pixel color
      v3d v = v3d(in(row, col))/255.;
      
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
      colors(row,col) = sRGB(255.*u);
      
      int id = 0;
      int r, g, b;
      int red = 0;
      int green = 0;
      int blue = 0;

      double b_thresh = 0.75;
      if (v.g()/v.b() > 0.5){
	//b_thresh = 0.99;
      }
      if (v.b() > b_thresh){
	id |= 4;
	blue = 255;
      }

      double g_thresh = 0.25;
      if (v.b()/v.g() > 2.7){
	g_thresh = 0.5;
      }
      if (v.g() > g_thresh){
	id |= 2;
	green = 255;
      }

      //double r_thresh = 0.125;
      double r_thresh = 0.6;
      if (v.r() > r_thresh){
	id |= 1;
	red = 255;
      }

      (*ids)(row, col) = id;
      enh_colors(row, col) = sRGB(red, green, blue);
    }
  }

  
  //Image<byte> *ids2 = new Image<byte>(ids->getRows(),ids->getCols());
  Image<byte> *ids2 = new Image<byte>(*ids); 
  Copy(bounds,*ids,*ids2);

  for (int g = 0; g < grow; g++) {
    Swap(ids,ids2);
    std::cout << "GROW" << std::endl;
    // grow by one pixel
    for (int row=0; row<bounds.getRows(); row++){
      for (int col=bounds(row,0); col<=bounds(row,1); col++){
	for (int i = -1; i <= 1; i++) {
	  for (int j = -1; j <= 1; j++) {
	    if (i == 0 && j == 0) continue;
	    if (fabs(i) == 1 && fabs(j) == 1) continue;
	    if ((*ids)(row,col)==0) continue;
	    if (row+i < 0 || row+i >= bounds.getRows()) continue;
	    if (col+j < bounds(row,0) || col+j >= bounds(row,1)) continue;
	    if ((*ids)(row+i,col+j)!=0) continue;
	    (*ids2)(row+i,col+j) = (*ids)(row,col);
	    enh_colors(row+i,col+j) = enh_colors(row,col);
	  }
	}
      }
    }
    Copy(bounds,*ids2,*ids);
  }


  for (int s = 0; s < shrink; s++) {
    Swap(ids,ids2);
    // shrink by one pixel
    std::cout << "SHRINK" << std::endl;
    for (int row=0; row<bounds.getRows(); row++){
      for (int col=bounds(row,0); col<=bounds(row,1); col++){
	for (int i = -1; i <= 1; i++) {
	  for (int j = -1; j <= 1; j++) {
	    if (i == 0 && j == 0) continue;
	    if (fabs(i) == 1 && fabs(j) == 1) continue;
	    if ((*ids)(row,col)!=0) continue;
	    if (row+i < 0 || row+i >= bounds.getRows()) continue;
	    if (col+j < bounds(row,0) || col+j >= bounds(row,1)) continue;
	    if ((*ids)(row+i,col+j)==0) continue;
	    (*ids2)(row+i,col+j) = 0; //ids(row,col);
	    enh_colors(row+i,col+j) = v3d(0,0,0); //enh_colors(row,col);
	  }
	}
      }
    }
    Copy(bounds,*ids2,*ids);
  }

  if (WRITE_DEBUG_IMAGES) colors.write("debug_labels/colors.ppm");
  if (WRITE_DEBUG_IMAGES) enh_colors.write("debug_labels/enh_colors.ppm");
  delete ids2;
  return ids;
}

// ======================================================================

template <typename Pixel>
class PixelSetter : public ImagePointFunctor<Pixel>
{
 public:
  PixelSetter(Pixel val){
    this->val = val;
  }
  void operator()(Pixel& pixel, int row, int col){
    pixel = val;
  }
 private:
  Pixel val;
};

matrix2x2 point_stats(std::vector<point> &points, vector2 &mean)
{
  std::vector<point>::iterator pt;
  mean = vector2(0.0, 0.0);
  int n = points.size();

  for (pt = points.begin(); pt != points.end(); ++pt){
    mean.x += pt->col;
    mean.y += pt->row;
  }
  mean.x /= double(n);
  mean.y /= double(n);

  double sxx = 0.0;
  double sxy = 0.0;
  double syy = 0.0;
  for (pt = points.begin(); pt != points.end(); ++pt){
    sxx += (pt->col - mean.x) * (pt->col - mean.x);
    sxy += (pt->col - mean.x) * (pt->row - mean.y);
    syy += (pt->row - mean.y) * (pt->row - mean.y);
  }

  matrix2x2 cov(sxx/n, sxy/n, sxy/n, syy/n);
  return cov;
}

// ======================================================================
// CIRCLE
// ======================================================================

class Circle {
public:
  Circle(){
    xc = yc = r = 0;
  }
  Circle (double xc, double yc, double r){
    this->xc = xc;
    this->yc = yc;
    this->r = r;
  }
  Circle(std::vector<point> &points, point com = point(0,0), 
         double max_inlier_thresh = 0., double min_inlier_thresh = 0.){
    if (max_inlier_thresh > 0.){
      const int steps = 10;
      fit(points, com, max_inlier_thresh, false);
      for (int i=1; i<steps; i++){
	double inlier_thresh = max_inlier_thresh - 
	  ((max_inlier_thresh - min_inlier_thresh) * i) / (steps-1);
	fit(points, com, inlier_thresh, true);
      }
    } else {
      fit(points, com, 0., false);
      fit(points, com, min_inlier_thresh, true);
    }
  }

  double point_error(point &p){
    double rp = sqrt((p.col - xc)*(p.col - xc) + (p.row - yc)*(p.row - yc));
    return fabs(rp - r);
  }

  double point_error(std::vector<point> &points){
    double sum = 0.;
    for (unsigned i=0; i<points.size(); i++){
      sum += point_error(points[i]);
    }
    return sum / points.size();
  }

  double get_xc(){return xc;}
  double get_yc(){return yc;}
  double get_r(){return r;}


  void set_xc(double xc){this->xc = xc;}
  void set_yc(double yc){this->yc = yc;}
  void set_r(double r){this->r = r;}

  bool inside(point &p){
    if (r*r > ((p.col-xc)*(p.col-xc) + (p.row-yc)*(p.row-yc))){
      return true;
    } else {
      return false;
    }
  }

  // com = center of mass
  void fit(std::vector<point> &points, point &com, 
	   double inlier_thresh, bool init){
    double sxx = 0.;
    double sxy = 0.;
    double syy = 0.;
    double  sx = 0.;
    double  sy = 0.;
    double   n = 0.;
    double sx3 = 0.;
    double sy3 = 0.;
    double sxy2 = 0.;
    double sx2y = 0.;

    // note: points centered before fitting
    for (unsigned i=0; i<points.size(); i++){
      if (!init || point_error(points[i]) < inlier_thresh){
	double x2 = (points[i].col - com.col) * (points[i].col - com.col);
	sxx += x2;
	sx3 += x2 * (points[i].col - com.col);
	
	sxy += (points[i].col - com.col) * (points[i].row - com.row); 
	
	double y2 = (points[i].row - com.row) * (points[i].row - com.row); 
	syy += y2;
	sy3 += y2 * (points[i].row - com.row); 
	
	sxy2 += (points[i].col - com.col) * y2;
	sx2y += x2 * (points[i].row - com.row);
	
	sx += (points[i].col - com.col);
	sy += (points[i].row - com.row);
	n += 1;
      }
    }
      
    m3d A(sxx, sxy, sx,
	  sxy, syy, sy,
	  sx,   sy,  n);
    v3d b(sx3 + sxy2, sx2y + sy3, sxx + syy);
    v3d fit = -(A.inverse() * b);
    
    // note: add center of mass back to circle center
    xc = com.col - 0.5*fit.x();
    yc = com.col - 0.5*fit.y();
    r = sqrt(0.25*(fit.x()*fit.x()+fit.y()*fit.y()) - fit.z());
  }
private:
  double xc;
  double yc;
  double r;
};

// ======================================================================
// line
// ======================================================================

line ransac_line(std::vector<point> &edge_points, 
		 std::vector<point> &points){
  if (edge_points.size() < 2){
    fprintf(stderr, "too few points in ransac_line()\n");
    exit(-1);
  }

  const int maxiter = 200;
  line best_line;
  int best_score = 0;
  for (int iter=0; iter<maxiter; iter++){

    // choose a line between two random points
    int i1, i2 = rand() % edge_points.size();
    do {
      i1 = rand() % edge_points.size();
    } while(i1 == i2);
    std::vector<point> pts;
    pts.push_back(edge_points[i1]);
    pts.push_back(edge_points[i2]);
    line l(pts);

    // fit line with points closer than threshold
    double inlier_thresh = 2.0;
    for (int i=0; i<3; i++){
      l.refit(edge_points, inlier_thresh);
    }

    int num_inliers = l.points_closer_than(edge_points, inlier_thresh);
    if (num_inliers < best_score){
      continue;
    }

    // reject line if most points are not on one side
    int count1 = 0;
    int count2 = 0;
    double side_tolerance = 2.5;
    for (unsigned i=0; i<points.size(); i++){
      int side = l.side_or_line(points[i], side_tolerance);
      if (side < 0){
	count1++;
      } else if (side > 0) {
	count2++;
      }
    }
    double side_thresh = 0.05;
    if (min(count1, count2)/double(points.size()) > side_thresh){
      continue;
    }

    best_score = num_inliers;
    best_line = l;
  }

  return best_line;
}

struct linepair {
  linepair(const line &l1, const line &l2){
    this->l1 = l1;
    this->l2 = l2;
    dot = l1.a * l2.a + l1.b * l2.b;
    matrix2x2 m(l1.a, l1.b, l2.a, l2.b);
    vector2 v = m.inverse() * vector2(-l1.c, -l2.c);  
    intersection = point(v.y, v.x);    
  }
  line l1;
  line l2;
  double dot;
  point intersection;
};

// sort predicate for linepairs - sorted by perpendicularity
bool linepair_by_dot(const linepair &a, const linepair &b){
  return fabs(a.dot) < fabs(b.dot);
}

/*
class OpenQuad {
public:
  point corner_points[4];
  line edges[4];
  int edge_side[4];
  bool valid;
  v3d world_points[4];

  OpenQuad(){}
  OpenQuad(std::vector<point> &edge_points, 
       std::vector<point> &points){

    // don't fit quads with fewer than this many edge pixels
    if (points.size() < 200){
      valid = false;
      return;
    }

    // find first line
    line l1 = ransac_line(edge_points, points);
    std::vector<point> remaining_points;
    double inlier_thresh = 2.0;
    for (unsigned i=0; i<edge_points.size(); i++){
      double err = l1.point_error(edge_points[i]);
      if (err > inlier_thresh){
        remaining_points.push_back(edge_points[i]);
      }
    }
    
    // second line
    line l2 = ransac_line(remaining_points, points);
    std::vector<point> remaining_points2;
    for (unsigned i=0; i<remaining_points.size(); i++){
      double err = l2.point_error(remaining_points[i]);
      if (err > inlier_thresh){
        remaining_points2.push_back(remaining_points[i]);
      }
    }
    
    // third line
    line l3 = ransac_line(remaining_points2, points);
    remaining_points.clear();
    for (unsigned i=0; i<remaining_points2.size(); i++){
      double err = l3.point_error(remaining_points2[i]);
      if (err > inlier_thresh){
        remaining_points.push_back(remaining_points2[i]);
      }
    }
    
    //  fourth line
    // line l4 = ransac_line(remaining_points, points);
        
    // choose 2 most perpendicular pairs of lines to intersect for corners
    linepair pair12(l1, l2);
    linepair pair23(l2, l3);
    linepair pair31(l3, l1);
    line middle, side1, side2;    

    // identify the set of parallel lines based on least perpendicular pair
    if(linepair_by_dot(pair23, pair12) && linepair_by_dot(pair31, pair12)){
      // lines 1 and 2 are most parallel
      middle = l3;
      side1 = l1;
      side2 = l2;
      corner_points[0] = pair31.intersection;
      corner_points[1] = pair23.intersection;
    }
    else if(linepair_by_dot(pair12, pair23) && linepair_by_dot(pair31, pair23)){
      // lines 2 and 3 are most parallel
      middle = l1;
      side1 = l2;
      side2 = l3;
      corner_points[0] = pair12.intersection;
      corner_points[1] = pair31.intersection;
    }
    else if(linepair_by_dot(pair12, pair31) && linepair_by_dot(pair23, pair31)){
      // lines 1 and 3 are most parallel
      middle = l2;
      side1 = l1;
      side2 = l3;
      corner_points[0] = pair12.intersection;
      corner_points[1] = pair23.intersection;
    }
    
    // now estimate the missing corners as the inliers furthest from the intersection point for each side
    double dist = 0;
    for (unsigned i=0; i<edge_points.size(); i++){
      double err = side1.point_error(edge_points[i]);
      if (err <= inlier_thresh){
	double new_dist = edge_points[i].distance(corner_points[0]);
	if(new_dist > dist){
	  dist = new_dist;
	  corner_points[2] = edge_points[i];
	}
      }
    }

    dist = 0;
    for (unsigned i=0; i<edge_points.size(); i++){
      double err = side2.point_error(edge_points[i]);
      if (err <= inlier_thresh){
	double new_dist = edge_points[i].distance(corner_points[1]);
	if(new_dist > dist){
	  dist = new_dist;
	  corner_points[3] = edge_points[i];
	}
      }
    }

    std::cerr << "CHECK" << std::endl;

    // calculate center of 4 corners
    double xc = 0.;
    double yc = 0.;
    std::vector<point> corners;
    for (unsigned i=0; i<4; i++){
      corners.push_back(corner_points[i]);
      xc += corner_points[i].col;
      yc += corner_points[i].row;
    }
    xc /= 4.;
    yc /= 4.;

    // corner at first intersection

    // sort points into angular order around the center
    // N^2 sort (but 4^2 = 16)
    for (int i=0; i<3; i++){
      double ang1 = atan2(corners[i].row - yc, corners[i].col - xc);
      if (ang1 < 0.){
        ang1 += 2*pi;
      }    
      int closest = 0;
      double min_dist = 2*pi;
      for (int j=i+1; j<4; j++){
        double ang2 = atan2(corners[j].row - yc, corners[j].col - xc);
        if (ang2 < 0.){
          ang2 += 2*pi;
        }
        // find clockwise angular distance between points
        double dist = ang2 - ang1;
        if (dist < 0.){
          dist += 2*pi;
        }
        if (dist < min_dist){
          min_dist = dist;
          closest = j;
        }
      }
      point temp = corners[i+1];
      corners[i+1] = corners[closest];
      corners[closest] = temp;
    }
    
    for (int i=0; i<4; i++){
      corner_points[i] = corners[i];
    }
    
    valid = true;
  }


  void draw(Image<sRGB> &image){
    PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));

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
        image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col));
      }
    }
  }


  void project(CalibratedCamera &camera){
    double height = 0.;
    for (int i=0; i<4; i++){
      world_points[i] = camera.PointFromPixel(corner_points[i].row,
                                              corner_points[i].col,
                                              height);
    }    
  }

  void write(FILE *fp){
    for (int i=0; i<4; i++){
      fprintf(fp, "%+8.4f %+8.4f ",
              world_points[i].x(),  
              world_points[i].z());
    }
  }

  bool inside(point &p){
    for (int i=0; i<4; i++){
      if (edges[i].side_or_line(p) != edge_side[i]){
        return false;
      }
    }
    return true;
  }

  int num_inside(std::vector<point> &points){
    int count = 0;
    for (unsigned i=0; i<points.size(); i++){
      if (inside(points[i])){
        count++;
      }
    }
    return count;
  }

  double edge_length(int i){
    int j = (i+1)%4;
    return corner_points[i].distance(corner_points[j]);
  }

};
*/

// ======================================================================
// QUAD
// ======================================================================

class Quad {
public:
  point corner_points[4];
  line edges[4];
  int edge_side[4];
  bool valid;
  v3d world_points[4];

  std::vector<int> edge_line_ids;
  std::vector<point> e_points;

  Quad(){}
  Quad(std::vector<point> &edge_points, 
       std::vector<point> &points){

    // don't fit quads with fewer than this many edge pixels
    if (points.size() < 200 || 
      edge_points.size() < 200){
      valid = false;
      return;
    }
    
    e_points = edge_points;

    // find first line
    line l1 = ransac_line(edge_points, points);
    std::vector<point> remaining_points;
    double inlier_thresh = 2.0;
    for (unsigned i=0; i<edge_points.size(); i++){
      double err = l1.point_error(edge_points[i]);
      if (err > inlier_thresh){
        remaining_points.push_back(edge_points[i]);
	edge_line_ids.push_back(-1); // point is unassigned
      }
      else{
	edge_line_ids.push_back(0); // point is assigned to the first line
      }
    }
    
    // second line
    line l2 = ransac_line(remaining_points, points);
    std::vector<point> remaining_points2;
    //for (unsigned i=0; i<remaining_points.size(); i++){
    for(unsigned i = 0; i < edge_points.size(); i++){
      if(edge_line_ids[i] == -1){
	double err = l2.point_error(edge_points[i]); //remaining_points[i]);
	if (err > inlier_thresh){
	  remaining_points2.push_back(edge_points[i]); // remaining_points[i]);
	}
	else{
	  edge_line_ids[i] = 1; // assign point
	}
      }
    }
    
    // third line
    line l3 = ransac_line(remaining_points2, points);
    remaining_points.clear();
    //for (unsigned i=0; i<remaining_points2.size(); i++){
    for(unsigned i = 0; i < edge_points.size(); i++){
      if(edge_line_ids[i] == -1){
	double err = l3.point_error(edge_points[i]); //remaining_points2[i]);
	if (err > inlier_thresh){
	  remaining_points.push_back(edge_points[i]); //remaining_points2[i]);
	}
	else{
	  edge_line_ids[i] = 2;
	}
      }
    }
    
    std::cerr << "remaining points : " << remaining_points.size() << std::endl;

    //  fourth line
    line l4 = ransac_line(remaining_points, points);
    for(unsigned i = 0; i < edge_points.size(); i++){
      if(edge_line_ids[i] == -1){
	double err = l4.point_error(edge_points[i]);
	if (err <= inlier_thresh){
	  edge_line_ids[i] = 3;
	}
      }
    }
        
    // choose 4 most perpendicular pairs of lines to intersect for corners
    std::vector<linepair> pairs;
    pairs.push_back(linepair(l1, l2));
    pairs.push_back(linepair(l1, l3));
    pairs.push_back(linepair(l1, l4));
    pairs.push_back(linepair(l2, l3));
    pairs.push_back(linepair(l2, l4));
    pairs.push_back(linepair(l3, l4));
    std::sort(pairs.begin(), pairs.end(), linepair_by_dot);
    
    // calculate center of 4 corners
    double xc = 0.;
    double yc = 0.;
    std::vector<point> corners;
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
        ang1 += 2*pi;
      }    
      int closest = 0;
      double min_dist = 2*pi;
      for (int j=i+1; j<4; j++){
        double ang2 = atan2(corners[j].row - yc, corners[j].col - xc);
        if (ang2 < 0.){
          ang2 += 2*pi;
        }
        // find clockwise angular distance between points
        double dist = ang2 - ang1;
        if (dist < 0.){
          dist += 2*pi;
        }
        if (dist < min_dist){
          min_dist = dist;
          closest = j;
        }
      }
      point temp = corners[i+1];
      corners[i+1] = corners[closest];
      corners[closest] = temp;
    }
    
    for (int i=0; i<4; i++){
      corner_points[i] = corners[i];
    }
    
    valid = true;
  }


  void draw(Image<sRGB> &image){
    PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));
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
        image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col));
      }
    }
    

    for(unsigned int i = 0; i < e_points.size(); i++){
      sRGB color(255,255,255);
      if(edge_line_ids[i] == 0){
	color = sRGB(255,0,0);
      }
      else if(edge_line_ids[i] == 1){
	color = sRGB(0,255,0);
      }
      else if(edge_line_ids[i] == 2){
	color = sRGB(0,0,255);
      }
      else if(edge_line_ids[i] == 3){
	color = sRGB(255,255,0);
      }

      image(e_points[i].row, e_points[i].col) = color;
    }
    
  }

  void project(CalibratedCamera &camera){
    double height = 0.;
    for (int i=0; i<4; i++){
      world_points[i] = camera.PointFromPixel(corner_points[i].row,
                                              corner_points[i].col,
                                              height);
    }    
  }

  void write(FILE *fp){
    for (int i=0; i<4; i++){
      fprintf(fp, "%+8.4f %+8.4f ",
              world_points[i].x(),  
              world_points[i].z());
    }
    fprintf(fp, "\n");
  }

  bool inside(point &p){
    for (int i=0; i<4; i++){
      if (edges[i].side_or_line(p) != edge_side[i]){
        return false;
      }
    }
    return true;
  }

  int num_inside(std::vector<point> &points){
    int count = 0;
    for (unsigned i=0; i<points.size(); i++){
      if (inside(points[i])){
        count++;
      }
    }
    return count;
  }

  double edge_length(int i){
    int j = (i+1)%4;
    return corner_points[i].distance(corner_points[j]);
  }

};

// ======================================================================
// PLATFORM
// ======================================================================

class Platform {
public:
  point corner_points[4];
  //line edges[4];
  bool valid;
  v3d world_points[4];

  Platform(const Quad& q){
    for(int i = 0; i < 4; i++){
      corner_points[i] = q.corner_points[i];
      //edges[i] = q.edges[i];
    }
    valid = true;
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
    PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));

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
        image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col));
      }
    }
  }

private:
  Platform(){}
};

// ======================================================================
// RAMP
// ======================================================================

class Ramp {
public:
  point corner_points[4];
  line edges[4];
  bool valid;
  v3d world_points[4];

  Ramp(Image<byte> &component_image,
       const Quad& q, std::vector<point> &points){
    for(int i = 0; i < 4; i++){
      corner_points[i] = q.corner_points[i];
      //edges[i] = q.edges[i];
    }
    
    // first recompute edge lines based on corners
    // (redundant, fix later)
    std::vector<point> endpts(2);
    for(int i = 0; i < 3; i++){
      endpts[0] = corner_points[i];
      endpts[1] = corner_points[i+1];
      edges[i] = line(endpts);
    }
    endpts[0] = corner_points[3];
    endpts[1] = corner_points[0];
    edges[3] = line(endpts);

    // assign cyan points to each line based on minimal error
    std::vector<point> edge_inliers[4];

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
      valid = false;
    }
    else{
      // reorder edges and corner points based on orientation of ground edge
      // first create temp copies of the corners and edges
      
      point temp_corners[4];
      line temp_edges[4];
      for(int i = 0; i < 4; i++){
	temp_corners[i] = corner_points[i];
	temp_edges[i] = edges[i];
      }
      for(int i = 0, j = ground_edge_id; i < 4; i++, j = (j+3)%4){
	corner_points[3-j] = temp_corners[i];
	edges[3-j] = temp_edges[i];
      }
      
      // check for approximate aspect ratio 5:3 (add this later)

      valid = true;
    }
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
    
    for (int i=0; i<4; i++){
      v3d v1 = world_points[i] - world_points[(i+1)%4];
      printf("distance = %f\n", v1.length() * 39.3700787);
    }
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
    PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));

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
        image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col));
      }
    }
  }

private:
  Ramp(){}
};

// ======================================================================
// HISTOGRAM
// ======================================================================

class Histogram {
public:
  Histogram(int nbins){
    this->nbins = nbins;
    hist.resize(nbins);
    for (int i=0; i<nbins; i++){
      hist[i] = 0;
    }
  }
  int &operator()(int idx){
    assert(idx >= 0);
    assert(idx < nbins);
    return hist[idx];
  }
  int peak_idx(){
    int best_idx = 0;
    int max_count = hist[0];
    for (unsigned i=1; i<hist.size(); i++){
      if (hist[i] > max_count){
        max_count = hist[i];
        best_idx = i;
      }
    }
    return best_idx;
  }
private:
  int nbins;
  std::vector<int> hist;
};

class Scene;
Scene *pscene;


// ======================================================================
// RECTANGLE
// ======================================================================

class Rectangle {
public:
  Rectangle(){}

  Rectangle(line line1, line cap1, line line2, line cap2, line centerline){
    this->line1 = line1;
    this->line2 = line2;
    this->centerline = centerline;
    this->cap1 = cap1;
    this->cap2 = cap2;

    for (int i=0; i<4; i++){
      point_order[i] = i;
    }

    // create intersection points
    std::vector<linepair> pairs;
    pairs.push_back(linepair(line1, cap1));
    pairs.push_back(linepair(line1, cap2));
    pairs.push_back(linepair(line2, cap1));
    pairs.push_back(linepair(line2, cap2));

    // calculate center of 4 corners
    double xc = 0.;
    double yc = 0.;
    std::vector<point> corners;
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
	ang1 += 2*pi;
      }    
      int closest = 0;
      double min_dist = 2*pi;
      for (int j=i+1; j<4; j++){
	double ang2 = atan2(corners[j].row - yc, corners[j].col - xc);
	if (ang2 < 0.){
	  ang2 += 2*pi;
	}
	// find clockwise angular distance between points
	double dist = ang2 - ang1;
	if (dist < 0.){
	  dist += 2*pi;
	}
	if (dist < min_dist){
	  min_dist = dist;
	  closest = j;
	}
      }
      point temp = corners[i+1];
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
    PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));
    
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
        image.applyFunctorOnLine(setPixel, 
                                 int(corner_points[i].row),
                                 int(corner_points[i].col),
                                 int(corner_points[j].row),
                                 int(corner_points[j].col));
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
    linepair lp1(line1, cap1);
    linepair lp2(line1, cap2);

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

    linepair lp3(line2, cap1);
    linepair lp4(line2, cap2);

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
  line centerline;
  line line1;
  line line2;
  line cap1;
  line cap2;
  v3d world_points[4];
  point corner_points[4];
  int point_order[4];
};

// ======================================================================
// CURVED WALL
// ======================================================================

class CurvedWall {
public:

  CurvedWall(std::vector<point> &edge_points, 
             std::vector<point> &points, 
             Image<byte> &component_image,
             Circle init_circle){
    centercircle = init_circle;

    // fit inner and outer circles
    std::vector<point> inside_points;
    std::vector<point> outside_points;
    for (unsigned i=0; i<edge_points.size(); i++){
      if (centercircle.inside(edge_points[i])){
        inside_points.push_back(edge_points[i]);
      } else {
        outside_points.push_back(edge_points[i]);
      }
    }
    point com(0,0); //!!!! use the real center here
    double max_inlier_thresh = 6.0;
    double min_inlier_thresh = 1.0;
    innercircle = Circle(inside_points, com, max_inlier_thresh,
                         min_inlier_thresh);
    outercircle = Circle(outside_points, com, max_inlier_thresh,
                         min_inlier_thresh);
    
    // estimate common center
    double xc = (innercircle.get_xc() + outercircle.get_xc())/2;
    double yc = (innercircle.get_yc() + outercircle.get_yc())/2;
    double inlier_thresh = 1.;
    double r_inner = 0.;
    int count = 0;
    for (unsigned i=0; i<inside_points.size(); i++){
      if (innercircle.point_error(inside_points[i]) < inlier_thresh){
        r_inner += sqrt((inside_points[i].col - xc)*
                        (inside_points[i].col - xc)+
                        (inside_points[i].row - yc)*
                        (inside_points[i].row - yc));
        count++;
      }
    }
    r_inner /= count;
    double r_outer = 0.;
    count = 0;
    for (unsigned i=0; i<outside_points.size(); i++){
      if (outercircle.point_error(outside_points[i]) < inlier_thresh){
        r_outer += sqrt((outside_points[i].col - xc)*
                        (outside_points[i].col - xc)+
                        (outside_points[i].row - yc)*
                        (outside_points[i].row - yc));
        count++;
      }
    }
    r_outer /= count;
    innercircle.set_xc(xc);
    innercircle.set_yc(yc);
    innercircle.set_r(r_inner);
    outercircle.set_xc(xc);
    outercircle.set_yc(yc);
    outercircle.set_r(r_outer);

    // check min, max thickness
    if ((r_outer - r_inner) < 5. || (r_outer - r_inner) > 19.){
      valid = false;
      return;
    }

    // re-estimate centerline circle for angle detemination
    double r_center = 0.;
    count = 0;
    for (unsigned i=0; i<edge_points.size(); i++){
      r_center += sqrt((edge_points[i].col - xc)*
                       (edge_points[i].col - xc)+
                       (edge_points[i].row - yc)*
                       (edge_points[i].row - yc));
      count++;
    }
    r_center /= count;
    centercircle.set_xc(xc);
    centercircle.set_yc(yc);
    centercircle.set_r(r_center);
    
    // find angular interval: first, find all angles 0-2pi
    std::vector<double> theta;
    for (unsigned i=0; i<edge_points.size(); i++){
      double th = atan2(edge_points[i].row - yc, 
                        edge_points[i].col - xc);
      if (th < 0.) th += 2*pi;
      theta.push_back(th);
    }
    std::sort(theta.begin(), theta.end());

    // find angular interval: now, find largest angular gap
    double max_gap = 0.;
    for (unsigned i=0; i<theta.size(); i++){
      unsigned j = (i+1) % theta.size();
      double gap = theta[j] - theta[i];
      if (gap < 0) gap += 2*pi;
      if (gap > max_gap){
        max_gap = gap;
        min_angle = theta[j];
        max_angle = theta[i];
      }
    }
    if (max_angle < min_angle){
      max_angle += 2*pi;
    }

    // no curved walls > pi
    if (max_gap < pi){
      valid = false;
    } else {
      valid = true;
    }

    // find dominant color to get height
    if (valid){
      Histogram hist(8);
      for (unsigned i=0; i<points.size(); i++){
        hist(component_image(int(points[i].row), int(points[i].col)))++;
      }
      height = wall_idx_to_height(hist.peak_idx());
      if (height == 0.){
        valid = false;
      }
    }
  }

  // closest distance to any point on the curved wall
  double point_distance(point p){
    double closest = std::numeric_limits<double>::max();
    
    double x, y, r, dist;
    
    // check outer circle ends
    // point 1
    x = centercircle.get_xc() + outercircle.get_r()*cos(min_angle);
    y = centercircle.get_yc() + outercircle.get_r()*sin(min_angle);
    dist = p.distance(point(y, x));
    if (dist < closest) closest = dist;

    // point 2
    x = centercircle.get_xc() + outercircle.get_r()*cos(max_angle);
    y = centercircle.get_yc() + outercircle.get_r()*sin(max_angle);
    dist = p.distance(point(y, x));
    if (dist < closest) closest = dist;

    // check outer circle
    r = sqrt((p.col - centercircle.get_xc()) * 
             (p.col - centercircle.get_xc()) +
             (p.row - centercircle.get_yc()) *
             (p.row - centercircle.get_yc()));
    dist = fabs(r - outercircle.get_r());
    if (dist < closest){
      // also must be on the section of arc
      double th = atan2(p.row - centercircle.get_yc(), 
                        p.col - centercircle.get_xc());
      if (th < 0.) th += 2*pi;
      if (th >= min_angle && th <= max_angle){
        closest = dist;
      }
    }

    // check inner circle ends
    // point 1
    x = centercircle.get_xc() + innercircle.get_r()*cos(min_angle);
    y = centercircle.get_yc() + innercircle.get_r()*sin(min_angle);
    dist = p.distance(point(y, x));
    if (dist < closest) closest = dist;

    // point 2
    x = centercircle.get_xc() + innercircle.get_r()*cos(max_angle);
    y = centercircle.get_yc() + innercircle.get_r()*sin(max_angle);
    dist = p.distance(point(y, x));
    if (dist < closest) closest = dist;

    // check inner circle
    r = sqrt((p.col - centercircle.get_xc()) * 
             (p.col - centercircle.get_xc()) +
             (p.row - centercircle.get_yc()) *
             (p.row - centercircle.get_yc()));
    dist = fabs(r - innercircle.get_r());
    if (dist < closest){
      // also must be on the section of arc
      double th = atan2(p.row - centercircle.get_yc(), 
                        p.col - centercircle.get_xc());
      if (th < 0.) th += 2*pi;
      if (th >= min_angle && th <= max_angle){
        closest = dist;
      }
    }

    return closest;
  }

  void draw(Image<sRGB> &image){
    for (int i=0; i<200; i++){
      double th = min_angle + (max_angle-min_angle)*i/200.;
      int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
      int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
      if (row >= 0 && col >= 0 && 
          row < image.getRows() && col < image.getCols()){
        image(row,col) = sRGB(70, 70, 70);
      }
    }
    
    for (int i=0; i<200; i++){
      double th = min_angle + (max_angle-min_angle)*i/200.;
      int row = int(innercircle.get_yc() + innercircle.get_r()*sin(th));
      int col = int(innercircle.get_xc() + innercircle.get_r()*cos(th));
      if (row >= 0 && col >= 0 && 
          row < image.getRows() && col < image.getCols()){
        image(row,col) = sRGB(255, 255, 255);
      }
    }
    
    for (int i=0; i<200; i++){
      double th = min_angle + (max_angle-min_angle)*i/200.;
      int row = int(outercircle.get_yc() + outercircle.get_r()*sin(th));
      int col = int(outercircle.get_xc() + outercircle.get_r()*cos(th));
      if (row >= 0 && col >= 0 && 
          row < image.getRows() && col < image.getCols()){
        image(row,col) = sRGB(255, 255, 255);
      }
    }
  }

  void project(CalibratedCamera &camera, double extension, double angle_adjust){
    center = camera.PointFromPixel(int(centercircle.get_yc()),
                                   int(centercircle.get_xc()),
                                   height);
    // project corner points to determine true radiuses
    int row = int(outercircle.get_yc() + outercircle.get_r()*sin(min_angle));
    int col = int(outercircle.get_xc() + outercircle.get_r()*cos(min_angle));
    v3d p_outer = camera.PointFromPixel(row, col, height);
    outer_radius = (center - p_outer).length();

    row = int(innercircle.get_yc() + innercircle.get_r()*sin(min_angle));
    col = int(innercircle.get_xc() + innercircle.get_r()*cos(min_angle));
    v3d p_inner = camera.PointFromPixel(row, col, height);
    inner_radius = (center - p_inner).length();

    // extend tips
    double dtheta = 0.5 * extension / (inner_radius + outer_radius);
    min_angle -= dtheta;
    max_angle += dtheta;

    min_angle += angle_adjust;
    max_angle += angle_adjust;
  }

  void write(FILE *fp){
    fprintf(fp, "curved_wall %+8.4f %+8.4f %8.4f %8.4f %8.4f %8.4f %+8.4f %d\n",
            center.x(), center.z(), 
            inner_radius, outer_radius,
            min_angle, max_angle, center.y(), color_idx);
  }


  double height;
  v3d center;
  double inner_radius;
  double outer_radius;
  Circle centercircle;
  Circle innercircle;
  Circle outercircle;
  double min_angle;
  double max_angle;
  bool valid;
  int color_idx;
};

void draw_curved_wall(CurvedWall &curvedwall, Image<byte> &im){

  for (int i=0; i<200; i++){
    Circle centercircle = curvedwall.centercircle;
    double th = curvedwall.min_angle + 
      (curvedwall.max_angle-curvedwall.min_angle)*i/200.;
    int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
    int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
    if (row >= 0 && col >= 0 && 
        row < im.getRows() && col < im.getCols()){
      im(row,col) = 70;
    }
  }

  for (int i=0; i<200; i++){
    Circle centercircle = curvedwall.innercircle;
    double th = curvedwall.min_angle + 
      (curvedwall.max_angle-curvedwall.min_angle)*i/200.;
    int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
    int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
    if (row >= 0 && col >= 0 && 
        row < im.getRows() && col < im.getCols()){
      im(row,col) = 255;
    }
  }

  for (int i=0; i<200; i++){
    Circle centercircle = curvedwall.outercircle;
    double th = curvedwall.min_angle + 
      (curvedwall.max_angle-curvedwall.min_angle)*i/200.;
    int row = int(centercircle.get_yc() + centercircle.get_r()*sin(th));
    int col = int(centercircle.get_xc() + centercircle.get_r()*cos(th));
    if (row >= 0 && col >= 0 && 
        row < im.getRows() && col < im.getCols()){
      im(row,col) = 255;
    }
  }
}

// ======================================================================
// WINDOW
// ======================================================================

class Object;

class Window {
public:
  Window (Rectangle &rect, const char *window_keyword){
    rectangle = rect;
    keyword = window_keyword;
  }
  enum WindowMarker {CYAN, MAGENTA, YELLOW};
  void draw(Image<sRGB> &image){
    rectangle.draw(image);
  }
  void project(CalibratedCamera &camera, double height){
    rectangle.project(camera, height);
  }
                                            
  void write(FILE *fp){
    fprintf(fp, "window ");
    rectangle.write(fp);
    fprintf(fp, " %s\n", keyword);
  }
private:
  Object *object;
  WindowMarker marker_color;
  Rectangle rectangle;
  const char *keyword;
};

/*
// estimate pixel error in fitting a quadrilateral
unsigned quad_consistency(std::vector<point> &edge_points){
  std::vector<vector2> edgeNorms(edge_points.size());
  // compute edgel gradients
  
}
*/

// estimate pixel error in fitting a wall
// !!! this is a hack, clean this up, and don't do this calculation twice
unsigned wall_consistancy(std::vector<point> &edge_points, line centerline, 
			  double inlier_thresh){
  // fit lines to both sides
  std::vector<point> side1_points;
  std::vector<point> side2_points;
  for (unsigned i=0; i<edge_points.size(); i++){
    if (centerline.side(edge_points[i])){
      side1_points.push_back(edge_points[i]);
    } else {
      side2_points.push_back(edge_points[i]);
    }
  }
  line line1(side1_points, inlier_thresh);
  line line2(side2_points, inlier_thresh);
  int count = 0;
  for (unsigned i=0; i<side1_points.size(); i++){
    if (line1.point_error(side1_points[i]) < inlier_thresh){
      count++;
    }
  }
  for (unsigned i=0; i<side2_points.size(); i++){
    if (line2.point_error(side2_points[i]) < inlier_thresh){
      count++;
    }
  }
  return count;
}

// DOLCEA: this needs to be renamed!
// estimate pixel error in fitting a curved wall
// !!! this is a hack, clean this up, and don't do this calculation twice
unsigned curvedwall_consistancy(std::vector<point> &edge_points,
				Circle centercircle, 
			  double inlier_thresh){
  // fit inner and outer circles
  std::vector<point> inside_points;
  std::vector<point> outside_points;
  for (unsigned i=0; i<edge_points.size(); i++){
    if (centercircle.inside(edge_points[i])){
      inside_points.push_back(edge_points[i]);
    } else {
      outside_points.push_back(edge_points[i]);
    }
  }
  point com(0,0); //!!!! use the real center here
  double max_inlier_thresh = 6.0;
  double min_inlier_thresh = 1.0;
  Circle innercircle(inside_points, com, max_inlier_thresh,
		     min_inlier_thresh);
  Circle outercircle(outside_points, com, max_inlier_thresh,
		     min_inlier_thresh);
  int count = 0;
  for (unsigned i=0; i<inside_points.size(); i++){
    if (innercircle.point_error(inside_points[i]) < inlier_thresh){
      count++;
    }
  }
  for (unsigned i=0; i<outside_points.size(); i++){
    if (outercircle.point_error(outside_points[i]) < inlier_thresh){
      count++;
    }
  }
  return count;
}

// ======================================================================
// WALL
// ======================================================================

class Wall {
public:

  Wall(std::vector<point> &edge_points, line centerline,
       std::vector<point> &points, Image<byte> &component_image,
       std::vector<Wall> &previous_walls){
    this->centerline = centerline;

    valid = true;
    n_points = points.size();

    // fit lines to both sides
    std::vector<point> side1_points;
    std::vector<point> side2_points;
    for (unsigned i=0; i<edge_points.size(); i++){
      if (centerline.side(edge_points[i])){
        side1_points.push_back(edge_points[i]);
      } else {
        side2_points.push_back(edge_points[i]);
      }
    }
    double inlier_thresh = 2.0;
    line1 = line(side1_points, inlier_thresh);
    line2 = line(side2_points, inlier_thresh);

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

      printf("wall too thick = %f\n", thickness);
      valid = false;
      return;
    }
    
    // check for reasonable parallelism; reject "wedge"-shaped walls
    double dot = line1.a * line2.a + line1.b * line2.b;
    double parallel_dot_thresh = 0.95;
    if (fabs(dot) < parallel_dot_thresh){
      valid = false;
      return;
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
    std::vector<Histogram> hist(nhist, Histogram(8));
    Histogram global_hist(8);
    for (unsigned i=0; i<points.size(); i++){
      byte color = component_image(int(points[i].row),
                                   int(points[i].col));
      double d = centerline.a*points[i].row - 
        centerline.b*points[i].col;
      int idx = int(d) - min_idx;
      assert(idx >= 0);
      assert(idx < nhist);
      global_hist(color)++;
      hist[idx](color)++;
    }

    // find dominant "wall" color over entire length
    int wall_idx = RED_IDX;
    int max_count = global_hist(RED_IDX);
    if (global_hist(GREEN_IDX) > max_count){
      wall_idx = GREEN_IDX;
      max_count = global_hist(GREEN_IDX);
    }
    if (global_hist(BLUE_IDX) > max_count){
      wall_idx = BLUE_IDX;
      max_count = global_hist(BLUE_IDX);
    }

    double wall_height = wall_idx_to_height(wall_idx);
    y = wall_height;

    // classify each bin along length by dominant color
    std::vector<int> dominant_color(nhist);
    for (int i=0; i<nhist; i++){
      int best_idx = 0;
      int max_count = hist[i](0);
      for (int j=1; j<8; j++){
        if (hist[i](j) > max_count){
          max_count = hist[i](j);
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
      Histogram filter_hist(8);
      for (int j=-half_width; j<=half_width; j++){
	filter_hist(dominant_color[i+j])++;
      }
      
      // find dominant color over filter length
      // either the wall color, or a window color
      int filtered_idx = wall_idx;
      int max_count = filter_hist(filtered_idx);
      if (filter_hist(CYAN_IDX) > max_count){
	filtered_idx = CYAN_IDX;
	max_count = filter_hist(CYAN_IDX);
      }
      if (filter_hist(MAGENTA_IDX) > max_count){
	filtered_idx = MAGENTA_IDX;
	max_count = filter_hist(MAGENTA_IDX);
      }
      if (filter_hist(YELLOW_IDX) > max_count){
	filtered_idx = YELLOW_IDX;
      }
      filtered_dominant_color[i] = filtered_idx;
    }
    
    dominant_color = filtered_dominant_color;

    // cut into sections by dominant color (detect windows)
    // ignore color runs less than this number of pixels long
    // DOLCEA: took all this out

    // create cap lines
    cap1 = line(centerline.b, -centerline.a, min_d);
    cap2 = line(centerline.b, -centerline.a, max_d);

    // find endpoints on center line
    linepair pair1(cap1, centerline);
    end_point1 = pair1.intersection;
    linepair pair2(cap2, centerline);
    end_point2 = pair2.intersection;

    rectangle = Rectangle(line1, cap1, line2, cap2, centerline);

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
	point new_end1 = end_point2;
	point new_end2 = w.end_point2;
	line new_cap1 = cap2;
	line new_cap2 = w.cap2;
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
	  valid = false;
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
	  //previous_walls[i].windows.insert(previous_walls[i].windows.end(),
	  //				   windows.begin(), windows.end());
	  // note that this fails if middle section of wall is processed last
	  return;
	}
      }
    }
  }

  double point_distance(point p){
    double closest = std::numeric_limits<double>::max();

    // test the four corner points
    for (int i=0; i<4; i++){
      double d = p.distance(rectangle.corner_points[i]);
      if (d < closest) closest = d;
    }

    // test distance to centroid of corner points
    double row = 0.;
    double col = 0.;
    for (int i=0; i<4; i++){
      row += rectangle.corner_points[i].row;
      col += rectangle.corner_points[i].col;
    }
    row /= 4.;
    col /= 4.;
    
    double dist = p.distance(point(row,col));
    if (dist < closest) closest = dist;

    return closest;
  }

  void draw(Image<sRGB> &image){
    rectangle.draw(image);
    //for (unsigned i=0; i<windows.size(); i++){
    //  windows[i].draw(image);
    //}
  }

  void project(CalibratedCamera &camera){
    rectangle.project_and_extend(camera, y, wall_tips);
    //for (unsigned i=0; i<windows.size(); i++){
    //  windows[i].project(camera, y);
    //}
  }

  void write(FILE *fp){
    fprintf(fp, "wall   ");
    rectangle.write(fp);
    fprintf(fp, "%+8.4f\n", y);
    //fprintf(fp, "%+8.4f %d\n", y, color_idx);
    //fprintf(fp, "num_windows %d\n", (int)windows.size());
    //for (unsigned i=0; i<windows.size(); i++){
    //  windows[i].write(fp);
    //}
  }

  int color_idx;
  bool valid;
private:
  int n_points;
  Rectangle rectangle;
  line line1;
  line line2;
  line cap1;
  line cap2;
  line centerline;
  point end_point1, end_point2;
  double y;
  // std::vector<Window> windows;
};

// ======================================================================
// COLUMN
// ======================================================================

class Column {
public:
  Column(Image<byte> &component_image, std::vector<point> &points,
	 std::vector<point> &edge_points, CalibratedCamera &camera){
    valid = true;
    material_idx = 0;

    // find dominant color and height
    Histogram hist(8);
    for (unsigned i=0; i<points.size(); i++){
      hist(component_image(int(points[i].row), int(points[i].col)))++;
    }
    height = wall_idx_to_height(hist.peak_idx());
    if (height == 0.){
      valid = false;
      return;
    }

    // find centroid (world space)
    v3d centroid(0., 0., 0.);
    for (unsigned i=0; i<points.size(); i++){
      centroid += camera.PointFromPixel(int(points[i].row),
					int(points[i].col),
					height);
    }
    centroid = centroid / double(points.size());
    xc = centroid.x();
    zc = centroid.z();

    // estimate rough radius (world-space coords)
    double r_sum = 0.;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid - p).length();
      r_sum += r;
    }
    radius = r_sum / edge_points.size();

    // re-estimate radius, ignoring outliers
    double inlier_thresh = 1.5;
    r_sum = 0.;
    int count = 0;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid - p).length();
      if (fabs(r - radius) < inlier_thresh){
	r_sum += r;
	count++;
      }
    }
    radius = r_sum / count;

    // check for proper size 7/8" diameter = 0.0111 m radius
    //  but some are actually 1" diameter, so make tolerance larger
    double tol = 0.005;  //  +/-20% tolerance
    double upper_thresh = 0.0111 + tol;
    double lower_thresh = 0.0111 - tol;
    //    printf ("column_radius = %f (%f - %f)\n", 
    //	    radius, lower_thresh, upper_thresh);
    if (radius < lower_thresh || radius > upper_thresh){
      valid = false;
    }

    // estimate image-space center and radius for overlay image
    v3d p = camera.PixelFromPoint(centroid);
    center_row = p.y();
    center_col = p.x();
    v3d p1 = camera.PixelFromPoint(centroid + v3d(radius, 0., 0.));
    pixel_radius = (p - p1).length();
  }

  void write(FILE *fp){
    fprintf(fp, "column %+8.4f %+8.4f %+8.4f %+8.4f %d\n",
	    xc, zc, radius, height, material_idx);
  }

  void draw(Image<sRGB> &image){
    int n_pts = 300;
    for (int i=0; i<n_pts; i++){
      double th = (2.*pi*i)/n_pts;
      int r = center_row + pixel_radius * cos(th);
      int c = center_col + pixel_radius * sin(th);
      if (r >= 0 && r < image.getRows() &&
	  c >= 0 && c < image.getCols()){
	image(r, c) = sRGB(255, 255, 255);
      }
    }
  }

  bool valid;
private:
  double xc, zc;
  double height;
  double radius;
  int material_idx;
  // image space coordintes:
  double center_row;
  double center_col;
  double pixel_radius;
};

// ======================================================================
// SOLDIER
// ======================================================================

class Soldier {
public:
  Soldier(Image<byte> &component_image, std::vector<point> &points,
	  std::vector<point> &edge_points, CalibratedCamera &camera, int mat_idx){
    valid = true;
    material_idx = mat_idx;

    // find dominant color and height
    Histogram hist(8);
    for (unsigned i=0; i<points.size(); i++){
      hist(component_image(int(points[i].row), int(points[i].col)))++;
    }
    height = 0.0508; // hard-coded soldier height (about 2 inches)
    if (height == 0.){
      valid = false;
      return;
    }

    // find centroid (world space)
    v3d centroid(0., 0., 0.);
    for (unsigned i=0; i<points.size(); i++){
      centroid += camera.PointFromPixel(int(points[i].row),
					int(points[i].col),
					height);
    }
    centroid = centroid / double(points.size());
    xc = centroid.x();
    zc = centroid.z();

    // estimate rough radius (world-space coords)
    double r_sum = 0.;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid - p).length();
      r_sum += r;
    }
    radius = r_sum / edge_points.size();

    // re-estimate radius, ignoring outliers
    double inlier_thresh = 1.5;
    r_sum = 0.;
    int count = 0;
    for (unsigned i=0; i<edge_points.size(); i++){
      v3d p =  camera.PointFromPixel(int(edge_points[i].row),
				     int(edge_points[i].col),
				     height);
      double r = (centroid - p).length();
      if (fabs(r - radius) < inlier_thresh){
	r_sum += r;
	count++;
      }
    }
    radius = r_sum / count;

    std::cerr << "soldier radius: " << radius << std::endl;

    // check for proper size 7/8" diameter = 0.0111 m radius
    //  but some are actually 1" diameter, so make tolerance larger
    double avg_radius = 0.008927355; // hard-coded average
    double tol = avg_radius * 0.5; // 40% tolerance
    double upper_thresh = avg_radius + tol;
    double lower_thresh = avg_radius - tol;
    //    printf ("column_radius = %f (%f - %f)\n", 
    //	    radius, lower_thresh, upper_thresh);
    if (radius < lower_thresh || radius > upper_thresh){
      std::cerr << "invalid radius : " << radius << std::endl;
      valid = false;
    }

    // estimate image-space center and radius for overlay image
    v3d p = camera.PixelFromPoint(centroid);
    center_row = p.y();
    center_col = p.x();
    v3d p1 = camera.PixelFromPoint(centroid + v3d(radius, 0., 0.));
    pixel_radius = (p - p1).length();
  }

  void write(FILE *fp){
    fprintf(fp, "soldier %+8.4f %+8.4f %+8.4f\n", // %+8.4f %d\n",
	    xc, zc, radius); //, height, material_idx);
  }

  void draw(Image<sRGB> &image){
    sRGB tmp;
    if (material_idx == RED_IDX) {
      tmp = RED;
    } else {
      assert (material_idx == GREEN_IDX);
      tmp = GREEN;
    }

    int n_pts = 300;
    for (int i=0; i<n_pts; i++){
      double th = (2.*pi*i)/n_pts;
      int r = center_row + pixel_radius * cos(th);
      int c = center_col + pixel_radius * sin(th);
      if (r >= 0 && r < image.getRows() &&
	  c >= 0 && c < image.getCols()){
	image(r, c) = tmp;
      }
    }
    for(int i = -5; i < 6; i++){
      image(center_row + i, center_col + i) = tmp;
      image(center_row + i, center_col - i) = tmp;
      image(center_row - i, center_col + i) = tmp;
      image(center_row - i, center_col - i) = tmp;
    }
  }

  bool valid;
private:
  double xc, zc;
  double height;
  double radius;
  int material_idx;
  // image space coordintes:
  double center_row;
  double center_col;
  double pixel_radius;
};

// ======================================================================
/// UNIDENTIFIED
// ======================================================================

class UnidentifiedObject{
public:
  UnidentifiedObject(Image<byte> &component_image, 
		     std::vector<point> &points){
    valid = true;

    // estimate image-space center and radius for overlay image
    for(unsigned int i = 0; i < points.size(); i++){
      center_row += points[i].row;
      center_col += points[i].col;
    }
    center_row /= points.size();
    center_col /= points.size();
  }

  void draw(Image<sRGB> &image){
    for (int i=-10; i<11; i++){
      image(center_row + i, center_col) = sRGB(255, 255, 255);
      image(center_row, center_col + i) = sRGB(255, 255, 255);
    }
  }

  bool valid;
private:
  double center_row;
  double center_col;
};

// ======================================================================
// ======================================================================

int dominant_color_idx(std::vector<point> &points, Image<byte> &image){
  Histogram hist(8);
  for (unsigned i=0; i<points.size(); i++){
    hist(image(int(points[i].row), int(points[i].col)))++;
  }
  return hist.peak_idx();
}


// ======================================================================
// NORTH ARROW
// ======================================================================

// !!! clean up this arrowpoint stuff
// SIGGRAPH 2009 deadline hack
struct ArrowPoint {
  ArrowPoint (double d, double dist){
    this->d = d;
    this->dist = dist;
  }
  double d, dist;
};

bool ArrowPointBydist(const ArrowPoint &a, const ArrowPoint &b){
  return a.dist > b.dist;
}

class Arrow {
 public:
  Arrow(line centerline, std::vector<point> &edge_points, 
        std::vector<point> &points, Image<byte> &component_image,
        Object &object){
    valid = true;
    this->centerline = centerline;
    //this->edge_points = &edge_points;

    // has to have enough edge points
    if (edge_points.size() < 20){
      valid = false;
      return;
    }

    // has to be the right color
    int color_idx = dominant_color_idx(points, component_image);
    if (ARROW_COLOR_IDX != color_idx){
      valid = false;
      return;
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
	valid = false;
	return;
      }
    }    
    
    // find points for debug drawing
    linepair pair1(centerline, line(centerline.b, -centerline.a, min_d));
    linepair pair2(centerline, line(centerline.b, -centerline.a, mid_d));
    linepair pair3(centerline, line(centerline.b, -centerline.a, max_d));
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
    
    //    printf("original_north = %f\n", north);
  }


  void draw(Image<sRGB> &image){
    PixelSetter<sRGB> setPixel(sRGB(255, 255, 255));
    if (valid){
      if (p1.row >=0 && p1.row < image.getRows() &&
          p2.row >=0 && p2.row < image.getRows() &&
          p1.col >=0 && p1.col < image.getCols() &&
          p2.col >=0 && p2.col < image.getCols()){
        image.applyFunctorOnLine(setPixel, 
                                 int(p1.row),
                                 int(p1.col),
                                 int(p2.row),
                                 int(p2.col));
      }
    }
  }


  point p1, p2;
  line centerline;
  double north;
  bool valid;
};


// ======================================================================
// SCENE
// ======================================================================

class Scene {
public:
  std::vector<Soldier> red_soldiers;
  std::vector<Soldier> green_soldiers;
  std::vector<Object> objects;
  std::vector<Wall> walls;
  std::vector<Platform> platforms;
  std::vector<Ramp> ramps;
  std::vector<CurvedWall> curvedwalls;
  std::vector<UnidentifiedObject> ufs;
  std::vector<sRGB> wall_materials;
  std::vector<Arrow> arrows;
  std::vector<Column> columns;
  Image<byte> *component_image;
  Image<sRGB> *input_image;
  Image<byte> *table_mask;
  double north;
  sRGB floor_color;
  sRGB ceiling_color;
  CalibratedCamera camera;
  Table *table;

  Scene(){
    north = 0.;
  }

  /*
    // DOLCEA: this function used to assign tokens to walls, but now it might not be needed at all
  void assign_tokens(){
    ceiling_color = sRGB(255, 255, 255);
    floor_color = sRGB(255, 255, 255);
    sRGB default_wall_color = sRGB(255, 255, 255);
    wall_materials.push_back(default_wall_color);

    for (unsigned i=0; i<walls.size(); i++){
      walls[i].color_idx = 0;
    }
  }
  */

  void write(const char *filename){

    // approximate rotation to align pixel angles with x-z world
    // coordinate angles
    double r1 = 0.;
    double c1 = 0.;
    double r2 = 1000.;
    double c2 = 1000.;
    double height = 0.;
    v3d p1 = camera.PointFromPixel(r1, c1, height);
    v3d p2 = camera.PointFromPixel(r2, c2, height);
    double pixel_angle = atan2(r2-r1, c2-c1);
    if (pixel_angle < 0.) pixel_angle += 2*pi;
    //    printf("pixel_angle = %f\n", pixel_angle);
    double world_angle = atan2(p1.z()-p2.z(), p2.x()-p1.x());
    if (world_angle < 0.) world_angle += 2*pi;
    // printf("world_angle = %f\n", world_angle);
    double angle_adjust = -(world_angle - pixel_angle);
    if (angle_adjust < -pi) angle_adjust += 2*pi;
    if (angle_adjust >  pi) angle_adjust -= 2*pi;
    // printf("angle_adjust = %f\n", angle_adjust);

    // adjust the north direction
    north += pi;
    north += angle_adjust;
    if (north < -pi) north += 2*pi;
    if (north >  pi) north -= 2*pi;

#warning hack: align coordinate system with camera frame (correct solution)
    //angle_adjust = 0.;
    angle_adjust -= pi/2.;

    //printf("north = %f\n", north);
    
    FILE *fp = fopen(filename, "wt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", filename);
      exit(-1);
    }

    /*
    fprintf(fp, "north %+6.3f\n", north);
    fprintf(fp, "floor_material   %5.3f %5.3f %5.3f\n", 
            floor_color.r()/255.,
            floor_color.g()/255.,
            floor_color.b()/255.);
    fprintf(fp, "ceiling_material %5.3f %5.3f %5.3f\n", 
            ceiling_color.r()/255.,
            ceiling_color.g()/255.,
            ceiling_color.b()/255.);

    fprintf(fp, "num_wall_materials %d\n", (int)wall_materials.size());
    for (unsigned i=0; i<wall_materials.size(); i++){
      fprintf(fp, "%5.3f %5.3f %5.3f\n", 
              wall_materials[i].r()/255.,
              wall_materials[i].g()/255.,
              wall_materials[i].b()/255.);
    }
    */

    if (GLOBAL_find_army_terrain) {

#define TABLE
#ifdef TABLE
      v3d center = table->getCenterWorld();
      fprintf(fp, "table %f %f %f %f\n",
	      center.x(), center.y(), center.z(), table->getRadius());
#endif
      fprintf(fp, "num_walls %d\n", (int)walls.size());
      for (unsigned i=0; i<walls.size(); i++){
	walls[i].project(camera);
	walls[i].write(fp);
      }
      
      fprintf(fp, "num_columns %d\n", (int)columns.size());
      for (unsigned i=0; i<columns.size(); i++){
	columns[i].write(fp);
      }
      
      fprintf(fp, "num_platforms %d\n", (int)platforms.size());
      for (unsigned i=0; i<platforms.size(); i++){
	platforms[i].project(camera);
	platforms[i].write(fp);
      }
      
      fprintf(fp, "num_ramps %d\n", (int)ramps.size());
      for (unsigned i=0; i<ramps.size(); i++){
	ramps[i].project(camera);
	ramps[i].write(fp);
      }
      
    }  
    
    if (GLOBAL_find_army_soldiers) {    
      fprintf(fp, "num_red_soldiers %d\n", (int)red_soldiers.size());
      for (unsigned i=0; i<red_soldiers.size(); i++){
	red_soldiers[i].write(fp);
      }
      
      fprintf(fp, "num_green_soldiers %d\n", (int)green_soldiers.size());
      for (unsigned i=0; i<green_soldiers.size(); i++){
	green_soldiers[i].write(fp);
      }
    }

    fclose(fp);
  }

  void draw(Image<sRGB> &image){
    for (unsigned i=0; i<red_soldiers.size(); i++){
      red_soldiers[i].draw(image);
    }
    for (unsigned i=0; i<green_soldiers.size(); i++){
      green_soldiers[i].draw(image);
    }

    for (unsigned i=0; i<walls.size(); i++){
      walls[i].draw(image);
    }
    for (unsigned i=0; i<platforms.size(); i++){
      platforms[i].draw(image);
    }
    for (unsigned i=0; i<ramps.size(); i++){
      ramps[i].draw(image);
    }
    for (unsigned i=0; i<curvedwalls.size(); i++){
      curvedwalls[i].draw(image);
    }
    for (unsigned i=0; i<arrows.size(); i++){
      arrows[i].draw(image);
    }
    for (unsigned i=0; i<columns.size(); i++){
      columns[i].draw(image);
    }
    for (unsigned i=0; i<ufs.size(); i++){
      ufs[i].draw(image);
    }
  }
};
  
void FillObjectInImage(Image<byte> &image, std::vector<point> points,
		       bool fill){
  const int dilate = 2;

  if (fill){
    // find the vertical extents of the object
    int min_row = image.getRows()-1;
    int max_row = 0;
    for (unsigned i=0; i<points.size(); i++){
      if (points[i].row < min_row) min_row = points[i].row;
      if (points[i].row > max_row) max_row = points[i].row;
    }
    
    // find the extents along each row
    int num_rows = max_row - min_row + 1;
    int *min_col = new int[num_rows];
    int *max_col = new int[num_rows];
    for (int i=0; i<num_rows; i++){
      min_col[i] = image.getCols()-1;
      max_col[i] = 0;
    }
    for (unsigned i=0; i<points.size(); i++){
      int r = points[i].row - min_row;
      if (points[i].col < min_col[r]) min_col[r] = points[i].col;
      if (points[i].col > max_col[r]) max_col[r] = points[i].col;
    }

    // fill between extents on each row
    for (int i=0; i<num_rows; i++){
      int r = i + min_row;
      for (int col=min_col[i]; col<= max_col[i]; col++){
	image(r, col) = 0;
      }

      // dilate (only needed at edge points)
      for (int row=i+min_row-dilate; row<=i+min_row+dilate; row++){
	for (int col=min_col[i]-dilate; col<=min_col[i]+dilate; col++){
	  if (row >=0 && row < image.getRows() &&
	      col >=0 && col < image.getCols()){
	    image(row, col) = 0;
	  }
	}
	for (int col=max_col[i]-dilate; col<=max_col[i]+dilate; col++){
	  if (row >=0 && row < image.getRows() &&
	      col >=0 && col < image.getCols()){
	    image(row, col) = 0;
	  }
	}
      }
    }

    delete [] min_col;
    delete [] max_col;
  } else {			      
    // fill just the points
    for (unsigned i=0; i<points.size(); i++){
      int r = points[i].row;
      int c = points[i].col;
      for (int row=r-dilate; row<=r+dilate; row++){
	for (int col=c-dilate; col<=c+dilate; col++){
	  if (row >=0 && row < image.getRows() &&
	      col >=0 && col < image.getCols()){
	    image(row, col) = 0;
	  }
	}
      }
    }
  }
}

// ======================================================================
// OBJECT
// ======================================================================

class Object {
public:
  Object(){
    mass = 0;
    area = 0;
    aspect = 1;
    max_radius = 0.;
    for (int i=0; i<8; i++) ColorHist[i] = 0;
  }

  //private:
  std::vector<point> points;
  std::vector<point> edge_points;
  std::vector<vector2> edge_gradients;
  point center; // of mass
  int mass;
  int area;
  double aspect;
  double max_radius; // from table center, in pixels (image space)
  Scene *scene;
  int ColorHist[8];

  void compute_gradients(Image<int>& image, int label){
    for(std::vector<point>::iterator p = edge_points.begin(); p != edge_points.end(); p++){
      // sobel
      int dx = 0;
      int dy = 0;

      bool notTopRow = (p->row > 0);
      bool notBottomRow = (p->row < image.getRows());
      bool notFirstCol = (p->col > 0);
      bool notLastCol = (p->col < image.getCols());

      if(notTopRow){
	if(notFirstCol){
	  dx += -(image(p->row - 1, p->col - 1) == label);
	  dy += -(image(p->row - 1, p->col - 1) == label);
	}
	dy += -2 * (image(p->row - 1, p->col) == label);
	if(notLastCol){
	  dx += (image(p->row - 1, p->col + 1) == label);
	  dy += -(image(p->row - 1, p->col + 1) == label);
	}
      }
      
      dx += (notFirstCol) * -2 * (image(p->row, p->col - 1) == label);
      dx += (notLastCol) * 2 * (image(p->row, p->col + 1) == label);

      if(notBottomRow){
	if(notFirstCol){
	  dx += -(image(p->row + 1, p->col - 1) == label);
	  dy += (image(p->row + 1, p->col - 1) == label);
	}
	dy += 2 * (image(p->row - 1, p->col) == label);
	if(notLastCol){
	  dx += (image(p->row + 1, p->col + 1) == label);
	  dy += (image(p->row + 1, p->col + 1) == label);
	}
      }

      edge_gradients.push_back(vector2(dx,dy));
    }
  }

  void classify(){

    // toss small objects
    int mass_thresh = 100;
    if (mass < mass_thresh){
      return;
    }

    // check entropy of color class distribution
    int sum = 0;
    for (int i=0; i<8; i++){
      sum += ColorHist[i];
    }
    double entropy = 0.;
    for (int i=0; i<8; i++){
      double p = double(ColorHist[i])/double(sum);
      if (p > 0.){
	entropy +=  - p * log(p);
      }
    }
    entropy /= log(2.);  // convert nats to bits
    //    printf("entropy = %f\n", entropy);
    double max_entropy = 1.7;
    
    if (entropy > max_entropy){
      fprintf(stderr, "Entropy test failed\n");
      return; // too many mixed colors to be a wall
    }
    
    // calculate point stats
    vector2 mean;
    matrix2x2 cov = point_stats(edge_points, mean);
    double l1 = fabs(cov.l1());
    double l2 = fabs(cov.l2());
    aspect = min(l1, l2) / max(l1, l2);

    // fit line to all edge points
    vector2 v;
    if (l1 > l2){
      v = cov.evect1(); 
    } else {
      v = cov.evect2(); 
    }
    double c = v.y * mean.x - v.x * mean.y; 
    line centerline(-v.y, v.x, c);
    //double line_error = centerline.point_error(edge_points);
    double inlier_thresh = 2.0;
    int line_consistancy= wall_consistancy(edge_points, centerline,
					   inlier_thresh);

    // fit a circle to all edge points
    point com(0,0); //!!!! use the real center here
    Circle centercircle(points, com, 0., 2.);
    //    double circle_error = centercircle.point_error(edge_points);
    int circle_consistancy = curvedwall_consistancy(edge_points, centercircle,
						    inlier_thresh);

    // check color
    int color = dominant_color_idx(points, *scene->component_image);

    //    if (line_error < circle_error){

    /* // stuff for detecting bunkers later maybe
    if(entropy > max_entropy){
      if(circle_consistancy < line_consistancy){
	// throw it out
	return;
      }
      // otherwise see if its a curved wall
      CurvedWall curvedwall(edge_points, points, *scene->component_image, 
			    centercircle);
      if(curvedwall.valid){
	scene->curvedwalls.push_back(curvedwall);
	FillObjectInImage(*scene->table_mask, points, false);
      }
      else{
	return;
      }
    }
    */

    if(color == BLUE_IDX){
      // wall or column
      if (line_consistancy > circle_consistancy){
	// it's a wall
	Wall wall(edge_points, centerline, points, *scene->component_image,
		  scene->walls);
	if (wall.valid){
	  scene->walls.push_back(wall);
	  FillObjectInImage(*scene->table_mask, points, false);
	}
	else{
	  UnidentifiedObject uf(*scene->component_image, points);
	  scene->ufs.push_back(uf);
	}
      }
      else {
	// could be a column (assume no curved walls for now)
#ifdef GIGE_CAMERA
	
	Column column(*scene->component_image, points,
		      edge_points, scene->camera);
	
	if (column.valid){
	  scene->columns.push_back(column);
	} 
	else {
	  UnidentifiedObject uf(*scene->component_image, points);
	  scene->ufs.push_back(uf);
	}
	FillObjectInImage(*scene->table_mask, points, true);
      }
#endif // #ifdef GIGE_CAMERA
    }
    else if(color == YELLOW_IDX){
      // ramp or platform
      Quad quad(edge_points, points);
      bool validQuadObject = false;
      if (quad.valid){
	Ramp ramp(*scene->component_image, quad, points);
	if(ramp.valid){
	  // found a ramp!
	  std::cout << "found a ramp!" << std::endl;
	  validQuadObject = true;
	  scene->ramps.push_back(ramp);
	  FillObjectInImage(*scene->table_mask, points, false);
	}
	else{
	  Platform plat(quad);
	  if(plat.valid){
	    std::cout << "found a platform!" << std::endl;
	    validQuadObject = true;
	    scene->platforms.push_back(plat);
	    FillObjectInImage(*scene->table_mask, points, false);
	  }
	}
      }
      
      if(!validQuadObject){
	UnidentifiedObject uf(*scene->component_image, points);
	scene->ufs.push_back(uf);
      }
    } else if (color == RED_IDX || color == GREEN_IDX) {
      // army man!
      
      Soldier soldier(*scene->component_image, points,
		      edge_points, scene->camera, color);
      
      if (soldier.valid){
	if(color == RED_IDX){
	  scene->red_soldiers.push_back(soldier);
	}
	else{
	  scene->green_soldiers.push_back(soldier);
	}
      }
    } else {
      UnidentifiedObject uf(*scene->component_image, points);
      scene->ufs.push_back(uf);
      std::cerr << "Unidentified army object." << std::endl;
    }
    FillObjectInImage(*scene->table_mask, points, true);   
  }
};



template <typename pixel_t>
Image<pixel_t> CorrectRadialDistortion(Image<pixel_t> &in, double u0, double v0,
				       double k1, double k2, double a0){
  Image <pixel_t> out(in.getRows(), in.getCols());
  double denom = a0;
  for (int row=0; row<out.getRows(); row++){
    double y = (row - v0) / denom;
    for (int col=0; col<out.getCols(); col++){
      double x = (col - u0) / denom;
      double r2 = (x*x + y*y);
      double r4 = r2 * r2;
      double u = col + (col - u0)*(k1 * r2 + k2 * r4);
      double v = row + (row - v0)*(k1 * r2 + k2 * r4);
      out(row, col) = in.bilinear(v, u);
    }
  }
  return(out);      
}

// ====================================================================
// ====================================================================


void helper ();

int main(int argc, char **argv){
  
  for (int i = 1; i < argc; i++) {
    if (std::string(argv[i]) == "-input" ||
	std::string(argv[i]) == "-i") {
      i++;
      assert (i < argc);
      GLOBAL_input_file = argv[i];
    } else if (std::string(argv[i]) == "-output" ||
	       std::string(argv[i]) == "-o") {
      i++;
      assert (i < argc);
      GLOBAL_output_file = argv[i];
    } else if (std::string(argv[i]) == "-find_army") {
      GLOBAL_find_army_terrain = true;
      GLOBAL_find_army_soldiers = true;
    } else if (std::string(argv[i]) == "-find_army_terrain") {
      GLOBAL_find_army_terrain = true;
    } else if (std::string(argv[i]) == "-find_army_soldiers") {
      GLOBAL_find_army_soldiers = true;
    } else {
      std::cerr << "UNKNOWN COMMAND LINE ARGUMENT: " << argv[i] << std::endl;
    }
  }

  assert (GLOBAL_input_file != "");  
  assert (GLOBAL_output_file != "");
  assert (GLOBAL_find_army_terrain || GLOBAL_find_army_soldiers);

  helper();
  return 0;
}

void helper () {

  Image<sRGB> in_image(GLOBAL_input_file.c_str()); //argv[1]);

  CalibratedCamera camera;
  camera.loadCalibration("camera_calibration.dat");
  double a0 = camera.getA0();
  double u0 = camera.getCx();
  double v0 = camera.getCy();
  double k1 = camera.getK1();
  double k2 = camera.getK2();

  Image<sRGB> input = CorrectRadialDistortion(in_image, 
					      u0, v0, k1, k2, a0);

  input.write("debug_labels/undistorted.ppm");

  Table table(&camera, "table_cal.dat");
  // store min, max active column for each row
  Image<int> bounds = table.getBounds();

  // BARB ADDED 4 DEBUGGING
#if 0
  bounds = Image<int>(input.getRows(),2);
  for (int r = 0; r < input.getRows(); r++) {
    bounds(r,0) = 0;
    bounds(r,1) = input.getCols()-1;
  }
#endif

  Image<byte> table_mask = table.getMaskImage();

  Scene scene;
  scene.input_image = &input;
  scene.camera = camera;
  scene.table_mask = &table_mask;
  scene.table = &table;
  pscene = &scene;

  Image<byte>* components = find_colors(input, bounds,2,2); //0,0); //0,0); //1,1); //2,2);//1,1);

  /*
  // !!! hack for bottom few lines of image which get scrambled
  //  by camera or driver or whatever
  for (int row = components->getRows()-10; row<components->getRows(); row++){
    for (int col=0; col<components->getCols(); col++){
      (*components)(row,col) = 0;
    }
  }
  */

  components->write("debug_labels/components.ppm");

  scene.component_image = components;

  /*
  Image<byte> red_comps(components.getRows(), components.getCols());
  Image<byte> green_comps(components.getRows(), components.getCols());
  Image<byte> blue_comps(components.getRows(), components.getCols());
  bool no_label;

  for (int row=0; row<components.getRows(); row++){
    for (int col=0; col<components.getCols(); col++){
      // assign values red green and blue components as appropriate, making
      // sure that no pixel is assigned to both
      // (optimize later)
      red_comps(row, col) = components(row,col) & 1;
      no_label = (red_comps(row,col) == 0);
      green_comps(row, col) = no_label * (components(row,col) & 2);
      no_label = no_label && (green_comps(row,col) == 0);
      blue_comps(row, col) = no_label * (components(row,col) & 4);
    }
  }  
  */

  int numlabels; //, num_redlabels, num_greenlabels, num_bluelabels;
  //Image<int> labels = four_connected(components, bounds, numlabels);
  //Image<int> red_labels = ConnectedComponents(red_comps, num_redlabels);
  //Image<int> green_labels = ConnectedComponents(green_comps, num_greenlabels);
  //Image<int> blue_labels = ConnectedComponents(blue_comps, num_bluelabels);
  //numlabels = num_redlabels + num_greenlabels + num_bluelabels;
  Image<int> labels = ConnectedComponents((*components), numlabels);


  std::cout << "NUM LABELS " << numlabels;

  

  /*
  int green_label_offset = num_redlabels;
  int blue_label_offset = num_redlabels + num_greenlabels;

  // accumulate all lables into one image, altering them to make them mutually exclusive
  Image<int> labels = red_labels;
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      // safe to do this since it is ensured that no two labels share the same pixel
      labels(row,col) += (green_labels(row,col) > 0) * 
	(green_labels(row,col) + green_label_offset);
      labels(row,col) += (blue_labels(row,col) > 0) * 
	(blue_labels(row,col) + blue_label_offset);
    }
  }
  */
  srand48(37);

#ifdef DEBUG_LABELS_IMAGE
  
  std::map<int,sRGB> label_colors;

  Image<sRGB> label_debug(labels.getRows(), labels.getCols());
  //Image<byte> label_debugr(labels.getRows(), labels.getCols());
  //Image<byte> label_debugg(labels.getRows(), labels.getCols());
  //Image<byte> label_debugb(labels.getRows(), labels.getCols());
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){

      sRGB color(0,0,0);
      int which_label = labels(row,col);
      
      if (which_label > 0) {
	//std::cout << row << " " << col << " " << labels(row,col) << std::endl;
	std::map<int,sRGB>::iterator itr = label_colors.find(which_label);
	if (itr == label_colors.end()) {
	  color = sRGB(255*drand48(),255*drand48(),255*drand48());
	  label_colors[which_label] = color;
	} else {
	  color = itr->second;
	}
      }

      label_debug(row, col) = color; //255*(labels(row, col) > 0);
      //label_debug(row, col) = 1;
      //label_debugr(row, col) = 255*(red_labels(row, col) > 0);
      //label_debugg(row, col) = 255*(green_labels(row, col) > 0);
      //label_debugb(row, col) = 255*(blue_labels(row, col) > 0);
    }
  }  
  if (WRITE_DEBUG_IMAGES){
    label_debug.write("debug_labels/debug_labels.ppm");
    //label_debugr.write("debug_labels/red_labels.pgm");
    //label_debugg.write("debug_labels/green_labels.pgm");
    //label_debugb.write("debug_labels/blue_labels.pgm");
  }
#endif

  scene.objects.resize(numlabels);
  for (int i=0; i<numlabels; i++){
    scene.objects[i].scene = &scene;
  }

  Image<sRGB> out(labels.getRows(), labels.getCols());
  for (int row=0; row<out.getRows(); row++){
    for (int col=0; col<out.getCols(); col++){
      int r = ((*components)(row, col) & 1) >> 0;
      int g = ((*components)(row, col) & 2) >> 1;
      int b = ((*components)(row, col) & 4) >> 2;
      out(row, col) = sRGB(r*64, g*64, b*64);
    }
  }

  // process rest of rows
  for (int row=1; row<bounds.getRows()-1; row++){
    int *min_col = new int[numlabels];
    int *max_col = new int[numlabels];
    for (int i=0; i<numlabels; i++){
      min_col[i] = bounds(row,1);
      max_col[i] = bounds(row,0);
    }
    for (int col=bounds(row,0)+1; col<=bounds(row,1)-1; col++){
      if (labels(row,col)){
        assert(labels(row,col) < numlabels);
        assert(labels(row,col) > 0);
	// add pixel data to object's color histogram
	scene.objects[labels(row,col)].ColorHist[(*components)(row,col)&7]++;
        
	// update object's mass and collection of points
	scene.objects[labels(row,col)].
	  points.push_back(point(row,col));
	scene.objects[labels(row,col)].mass++;
	if (col < min_col[labels(row,col)]){
	  min_col[labels(row,col)] = col;
	}
	if (col > max_col[labels(row,col)]){
	  max_col[labels(row,col)] = col;
	}
	
	// edge detection : edge points are pixels that border the edge of the
	// object (4-connected not 8-connected)
	if ((labels(row-1,col) == 0 || 
             labels(row,col-1) == 0 ||
	     labels(row+1,col) == 0 ||
             labels(row,col+1) == 0)){
	  scene.objects[labels(row,col)].
	    edge_points.push_back(point(row,col));
	  out(row,col) = sRGB(63, 63, 63);
	}
      }
    }
    for (int i=0; i<numlabels; i++){
      if (max_col[i] >= min_col[i]){
	double r = sqrt((row-table.getCenterRow())*(row-table.getCenterRow())+
			(min_col[i] - table.getCenterCol()) *
			(min_col[i] - table.getCenterCol()));
	if (r > scene.objects[i].max_radius){
	  scene.objects[i].max_radius = r;
	}
	r = sqrt((row-table.getCenterRow())*(row-table.getCenterRow())+
		 (max_col[i] - table.getCenterCol()) *
		 (max_col[i] - table.getCenterCol()));
	if (r > scene.objects[i].max_radius){
	  scene.objects[i].max_radius = r;
	}

	// add to object's area : note that this may differ from mass, since
	// mass does not count holes in the middle of the object component
	scene.objects[i].area += (max_col[i] - min_col[i] + 1);
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
    double my_area = scene.objects[i].area;
    if ((scene.objects[i].area > max_area_thresh || 
	 scene.objects[i].aspect > min_aspect_ratio) &&
	scene.objects[i].max_radius > (table.getRadiusPixels()-border_pixels)){
      //      fprintf(stderr, "Object crossing table boundary; walls not located\n");
      // return -1;
    }
    //    fprintf(stderr, "area = %d\n", scene.objects[i].area);
    scene.objects[i].compute_gradients(labels, i);
    scene.objects[i].classify();
  }

  //scene.assign_tokens();
  scene.draw(out);

  if (WRITE_DEBUG_IMAGES) out.write("debug_labels/terrain_labels.ppm");

  scene.write(GLOBAL_output_file.c_str()); //argv[2]);

  table_mask.write("debug_labels/table_mask.pgm");

  delete components;

  //  return 0;
}
