#include "../common/Image.h"
#include "../common/ImageOps.h"
#include <vector>
#include <algorithm>
#include "../common/Vector3.h"
#include "../common/Matrix3.h"
#include <limits>
#include "../common/CalibratedCamera.h"

#define GIGE_CAMERA

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

Image<byte> find_colors(Image<sRGB> &in, Image<int> &bounds){
  Image<byte> ids(in.getRows(), in.getCols());
  Image<sRGB> colors(in.getRows(), in.getCols());
  Image<sRGB> enh_colors(in.getRows(), in.getCols());
  for (int row=0; row<bounds.getRows(); row++){
    for (int col=bounds(row,0); col<=bounds(row,1); col++){
#ifdef FIREWIRE_CAMERA
      v3d v = convertVector3(in(row, col), 0.)/255.;
      double d = max((v.mid()-v.min())/(v.mid()+v.min()),
		     (1. - 4.*(v.mid()*(1.-v.mid())))) *
	(v.max()-v.min())/(v.max()+v.min());
      v3d u = (v - v.min()) / v.max();


      double d_thresh = 0.032;
      if (v.max() == v.r()){
	d_thresh = 0.05;
      }
      if (v.max() != v.r() && fabs(v.g()-v.b())/(v.g()+v.b()) < 0.25){
	d_thresh = 0.01;
      }

      v = u * (d> d_thresh);
      v.normalize();
      colors(row,col) = convertVector3(255*v, byte(0));
      //colors(row,col) = convertVector3(255.*u, byte(0));
      
      int id = 0;
      int r, g, b;
      int red = 0;
      int green = 0;
      int blue = 0;

      double b_thresh = 0.625;
      if (v.b() > b_thresh){
	id |= 4;
	blue = 255;
      }

      double g_thresh = 0.375;
      if (v.g() > g_thresh){
	id |= 2;
	green = 255;
      }

      double r_thresh = 0.75;
      if (blue){
	r_thresh = 0.05;
      }
      if (v.r() > r_thresh){
	id |= 1;
	red = 255;
      }
#endif // #ifdef FIREWIRE_CAMERA

#ifdef GIGE_CAMERA
      v3d v = v3d(in(row, col))/255.;

      double d = min(1., max(v.max()-v.mid(),v.mid()-v.min())/(v.min()*v.min()));
      double db = 1-v.min();
      d = d*db;

      v3d u = (v - v3d(v.min())) / v3d(v.max());

      double d_thresh = 0.5;
#if 0
      if (v.min() == v.g() && (v.max()-v.min()) > 0.15*v.max()){
	d_thresh = 0.15;
      }
      if (v.min() == v.r() && (v.max()-v.min()) > 0.15*v.max()){
	d_thresh = 0.25;
      }
#endif
      v = u * double(d> d_thresh);
      v.normalize();
      //colors(row,col) = convertVector3(v3d(255*(d>d_thresh)), byte(0));
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

      double r_thresh = 0.125;
      if (v.r() > r_thresh){
	id |= 1;
	red = 255;
      }

#endif // #ifdef GIGE_CAMERA

      ids(row, col) = id;
      enh_colors(row, col) = sRGB(red, green, blue);
    }
  }
  if (WRITE_DEBUG_IMAGES) colors.write("colors.ppm");
  if (WRITE_DEBUG_IMAGES) enh_colors.write("enh_colors.ppm");
  return ids;
}

template <typename Pixel>
class PixelSetter: public ImagePointFunctor<Pixel>
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

class Quad {
public:
  point corner_points[4];
  line edges[4];
  int edge_side[4];
  bool valid;
  v3d world_points[4];

  Quad(){}
  Quad(std::vector<point> &edge_points, 
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
    line l4 = ransac_line(remaining_points, points);
        
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


class Object;

class Skylight {
public:
  Skylight(Quad &quad){
    this->quad = quad;
  }
  void draw(Image<sRGB> &image){
    quad.draw(image);
  }
  void project(CalibratedCamera &camera){
    quad.project(camera);
  }
  void write(FILE *fp){
    fprintf(fp, "skylight ");
    quad.write(fp);
    fprintf(fp, "\n");
  }
private:
  Object *object;
  Quad quad;
};

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
#ifdef FIREWIRE_CAMERA
    if (thickness < 8. || thickness > 19.){
#endif
#ifdef GIGE_CAMERA
    if (thickness < 5. || thickness > 19.){
#endif

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
    const int MIN_COLOR_RUN_LENGTH = 5;
    int current_color = dominant_color[0];
    int run_start = 0;
    for (int i=1; i<nhist; i++){
      if (dominant_color[i] != current_color){
        int run_length = i - run_start;
        bool valid_color = false;
        if (run_length > MIN_COLOR_RUN_LENGTH){
          const char *window_keyword;
          switch (current_color){
          case CYAN_IDX:
            window_keyword = CYAN_KEYWORD;
            valid_color = true;
            break;
          case MAGENTA_IDX:
            window_keyword = MAGENTA_KEYWORD;
            valid_color = true;
            break;
          case YELLOW_IDX:
            window_keyword = YELLOW_KEYWORD;
            valid_color = true;
            break;
          default:
            break;
          }
          if (valid_color){
            // found a window
            
            // find cap lines
            line cap1(centerline.b, -centerline.a, double(run_start+min_idx));
            line cap2(centerline.b, -centerline.a, double(i-1+min_idx));
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

	  // merge windows
	  previous_walls[i].windows.insert(previous_walls[i].windows.end(),
					   windows.begin(), windows.end());
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
    for (unsigned i=0; i<windows.size(); i++){
      windows[i].draw(image);
    }
  }

  void project(CalibratedCamera &camera){
    rectangle.project_and_extend(camera, y, wall_tips);
    for (unsigned i=0; i<windows.size(); i++){
      windows[i].project(camera, y);
    }
  }

  void write(FILE *fp){
    fprintf(fp, "wall   ");
    rectangle.write(fp);
    fprintf(fp, "%+8.4f %d\n", y, color_idx);
    fprintf(fp, "num_windows %d\n", (int)windows.size());
    for (unsigned i=0; i<windows.size(); i++){
      windows[i].write(fp);
    }
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
  std::vector<Window> windows;
};

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

class ColorToken {
public:
  ColorToken (CurvedWall &curvedwall, Image<sRGB> &image,
              Image<byte> &component_image, std::vector<point> &points){

    valid = true;

    // if radius = nan, this is not a colortoken
    if (curvedwall.centercircle.get_r() != curvedwall.centercircle.get_r()){
      valid = false;
      return;
    } 
    printf("inner radius =%f\n", curvedwall.centercircle.get_r());
    // too small or too big to be a color token
    if (curvedwall.centercircle.get_r() < 17.0 ||
        curvedwall.centercircle.get_r() > 30){
      valid = false;
      return;
    }

    // average the color of the inside patch
    center = point(curvedwall.centercircle.get_yc(),
                   curvedwall.centercircle.get_xc());

    // color patch radius
    const double r = 3.5;
    double r2 = r*r;
    // get the average "white enough" pixels from an annulus surrounding the
    // color swatch
    const double inner_r = 9.;
    const double outer_r = 12.;
    double inner_r2 = inner_r * inner_r;
    double outer_r2 = outer_r * outer_r;
    v3d sum(0., 0., 0.);
    int count = 0;
    v3d white_sum(0., 0., 0.);
    int white_count = 0;
    for (int row = int(center.row-outer_r-1); 
         row <= int(center.row+outer_r+1); row++){
      for (int col = int(center.col-outer_r-1); 
           col <= int(center.col+outer_r+1); col++){
        double rad2 = ((row-center.row)*(row-center.row) +
                       (col-center.col)*(col-center.col));
        v3d v = v3d(image(row, col));
        if (rad2 < r2){
          sum += v;
          count++;
        }
        if (rad2 > inner_r2 && rad2 < outer_r2){
          double score = (v.max() - v.min())/(v.min() + v.max());
          if (score < 0.2){
            white_sum += v;
            white_count++;
          }
        }
      }
    }
    white_sum = white_sum/(255.*white_count);
    
    sum = sum/double(count);
    sum = sum/white_sum;
    sum = min(sum, v3d(255.));
    sum = max(sum, v3d(0.));
    color = sRGB(sum);

    //#define QUANITZED_COLORS
#ifdef QUANITZED_COLORS
    v3d v = sum/255.;
    v = (v - v.min()) / v.max();
    v.normalize();
    
    int id = 0;
    int red = 0;
    int green = 0;
    int blue = 0;
    
    double b_thresh = 0.625;
    if (v.b() > b_thresh){
      id |= 4;
      blue = 255;
    }
    
    double g_thresh = 0.375;
    if (v.g() > g_thresh){
      id |= 2;
      green = 255;
    }
    
    double r_thresh = 0.75;
    if (blue){
      r_thresh = 0.05;
    }
    if (v.r() > r_thresh){
      id |= 1;
      red = 255;
    }

    color = sRGB(red, green, blue);

#if 0
    switch(id){
    case 0:
      break;
    case 1:
      color = sRGB(0.8*255, 0.2*255, 0.2*255);
      break;
    case 2:
      color = sRGB(0.2*255, 0.8*255, 0.2*255);
      break;
    case 3:
      break;
    case 4:
      color = sRGB(0.2*255, 0.2*255, 0.8*255);
      break;
    case 5:
      break;
    case 6:
      break;
    case 7:
      break;
    }
#endif

#endif

    // determine what type of color token this is
    sample_radius = curvedwall.centercircle.get_r();
    Histogram hist(8);
    for (unsigned i=0; i<points.size(); i++){
      int row = int(points[i].row);
      int col = int(points[i].col);
      hist(component_image(row,col))++;
    }
    switch (hist.peak_idx()){
    case SPECIFIC_WALL_TOKEN_IDX:
      type = SPECIFIC_WALL;
      break;
    case GLOBAL_WALL_TOKEN_IDX:
      type = GLOBAL_WALL;
      break;
    case FLOOR_TOKEN_IDX:
    case FLOOR_TOKEN_IDX2:
      type = FLOOR;
      break;
    case CEILING_TOKEN_IDX:
      type = CEILING;
      break;
    default:
      fprintf(stderr, "unknown token color %d\n", hist.peak_idx());
      valid = false;
    }
  }

  void draw(Image<sRGB> &image){
    const double r = 3.5;
    double r2 = r*r;
    const double inner_r = 9.;
    const double outer_r = 12.;
    double inner_r2 = inner_r * inner_r;
    double outer_r2 = outer_r * outer_r;
    for (int row = int(center.row-outer_r-1); 
         row <= int(center.row+outer_r+1); row++){
      for (int col = int(center.col-outer_r-1);
           col <= int(center.col+outer_r+1); col++){
        double rad2 = ((row-center.row)*(row-center.row) +
                       (col-center.col)*(col-center.col));
        if (rad2 < r2 || (rad2 > inner_r2 && rad2 < outer_r2)){
          image(row, col) = sRGB(128, 128, 128);
        }
      }
    }

    for (int i=0;i<200; i++){
      double theta = 2*pi*i/200.;
      int row = int(center.row + sample_radius*sin(theta));
      int col = int(center.col + sample_radius*cos(theta));
      image(row, col) = sRGB(192, 192, 192);
    }

  }

  enum {GLOBAL_WALL, SPECIFIC_WALL, FLOOR, CEILING} type;
  sRGB color;
  point center;
  double sample_radius;
  bool valid;
};

int dominant_color_idx(std::vector<point> &points, Image<byte> &image){
  Histogram hist(8);
  for (unsigned i=0; i<points.size(); i++){
    hist(image(int(points[i].row), int(points[i].col)))++;
  }
  return hist.peak_idx();
}


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


class Scene {
public:
  std::vector<Object> objects;
  std::vector<Wall> walls;
  std::vector<Skylight> skylights;
  std::vector<CurvedWall> curvedwalls;
  std::vector<ColorToken> colortokens;
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

  void assign_tokens(){
    ceiling_color = sRGB(255, 255, 255);
    floor_color = sRGB(255, 255, 255);
    sRGB default_wall_color = sRGB(255, 255, 255);
    wall_materials.push_back(default_wall_color);

    for (unsigned i=0; i<walls.size(); i++){
      walls[i].color_idx = 0;
    }
    for (unsigned i=0; i<curvedwalls.size(); i++){
      curvedwalls[i].color_idx = 0;
    }

    for (unsigned i=0; i<colortokens.size(); i++){
      switch (colortokens[i].type){
      case ColorToken::CEILING:
        ceiling_color = colortokens[i].color;
        break;
      case ColorToken::FLOOR:
        floor_color = colortokens[i].color;
        break;
      case ColorToken::GLOBAL_WALL:
        wall_materials[0] = colortokens[i].color;
        break;
      case ColorToken::SPECIFIC_WALL:
        {
          wall_materials.push_back(colortokens[i].color);

          // find closest wall
          unsigned closest_wall = 0;
          double wall_dist = std::numeric_limits<double>::max();
          for (unsigned j=0; j<walls.size(); j++){
            double dist = walls[j].point_distance(colortokens[i].center);
            if (dist < wall_dist){
              closest_wall = j;
              wall_dist = dist;
            }
          }

          // find closest curvedwall
          unsigned closest_curvedwall = 0;
          double curvedwall_dist = std::numeric_limits<double>::max();
          for (unsigned j=0; j<curvedwalls.size(); j++){
            double dist = curvedwalls[j].point_distance(colortokens[i].center);
            if (dist <curvedwall_dist){
              closest_curvedwall = j;
              curvedwall_dist = dist;
            }
          }
          if (wall_dist < curvedwall_dist && closest_wall < walls.size()){
            walls[closest_wall].color_idx = wall_materials.size()-1;
          } else if (closest_curvedwall < curvedwalls.size()){
            curvedwalls[closest_curvedwall].color_idx = wall_materials.size()-1;
          }
        }
        break;
      default:
        break;
      }
    }

  }

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

    //#warning hack: align coordinate system with camera frame (correct solution)
    //angle_adjust = 0.;
    angle_adjust -= pi/2.;

    //printf("north = %f\n", north);
    
    FILE *fp = fopen(filename, "wt");
    if (NULL == fp){
      fprintf(stderr, "unable to open %s\n", filename);
      exit(-1);
    }

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

    fprintf(fp, "num_curved_walls %d\n", (int)curvedwalls.size());
    for (unsigned i=0; i<curvedwalls.size(); i++){
      curvedwalls[i].project(camera, curved_wall_tips, angle_adjust);
      curvedwalls[i].write(fp);
    }

    fprintf(fp, "num_skylights %d\n", (int)skylights.size());
    for (unsigned i=0; i<skylights.size(); i++){
      skylights[i].project(camera);
      skylights[i].write(fp);
    }

    fprintf(fp, "num_columns %d\n", (int)columns.size());
    for (unsigned i=0; i<columns.size(); i++){
      columns[i].write(fp);
    }


    fclose(fp);
  }

  void draw(Image<sRGB> &image){
    for (unsigned i=0; i<walls.size(); i++){
      walls[i].draw(image);
    }
    for (unsigned i=0; i<curvedwalls.size(); i++){
      curvedwalls[i].draw(image);
    }
    for (unsigned i=0; i<skylights.size(); i++){
      skylights[i].draw(image);
    }
    for (unsigned i=0; i<colortokens.size(); i++){
      colortokens[i].draw(image);
    }
    for (unsigned i=0; i<arrows.size(); i++){
      arrows[i].draw(image);
    }
    for (unsigned i=0; i<columns.size(); i++){
      columns[i].draw(image);
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

class Object {
public:
  Object(){
    mass = 0;
    area = 0;
    max_radius = 0.;
    for (int i=0; i<8; i++) ColorHist[i] = 0;
  }

  //private:
  std::vector<point> points;
  std::vector<point> edge_points;
  point center; // of mass
  int mass;
  int area;
  double aspect;
  double max_radius; // from table center, in pixels (image space)
  Scene *scene;
  int ColorHist[8];

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

    //    if (line_error < circle_error){
    if (line_consistancy > circle_consistancy){
      // check for arrow here
      Arrow arrow(centerline, edge_points, 
                  points, *scene->component_image, *this);
      if (arrow.valid){
        scene->arrows.push_back(arrow);
	FillObjectInImage(*scene->table_mask, points, false);
        scene->north = arrow.north;
      } else {
        // it's a wall
        Wall wall(edge_points, centerline, points, *scene->component_image,
		  scene->walls);
        if (wall.valid){
          scene->walls.push_back(wall);
	  FillObjectInImage(*scene->table_mask, points, false);
        }
      }
    } else {
      // it's a curved wall, color token, or skylight

#ifdef FIREWIRE_CAMERA
      // check color - cyan == skylight
      int color = dominant_color_idx(points, *scene->component_image);
      if (CYAN_IDX == color){
        Quad quad(edge_points, points);
        Skylight skylight(quad);
        scene->skylights.push_back(skylight);
	FillObjectInImage(*scene->table_mask, points, true);
      } else {
        // check for valid curved wall to differentiate from color tokens
        CurvedWall curvedwall(edge_points, points, *scene->component_image, 
                              centercircle);
        if (curvedwall.valid){
          // it's a curved wall
          scene->curvedwalls.push_back(curvedwall);
	  FillObjectInImage(*scene->table_mask, points, false);
        } else {
          // it's a color token
          ColorToken token(curvedwall, *scene->input_image,
                           *scene->component_image, points);
          if (token.valid){
            scene->colortokens.push_back(token);
	    FillObjectInImage(*scene->table_mask, points, true);
          }
        }
      }
#endif // #ifdef FIREWIRE_CAMERA

#ifdef GIGE_CAMERA
      // check color - cyan or blue could be skylight
      int color = dominant_color_idx(points, *scene->component_image);
      bool possible_token = false;

      // could be curved wall, color token or skylight
      if (CYAN_IDX == color || BLUE_IDX == color){
	possible_token = true;
	Quad quad(edge_points, points);
	double min_token_edge_length = 36.;
	double max_token_edge_length = 50.;
	for (int i=0; i<4;i++){
	  //printf("%f ", quad.edge_length(i));
	  if (quad.edge_length(i) < min_token_edge_length ||
	      quad.edge_length(i) > max_token_edge_length){
	    possible_token = false;
	  }
	}
	  printf("\n");

	// check for valid curved wall to differentiate from color tokens
	CurvedWall curvedwall(edge_points, points, *scene->component_image, 
			      centercircle);
	if (curvedwall.valid){
	  // it's a curved wall
	  scene->curvedwalls.push_back(curvedwall);
	  FillObjectInImage(*scene->table_mask, points, false);
	} else {
	  if (possible_token){
	    // it's a color token
	    ColorToken token(curvedwall, *scene->input_image,
			     *scene->component_image, points);
	    if (token.valid){
	      scene->colortokens.push_back(token);
	      FillObjectInImage(*scene->table_mask, points, true);
	    }
	  } else {
	    // skylight or column
	    Column column(*scene->component_image, points,
			  edge_points, scene->camera);

	    if (column.valid){
	      scene->columns.push_back(column);
	    } else {
	      Skylight skylight(quad);
	      scene->skylights.push_back(skylight);
	    }
	    FillObjectInImage(*scene->table_mask, points, true);
	  }
	}

      } else {
	// its a token or a curved wall or a red column

	// check for valid curved wall to differentiate from color tokens
	CurvedWall curvedwall(edge_points, points, *scene->component_image, 
			      centercircle);
	if (curvedwall.valid){
	  // it's a curved wall
	  scene->curvedwalls.push_back(curvedwall);
	  FillObjectInImage(*scene->table_mask, points, false);
	} else {
	  // it's a color token
	  ColorToken token(curvedwall, *scene->input_image,
			   *scene->component_image, points);
	  if (token.valid){
	    scene->colortokens.push_back(token);
	    FillObjectInImage(*scene->table_mask, points, true);
	  } else {
	    // might be a column
	    Column column(*scene->component_image, points,
			  edge_points, scene->camera);
            
	    if (column.valid){
	      scene->columns.push_back(column);
              FillObjectInImage(*scene->table_mask, points, true);
            }
          }
	}
      }
#endif // #ifdef GIGE_CAMERA

    }
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
      pixel_t tmp = in.bilinear(v, u);
      pixel_t tmp2;
      tmp2 = tmp;
      out(row, col) = tmp2;
    }
  }
  return(out);      
}

#define CAMERA_CALIBRATION_DAT "../../state/table_top_calibration/camera_calibration.dat"
#define TABLE_CAL_DAT "../../state/table_top_calibration/table_cal.dat"

int main(int argc, char **argv){
  Image<sRGB> in_image(argv[1]);

  CalibratedCamera camera;
  camera.loadCalibration(CAMERA_CALIBRATION_DAT); //"camera_calibration.dat");
  double a0 = camera.getA0();
  double u0 = camera.getCx();
  double v0 = camera.getCy();
  double k1 = camera.getK1();
  double k2 = camera.getK2();

  Image<sRGB> input = CorrectRadialDistortion(in_image, 
					      u0, v0, k1, k2, a0);

  input.write("undistorted.ppm");

  Table table(&camera, TABLE_CAL_DAT); //"table_cal.dat");
  // store min, max active column for each row
  Image<int> bounds = table.getBounds();
  Image<byte> table_mask = table.getMaskImage();

  Scene scene;
  scene.input_image = &input;
  scene.camera = camera;
  scene.table_mask = &table_mask;
  scene.table = &table;
  pscene = &scene;

  Image<byte> components = find_colors(input, bounds);

  // !!! hack for bottom few lines of image which get scrambled
  //  by camera or driver or whatever
  for (int row = components.getRows()-10; row<components.getRows(); row++){
    for (int col=0; col<components.getCols(); col++){
      components(row,col) = 0;
    }
  }
  scene.component_image = &components;

  int numlabels;
  //Image<int> labels = four_connected(components, bounds, numlabels);
  Image<int> labels = ConnectedComponents(components, numlabels);

  //#define DEBUG_LABELS_IMAGE
#ifdef DEBUG_LABELS_IMAGE
  Image<byte> label_debug(labels.getRows(), labels.getCols());
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      label_debug(row, col) = 255*(labels(row, col) > 0);
    }
  }  
  if (WRITE_DEBUG_IMAGES) label_debug.write("debug_labels.pgm");
#endif

  scene.objects.resize(numlabels);
  for (int i=0; i<numlabels; i++){
    scene.objects[i].scene = &scene;
  }

  Image<sRGB> out(labels.getRows(), labels.getCols());
  for (int row=0; row<out.getRows(); row++){
    for (int col=0; col<out.getCols(); col++){
      int r = (components(row, col) & 1) >> 0;
      int g = (components(row, col) & 2) >> 1;
      int b = (components(row, col) & 4) >> 2;
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
	scene.objects[labels(row,col)].ColorHist[components(row,col)&7]++;
        
	scene.objects[labels(row,col)].
	  points.push_back(point(row,col));
	scene.objects[labels(row,col)].mass++;
	if (col < min_col[labels(row,col)]){
	  min_col[labels(row,col)] = col;
	}
	if (col > max_col[labels(row,col)]){
	  max_col[labels(row,col)] = col;
	}
	
	// edge detection
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
    if ((scene.objects[i].area > max_area_thresh || 
	 scene.objects[i].aspect > min_aspect_ratio) &&
	scene.objects[i].max_radius > (table.getRadiusPixels()-border_pixels)){
      //      fprintf(stderr, "Object crossing table boundary; walls not located\n");
      // return -1;
    }
    //    fprintf(stderr, "area = %d\n", scene.objects[i].area);
    scene.objects[i].classify();
  }

  scene.assign_tokens();
  scene.draw(out);

  if (WRITE_DEBUG_IMAGES) out.write("labels.ppm");

  scene.write(argv[2]);

  table_mask.write("table_mask.pgm");

  return 0;
}
