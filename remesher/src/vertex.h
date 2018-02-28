#ifndef _VERTEX_H
#define _VERTEX_H

#include <cstdio>
#include <cassert>
#include "vectors.h"
#include "utils.h"
#include "markable.h"
#include "element.h"

class Vertex;
class Mesh;
class Element;
class Triangle;

// ==========================================================

class Vertex : public Markable {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Vertex(const Vec3f &pos, double s, double t) {
    //if (s < 0 || t < 0) { std::cout << "in vertex " << s << " " << t << std::endl; }
    //assert (s >= 0); // && s <= 1);
    //assert (t >= 0); // && t <= 1);
    position_set = true;
    position = pos;
    moved_position_set = false;
    moved_position = Vec3f(0,0,0);
    normal_set = false;
    normal = Vec3f(0,0,0);
    special_normal_set = false;
    special_normal = Vec3f(0,0,0);
    texture_coordinate_s = s;
    texture_coordinate_t = t;
    moved_texture_coordinate_s = -1;
    moved_texture_coordinate_t = -1;
  }
  virtual ~Vertex() {}  
  
  // POSITION
  bool ValidPosition() const { return position_set; }
  const Vec3f& get() const { 
    assert (ValidPosition());
    assert (!hasMovedPosition());
    return position; 
  }
  void getTextureCoordinates(double &s, double &t) const { 
    //assert (texture_coordinate_s >= 0 && texture_coordinate_s <= 1);
    //assert (texture_coordinate_t >= 0 && texture_coordinate_t <= 1);
    s = texture_coordinate_s; t = texture_coordinate_t; }

  void setTextureCoordinates(double s, double t) {
    texture_coordinate_s = s; texture_coordinate_t = t; 
  }

  const Vec3f& getPositionOrMovedPosition() {
    assert (ValidPosition());
    if (hasMovedPosition())
      return moved_position;
    return position;
  }
  void set2(const Vec3f &v, double s, double t) { 
    //cout << "SET " << s << " " << t << endl;
    //assert (s >= 0 && s <= 1);
    //assert (t >= 0 && t <= 1);
    position_set = true;
    position = v; 
    moved_position_set = false;
    moved_position = Vec3f(0,0,0);
    normal_set = false;
    normal = Vec3f(0,0,0); 
    special_normal_set = false;
    special_normal = Vec3f(0,0,0);
    texture_coordinate_s = s;
    texture_coordinate_t = t;
  }
  void InvalidatePosition() { 
    position_set = false; 
    position = Vec3f(0,0,0);
    moved_position_set = false;
    moved_position = Vec3f(0,0,0);
    normal_set = false;
    normal = Vec3f(0,0,0);
    special_normal_set = false;
    special_normal = Vec3f(0,0,0);
  }

  // NORMAL
  bool ValidNormal() const { return (special_normal_set || normal_set); }
  const Vec3f& getNormal() const { if (special_normal_set) return special_normal; assert (ValidNormal()); return normal; }
  void setSpecialNormal(const Vec3f &n) { special_normal_set = 1; special_normal = n; }
  void ClearNormal() { 
    normal_set = false; 
    normal = Vec3f(0,0,0); }
  void incrNormal(const Vec3f n, double w) { 
    assert (!normal_set); 
    normal += w*n; }
  void normalizeNormal() { 
    assert(!normal_set); 
    normal.Normalize();
    normal_set = true; }

  // MOVED POSITION (REMESHING)
  bool hasMovedPosition() const { return moved_position_set; }
  void setMovedPosition(double x, double y, double z, double s, double t) { 
    //assert (s >= 0 && s <= 1);
    //assert (t >= 0 && t <= 1);
    assert (ValidPosition());
    assert (!hasMovedPosition());
    moved_position_set = true; 
    moved_position.Set(x,y,z); 
    moved_texture_coordinate_s = s;
    moved_texture_coordinate_t = t;
  }
  void MovePosition() {
    assert (ValidPosition());
    if (!hasMovedPosition()) return;
    set2(moved_position,moved_texture_coordinate_s,moved_texture_coordinate_t); 
  }

  void clearBlendDistanceFromOcclusion() { blending_distance_from_occlusion.clear(); }
  double getBlendDistanceFromOcclusion(unsigned int i) const {
    if (i >= blending_distance_from_occlusion.size()) return 0.0; 
    else return blending_distance_from_occlusion[i]; 
  }
  void setBlendDistanceFromOcclusion(unsigned int i, double weight);

  // =====
  // PRINT
  void Print(const char *s = NULL) {
    if (s != NULL) printf ("%s: ",s);
    if (!ValidPosition()) printf ("invalid intersection vertex\n");  
    position.Print("vertex: ");
  }

  bool getProjectorVisibility(int i) const {
    assert (i >= 0 && i < (int)projector_visibility.size());
    return projector_visibility[i];
  }
  void setProjectorVisibility(int i, bool vis) {
    while (i >= (int)projector_visibility.size()) projector_visibility.push_back(false);
    assert (i >= 0 && i < (int)projector_visibility.size());
    projector_visibility[i] = vis;
  }

private:
  
  // don't use these!
  Vertex() { assert(0); }
  const Vertex& operator=(const Vertex&) { assert(0); return *this; }
  Vertex(const Vertex&) { assert(0); }
  
  // ==============
  // REPRESENTATION

  Vec3f position;        
  Vec3f moved_position;
  Vec3f normal;
  Vec3f special_normal;

  bool position_set;
  bool normal_set;
  bool special_normal_set;
  bool moved_position_set;  

  double texture_coordinate_s;
  double texture_coordinate_t;

  double moved_texture_coordinate_s;
  double moved_texture_coordinate_t;

  std::vector<bool> projector_visibility;
  std::vector<double> blending_distance_from_occlusion;
};

// ==========================================================

#endif

