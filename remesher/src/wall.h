#ifndef _WALL_H_
#define _WALL_H_

#include "argparser.h"


enum FURNITURE_TYPE { FURNITURE_UNDEFINED, FURNITURE_BED, FURNITURE_DESK, FURNITURE_WARDROBE };

#define SKYLIGHT_FRAME_WIDTH (0.5 * INCH_IN_METERS)
//static double WHEEL_HEIGHT;

// NOTE:  These are table top dimensions, which are 1/12 scale!

#define GREEN_HEIGHT     (5 * INCH_IN_METERS)
#define BLUE_HEIGHT      (8 * INCH_IN_METERS)
#define RED_HEIGHT       (10 * INCH_IN_METERS)

#define BED_HEIGHT       (22 / 12.0 * INCH_IN_METERS)
#define DESK_HEIGHT      (29 / 12.0 * INCH_IN_METERS)
#define WARDROBE_HEIGHT  (78 / 12.0 * INCH_IN_METERS)

#define WINDOW_FROM_TOP  (6 / 12.0 * INCH_IN_METERS)


extern ArgParser *my_ugly_args_hack;

bool OnSegment(const Vec3f &pt, const Vec3f &endpt1, const Vec3f &endpt2);

inline void VecVertex(Vec3f v) { glVertex3f(v.x(),v.y(),v.z()); }

// ==================================================================================
// ==================================================================================

class ConvexQuad {
 public:
  ConvexQuad(Vec3f _a,Vec3f _b,Vec3f _c,Vec3f _d) {
    a=_a.x();b=_a.z();c=_b.x();d=_b.z();e=_c.x();f=_c.z();g=_d.x();h=_d.z(); 
    computeNormals();
  } 
  ConvexQuad(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h) {
    a=_a;b=_b;c=_c;d=_d;e=_e;f=_f;g=_g;h=_h; 
    computeNormals();
  }
  int numVerts() const { return 4; }
  const Vec3f& operator[](int i) const {
    assert (i >= 0 && i < 4);
    return verts[i];
  }
  Vec3f verthelper(int i) const {
    assert (i >= 0 && i < 4);
    switch (i) {
    case 0: return Vec3f(a,0,b);
    case 1: return Vec3f(c,0,d);
    case 2: return Vec3f(e,0,f);
    default: return Vec3f(g,0,h);
    }
  }
  void setVert(int i, Vec3f v) {
    //std::cout << "MOD CQ vert" << std::endl;
    assert (i >= 0 && i < 4);
    assert (fabs(v.y()-0) < 0.0000000001);
    switch (i) {
    case 0: { a=v.x(); b = v.z(); break; }
    case 1: { c=v.x(); d = v.z(); break; }
    case 2: { e=v.x(); f = v.z(); break; }
    default:{ g=v.x(); h = v.z(); break; }
    }
    computeNormals();
  }

  bool PointInside(const Vec3f &pt) const;
  Vec3f getCentroid() const {
    Vec3f sum = (*this)[0];
    for (int i = 1; i < 4; i++) { sum += (*this)[i]; }
    return sum /= 4.0;
  }
  bool invert() {
    // should have clockwise winding when viewed from correct side
    Vec3f centroid = this->getCentroid();
    Vec3f diff1 = (*this)[0]-centroid;
    Vec3f diff2 = (*this)[1]-centroid;
    diff1.Normalize();
    diff2.Normalize();
    Vec3f cross;
    Vec3f::Cross3(cross,diff1,diff2);
    if (cross.y() > 0) { 
      double tmp;
      tmp = a; a=c; c=tmp;
      tmp = b; b=d; d=tmp;
      tmp = e; e=g; g=tmp;
      tmp = f; f=h; h=tmp;    
      std::cout << " INVERTED! " << std::endl;
      computeNormals();
      return true;
    }
    return false;
  }
  void flip() { 
    double tmp;
    tmp = a; a=e; e=tmp;
    tmp = b; b=f; f=tmp;
    tmp = c; c=g; g=tmp;
    tmp = d; d=h; h=tmp;    
    computeNormals();
  }
  void rotate90() {
    double tmp;
    tmp = a; a=g; g=e; e=c; c=tmp;
    tmp = b; b=h; h=f; f=d; d=tmp;
    computeNormals();
  }
  void Print(const std::string &s = "") const {
    std::cout << "CONVEX QUAD " << s << std::endl;
    std::cout << "this " << this << std::endl;
    for (int i = 0; i < 4; i++) {
      (*this)[i].Print("  v");
    }
    std::cout << "  length = " << Length() << std::endl;
    std::cout << "  width = " << Width() << std::endl;
  }
  const Vec3f& getNormal(int i) const {
    assert (i >= 0 && i < 4);
    return edge_normals[i];
  }
  double Length() const {
    return DistanceBetweenTwoPoints(0.5*(verthelper(0)+verthelper(3)),
				    0.5*(verthelper(1)+verthelper(2)));				    
  }
  double Width() const {
    return DistanceBetweenTwoPoints(0.5*(verthelper(0)+verthelper(1)),
				    0.5*(verthelper(2)+verthelper(3)));				    
  }
  void computeNormals() {
    for (int i = 0; i < 4; i++) {
      verts[i] = verthelper(i);
    }
    for (int i = 0; i < 4; i++) {      
      Vec3f a = (*this)[i];
      Vec3f b = (*this)[(i+1)%4];
      Vec3f dir = b-a; dir.Normalize();
      Vec3f up = Vec3f(0,1,0);
      Vec3f::Cross3(edge_normals[i],up,dir); 
      assert(fabs(edge_normals[i].Length()-1) < 0.001);
    }
  }
  void set(const ConvexQuad &q) {
    //std::cout << "MOD CQ" << std::endl;
    a = q.a;
    b = q.b;
    c = q.c;
    d = q.d;
    e = q.e;
    f = q.f;
    g = q.g;
    h = q.h; 
    computeNormals();
  }
 protected:
  double a,b,c,d,e,f,g,h;
  Vec3f edge_normals[4];
  Vec3f verts[4];
};

// ==================================================================================
// ==================================================================================

class Window : public ConvexQuad {
 public:
 Window(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h, std::string _win_type, int _which_quad) 
    : ConvexQuad(_g,_h,_a,_b,_c,_d,_e,_f) {
    which_quad=_which_quad;
    win_type=_win_type;
    if (win_type == "magenta")
      color = Vec3f(1,0,1);
    else if (win_type == "cyan")
      color = Vec3f(0,1,1);
    else if (win_type == "yellow")
      color =  Vec3f(1,1,0);
    else {
      assert(0);
    }
  }
  void paint(double height) const;
  std::string getType() const { return win_type; }
  int whichQuad() const { return which_quad; }
 private:
  int which_quad;
  std::string win_type;
  Vec3f color;
};

// ==================================================================================
// ==================================================================================

class Skylight : public ConvexQuad {
 public:
 Skylight(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h) 
   : ConvexQuad(_a,_b,_c,_d,_e,_f,_g,_h) {}
  void paint() const;
  Vec3f interior_corner(int i) const {
    assert (i >= 0 && i < 4);
    Vec3f answer = (*this)[i];
    Vec3f v1 = (*this)[(i+1)%4]-(*this)[i];
    Vec3f v2 = (*this)[(i+3)%4]-(*this)[i];
    v1.Normalize();
    v2.Normalize();
    v1 *= SKYLIGHT_FRAME_WIDTH;
    v2 *= SKYLIGHT_FRAME_WIDTH;
    return answer + v1 + v2;
  }
 private:
};



class Furniture : public ConvexQuad {
public:
  Furniture(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h) 
    : ConvexQuad(_a,_b,_c,_d,_e,_f,_g,_h) { furn_type = FURNITURE_UNDEFINED; height = 0; material_index = -1; }
  static Furniture* Desk(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
                         double _height, int _material_index);
  static Furniture* Bed(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
                         double _height, int _material_index);
  static Furniture* Wardrobe(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
                             double _height, int _material_index);
  void paint() const;
  const enum FURNITURE_TYPE& getFurnitureType() { return furn_type; }
  float getHeight() const { 
    if (furn_type == FURNITURE_BED) return BED_HEIGHT;
    if (furn_type == FURNITURE_DESK) return DESK_HEIGHT;
    return WARDROBE_HEIGHT;
  }
private:
  float height;
  int material_index;
  enum FURNITURE_TYPE furn_type;
};

// ==================================================================================
// ==================================================================================

class BasicWall { 
 public:
  BasicWall(double _bottom_edge, double _height, int _material_index);
  static BasicWall* QuadWall(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
			    double _bottom_edge, double _height, int _material_index);
  static BasicWall* LShapedWall(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,double _i,double _j,double _k,double _l, 
				double _bottom_edge, double _height, int _material_index);
  static BasicWall* CurvedWall(double _a,double _b,double _c,double _d,double _e,double _f, 
			       double _bottom_edge, double _height, int _material_index);
  static BasicWall* Column(double x, double z, double radius,
			   double bottom_edge, double height, int material_index);

  static BasicWall* Platform(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
			     double _bottom_edge, double _height, int _material_index);
  static BasicWall* Ramp(double _a,double _b,double _c,double _d,double _e,double _f,double _g,double _h,
			 double _bottom_edge, double _height, int _material_index);

  // ACCESSORS
  void setName(std::string n) { wall_name = n; }
  std::string getName() { return wall_name; }
  void addWindow(const Window &w, int which_quad); // { windows.push_back(w); }
  int numVerts() const { assert (convex_quads.size() >= 1); return (convex_quads.size()+1)*4; } 
  bool IsCurvedWall() const { return is_curved_wall; }
  bool IsColumn() const { return is_column; }
  bool IsRamp() const { return is_ramp; }
  bool IsPlatform() const { return is_platform; }

  int numGoodVerts() const { assert (convex_quads.size() >= 1); return convex_quads.size()*2+2; }
  Vec3f getGoodVert(int i) const {
    assert (i >= 0 && i < numGoodVerts());
    int num_quads = convex_quads.size();
    if (i < num_quads)
      return convex_quads[i][0];
    else if (i == num_quads)
      return convex_quads[num_quads-1][1];
    else if (i == num_quads+1)
      return convex_quads[num_quads-1][2];
    else {
      int tmp = num_quads*2 - (i - 1);
      assert (tmp >=0 && tmp < num_quads);
      return convex_quads[tmp][3];
    }
  }

  Vec3f front_start()  const { //unsigned int n = convex_quads.size(); 
    assert (convex_quads.size()>=1); return convex_quads[0][0]; }
  Vec3f back_start()   const { //unsigned int n = convex_quads.size();
    assert (convex_quads.size()>=1); return convex_quads[0][3]; }
  Vec3f front_end() const { unsigned int n = convex_quads.size(); assert (n>=1); return convex_quads[n-1][1]; }
  Vec3f back_end()  const { unsigned int n = convex_quads.size(); assert (n>=1); return convex_quads[n-1][2]; }

  Vec3f get_start_trajectory() const;
  Vec3f get_end_trajectory() const;

  const Vec3f& getColor() const { return color; }
  void setColor(Vec3f c) { color = c; }

  Vec3f getCentroid() const;
  double getHeight2(int i) const { 
    //std::cout << i << std::endl;
    assert (i >= 0 && i < numGoodVerts()); 
    if (is_ramp && i == 0) return height_b;
    if (is_ramp && i == 1) return height_b;
    return height_a; 
  }
  double getRampHeight(const Vec3f &pos) const;

  double getMaxHeight() const { return height_a; }
  const std::vector<Window>& getWindows() const { return windows; }
  void getWindowDimensions(int i, double &left, double &right);
  double getBottomEdge() const { return bottom_edge; }
  int getMaterialIndex() const { return material_index; }
  bool SegmentIntersectsWall(Vec3f beginning, Vec3f end) const;
  bool SegmentIntersectsWallTop(Vec3f beginning, Vec3f end) const;
  bool PointInside(const Vec3f &v) const;

  const Vec3f& getCircleCenter() const { assert (IsCurvedWall()); return circle_center; }

  // MODIFIERS
  void invert();
  void flip();
  void rotate90();
  // PAINT
  void paint() const;

  int numConvexQuads() const { return convex_quads.size(); }
  const ConvexQuad& getConvexQuad(int i) const { return convex_quads[i]; }
  void setConvexQuad(int i, const ConvexQuad &q) { assert (i >= 0 && i < numConvexQuads()); convex_quads[i] = q; }
  void setWindowConvexQuad(int i, const ConvexQuad &q) { 
    assert (numConvexQuads() == 1);
    assert (i >= 0 && i < (int)windows.size()); 
    windows[i].set(q); }

  static void resetWallCounter() { wall_counter = 0; }
  
protected:
  static int wall_counter;

  std::string wall_name;
  std::vector<ConvexQuad> convex_quads;  
  std::vector<Window> windows;
  double height_a;
  double height_b;
  double bottom_edge;
  int material_index;
  Vec3f color;
  bool is_curved_wall;
  bool is_column;
  bool is_ramp;
  bool is_platform;

  Vec3f curved_start_trajectory;
  Vec3f curved_end_trajectory;
  Vec3f circle_center;

};

// ==================================================================================

#endif
