#ifndef _PATCH_
#define _PATCH_

#include <set>
#include <string>
#include "element.h"
#include "plane.h"

// these are int not unsigned int because -1 is a value assignment
typedef int PatchID;
typedef int ZoneID;

class Mesh;


// =========================================================================
// =========================================================================

class Patch {

 public:

  Patch() { assert(0); }
  Patch(PatchID my_pid, ElementID seed_id, const Vec3f &c) {
    //std::cout << "IN PATCH CONSTRUCTOR" << std::endl;
    pid = my_pid;
    seed = seed_id; // seed == 0 is invalid
    color = c;
    // assert (seed_id > 0);
    total_area = 0;
    zid = -1;
    elements.clear();
    if (seed > 0) {
      elements.insert(seed);
      Element *e = Element::GetElement(seed);
      assert (e != NULL);
      total_area += e->Area();
    }
    area_set = true;
  }

  // ACCESSORS
  PatchID getID() const { return pid; }
  ElementID getSeed() const { return seed; }
  const Vec3f& getColor() const { return color; }
  double getArea() const { 
    /*    double my_area = ComputeArea();
    //    return my_area;
    if (fabs(total_area-my_area) > 0.001) { 
      std::cout << "AREA MISMATCH  ";
      std::cout << "patch " << pid << " area " << total_area << " =? " << my_area << std::endl;
    }
    if (area_set != true) { std::cout << "patch " << pid << " AREA NOT SET" << std::endl;}
    */
    if (area_set != true) {
      Patch *p = (Patch*)this;
      p->recomputeArea();
    }
    assert (area_set == true); 
    return total_area; 
  }
  const Vec3f& getNormal() const { return normal; }
  const Vec3f& getCentroid() const { return centroid; }
  ZoneID getZone() const { return zid; }
  const std::set<ElementID>& getElementsInPatch() const { return elements; }

  double ComputeArea() const {
    double answer = 0; 
    for (std::set<ElementID>::const_iterator itr = elements.begin(); itr != elements.end(); itr++) {
      answer += Element::GetElement(*itr)->Area();
    }
    return answer;
  }


  // MODIFIERS
  void setSeed(ElementID s) { seed = s; }
  void setColor(const Vec3f& c) { color = c; }
  void setArea(double a) { area_set = true; total_area = a; }
  void recomputeArea() { area_set = true; total_area = ComputeArea(); }
  void setNormal(const Vec3f& n) { normal = n; }
  void setCentroid(const Vec3f& c) { centroid = c; }
  void setZone(ZoneID z) { zid = z; }
  void addElementToPatch(ElementID id) { 
    elements.insert(id); area_set = false; 
  }
  void removeElementFromPatch(ElementID id) { 
    std::set<ElementID>::iterator itr = elements.find(id); 
    assert (itr != elements.end());
    elements.erase(itr);
    area_set = false;
  }
  void clearElementsInPatch() { elements.clear(); }

 private:
  // REPRESENTATION
  PatchID pid;

  ElementID seed;
  Vec3f color;
  double total_area;
  bool area_set;
  Vec3f normal;
  Vec3f centroid;
  ZoneID zid;

  std::set<ElementID> elements;
};

class Zone {

 public:
 Zone(const std::string &n, const Vec3f &c) : name(n), color(c) { total_area = 0; }
  
  // ACCESSORS
  const std::string& getName() const { return name; }
  const Vec3f& getColor() const { return color; }
  double getArea() const { return total_area; }

  // MODIFIERS
  void setColor(const Vec3f& c) { color = c; }

  const std::set<PatchID>& getPatchesInZone() const { return patches; }
  void addPatchToZone(PatchID pid) {
    patches.insert(pid);
    area_set = false;
  }
  
private:
  //ZoneID zid;
  std::string name;
  double total_area;
  bool area_set;
  Vec3f color;
  std::set<PatchID> patches;
};


class PreZone {
public:
  PreZone(const std::string &n, const Vec3f &c, const Plane &p, double d) 
    : name(n), color(c), plane(p), tolerance(d) { }
  std::string name;
  Vec3f color;
  Plane plane;
  double tolerance;
};


#endif
