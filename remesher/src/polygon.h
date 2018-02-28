#ifndef _POLYGON_H
#define _POLYGON_H

#include <iostream>
#include <climits>
#include <string>
#include <cassert>
#include "element.h"
#include "boundingbox.h"
#include "element.h"
#include "wall_fingerprint.h"
#include "wall.h"

class Element;
class Mesh;
class Walls;

// HACK...  need to make sure that skylight is inside the room

// ===========================================================

class Poly : public Element {

public:
  
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Poly(Mesh *m) : Element(m,"") { cell_type = ACT_OTHER; wall_id = -1; window_index = -1;which_subgroup = -1;}
  virtual ~Poly() {}

  // =========
  // ACCESSORS
  int numVertices() const { assert (verts.size() >= 3); return verts.size(); }
  int operator[](int i) const { 
    assert (i >= 0 && i < numVertices()); 
    return verts[i]; }

  void setCellType(enum ARRANGEMENT_CELL_TYPE t, int _wall_id = -1, int _window_index = -1) {
    cell_type = t;
    wall_id = _wall_id;
    window_index = _window_index;
  }

  enum ARRANGEMENT_CELL_TYPE getCellType() const { return cell_type; }

  //int getWallID() const { assert (isWall()); assert(wall_id >= 0); return wall_id; } //NULL; } //assert (wall != NULL); return wall; }
  int getWallID() const { 
    if (!isWall()) {
      std::cout << "isWall BUG!!!!\n";
      return 0;
    }
    assert (isWall()); assert(wall_id >= 0); return wall_id; } //NULL; } //assert (wall != NULL); return wall; }

  bool isWall() const { 
    /*std::cout << "iswall=" << wall_id << std::endl;*/
    return (wall_id >= 0); }

  virtual void Print(const char *s = "") const; 
  void PrintEnclosure() const;

  virtual double Area() const;
  virtual bool isATriangle() const { return 0; }
  virtual bool isAQuad() const { return 0; }
  virtual bool isAPolygon() const { return 1; }

  bool PointInside(const Vec3f &pt) const;
  bool PointInsideOrOnBorder(const Vec3f &pt) const;

  // =========
  // MODIFIERS
  void addVert(int v); //const ArrangementData &d);
  static void StealVertices(Poly*& a, Poly*& b, int vert);
  bool checkVerts() const;
  void Paint(int flag, Walls *walls);
  bool AnalyzeForShortInferredWall(Walls *walls, bool recurse=false);
  void PaintEdges();
  void PaintBoundaryEdges();

  Vec3f getCentroid();
  Vec3f getOffsetVertex(int i);

  void setPercentEnclosed(double p) { percent_enclosed = p; }
  double getPercentEnclosed() { return percent_enclosed; }

  enum FURNITURE_TYPE getFurnitureType() { return (enum FURNITURE_TYPE)window_index; }

protected:

  // don't use these
  Poly() : Element(NULL,"") { assert(0); }
  Poly& operator = (const Poly &t) { assert(0); return *this; }
  
  // ==============
  // REPRESENTATION
  std::vector<int> verts;
  ARRANGEMENT_CELL_TYPE cell_type;
  int wall_id;
  int window_index;
  double percent_enclosed;

public:
  std::vector<WALL_FINGERPRINT> print_X;
  std::vector<std::pair<Vec3f,double> > enclosure_samples;
  int which_subgroup;
};


// ===========================================================

#endif
