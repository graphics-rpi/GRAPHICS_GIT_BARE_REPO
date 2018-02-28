#ifndef _ELEMENT_H
#define _ELEMENT_H

#include <cstdio>
#include <vector>
#include <cassert>
#include <set>
#include <string>
#include "markable.h"
#include "vectors.h"
#include "hash.h"
#include "element.h"



// JOSH ADDED
//#define NORMAL_GROUP_SIZE 8


template <class T, class T2> class AccelerationGrid; 

class Mesh;
class BoundingBox;
//template <class BAG_ELEMENT> class Bag;
typedef unsigned int ElementID;
class MeshMaterial;

class ArgParser;

// ============================================================
// Element is the parent class for Triangles and Polygons

class Element : public Markable {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Element(Mesh *m,std::string mat);
  virtual ~Element();

  // Every element ever created has a unique ID
  ElementID getID() const { return myID; }
  static Element* GetElement(ElementID id);
  friend std::ostream& operator<<(std::ostream &ostr, Element &e) {
    ostr << " element { ";
    ostr << " num_vertices=" << e.numVertices();
    ostr << " id=" << e.getID();
    Vec3f cent;
    e.computeCentroid(cent);
    ostr << " centroid=" << cent << " }" << std::endl;
    return ostr;
  }

  // =========
  // ACCESSORS
  // vertices
  virtual int numVertices() const = 0;
  virtual int operator[](int i) const = 0;
  Edge* get_my_edge(int i) const {
    if (isALine()) return NULL;
    assert ((int)my_edges.size() == numVertices());
    assert (i < numVertices());
    return my_edges[i]; 
  }
  int HasVertex(int v) const {
    for (int i = 0; i < numVertices(); i++) {
      if (v == (*this)[i]) return 1; }
    return 0; }
  int HasBothVertices(int v1,int v2) const {
    return (HasVertex(v1) && HasVertex(v2)); }
  int WhichVertex(int v) const {
    for (int i = 0; i < numVertices(); i++) {
      if (v == (*this)[i]) return i; }
    assert (0); return -1; }
  void getBoundingBox(BoundingBox &bb) const;

  std::vector<Element*> getNeighbors(int i) const; 
  bool hasNeighbor(const Element *e) const;

  Mesh* getMesh() const { assert (mesh != NULL); return mesh; }
  
  const MeshMaterial* getMaterialPtr() const { return material_ptr; }
  //std::string getRealMaterialName() const { return material_name; }
  std::string getRealMaterialName() const; // { return material_ptr->getName(); }
  //std::string getFakeMaterial() const { return material_name+"_FAKE"+sidedness+"DUMMY"; }
  std::string getFakeMaterial() const { return getRealMaterialName()+"_FAKE"+sidedness+"DUMMY"; }

  void setMaterial(const std::string &s);
  void setSidedness(unsigned int i, char c);
  char getSidedness(unsigned int i) const;
  void setSidedness(std::string s) { assert (s.size() == sidedness.size()); sidedness = s; }
  const std::string& getSidedness() const { return sidedness; }

  // printing
  virtual void Print(const char *s = "") const { printf ("ELEMENT PRINT %s %d\n", s, myID); }
  virtual void PrintLots(const char *s = "") const { Print(s); }

  // NOT implemented with dynamic casts, they were very expensive when profiled
  virtual bool isALine() const { return 0; }
  virtual bool isATriangle() const = 0;
  virtual bool isAQuad() const = 0;
  virtual bool isAPolygon() const = 0;

  double getBlendWeight(int vert, int proj) const {
    if (isALine()) return 1;
    assert (isATriangle());
    //assert (my_blend_weights != NULL);
    assert (vert >= 0 && vert < 3);
    assert (proj >= 0 && proj < num_projectors);
    return my_blend_weights[vert*num_projectors + proj];
  }
  void setBlendWeight(int vert, int proj, double weight) {
    if (isALine()) return;
    assert (isATriangle());
    //assert (my_blend_weights != NULL);
    assert (vert >= 0 && vert < 3);
    assert (proj >= 0 && proj < num_projectors);
    my_blend_weights[vert*num_projectors + proj] = weight;
  }


  double getBlendWeightWithDistance(int vert, int proj) const {
    if (isALine()) return 1;
    assert (isATriangle());
    //assert (my_blend_weights != NULL);
    assert (vert >= 0 && vert < 3);
    assert (proj >= 0 && proj < num_projectors);
    return my_blend_weights_with_distance[vert*num_projectors + proj];
  }
  void setBlendWeightWithDistance(int vert, int proj, double weight) {
    if (isALine()) return;
    assert (isATriangle());
    //assert (my_blend_weights != NULL);
    assert (vert >= 0 && vert < 3);
    assert (proj >= 0 && proj < num_projectors);
    my_blend_weights_with_distance[vert*num_projectors + proj] = weight;
  }


  //  virtual double getBlendWeight(int vert, int proj) const { exit(0); }
  //virtual void setBlendWeight(int vert, int proj, double weight) { exit(0); }
  //virtual 
  //void normalizeBlendWeights();// { exit(0); }

  //void Element::normalizeBlendWeights() {
  void normalizeBlendWeights(ArgParser *args);

  // ================
  // GEOMETRIC THINGS
  double Angle(int i);// const;
  virtual double Area() const = 0;

  bool IsBad() const { return (NearZeroArea() || BadNormal() || NearZeroAngle()); }  
  bool NearZeroArea() const;
  bool NearZeroAngle() const;
  bool BadNormal() const;
  bool HasBadNeighbor() const;

  virtual void computeCentroid(Vec3f &centroid) const;
  void LongestEdge(int &i, int &a, int &b, double &longest) const;
  //  double LongestEdge();
  double ShortestEdge() const;
  virtual int ShortestEdgeIndex() const { assert (0); return 0; }

  double ShortestMovedEdge() const;
  double MovedPositionArea() const;
  void computeMovedNormal(Vec3f &normal) const;

  virtual void computeNormal(Vec3f &normal) const;
  static void computeNormal(Mesh *m, int a, int b, int c, Vec3f &normal);

public:
  // lookup table for the id's
  static elementshashtype all_elements; 
  static AccelerationGrid<Element*,ElementID> *acceleration_grid;

  static void MakeAccelerationGrid(int dx, int dy, int dz);
  static void DestroyAccelerationGrid();

  static void GetElementsInBB(Vec3f &minx, Vec3f &maxx, std::set<ElementID> &ids);

  void setNewID(ElementID n) { newID = n; }
  ElementID getNewID() { return newID; }

  static ElementID getCurrentID() { return currentID; }


  
  // ADDED FOR LSVO
  // return all elements that have same material & similar normal
  // and share this vertex, the weights will sum to 1.0
  // If num_neighbors is specified, the vector will contain exactly that many elements
  // and the weights will be normalized to sum to 1.0
  std::vector<std::pair<ElementID,double> > getNeighborsForInterpolation(int i, int num_neighbors = -1);

  // JOSH
  //ElementID[3]* getNormalGroups() {return /*(ElementID **)*/normal_group_by_index; }
  //float ** getNormalGroupWeights() {return (float**) normal_group_by_index; }



private:

  

  // don't use the copy constructor!
  Element& operator = (const Element &/*e*/) { assert(0); exit(0); }
  Element(const Element &/*e*/) { assert(0); exit(0); }
  Element() { assert(0); exit(0); }

  // ==============
  // REPRESENTATION

 public:  // HORRIBLE HACK! TO ALLOW RESETTING OF ID
  static ElementID currentID;
 private:

  ElementID myID;
  Mesh *mesh;
  ElementID newID;

  //ugli laziness hack for my_edges :(
  friend class Mesh;

 public:
  int acceleration_bb[6];  // min_i,min_j,min_k,max_i,max_j,max_k;

 protected:
  const MeshMaterial *material_ptr; 
  //  std::string material_name;
  std::string sidedness;
  std::vector<Edge*> my_edges;
  std::vector<double> angles;

  std::vector<double> my_blend_weights;
  std::vector<double> my_blend_weights_with_distance;

  int num_projectors; // silly to store this here, but...


  // JOSH ADDED
  //ElementID normal_group_by_index [3][NORMAL_GROUP_SIZE];
  //float normal_group_weight_by_index [3][NORMAL_GROUP_SIZE];


};


void PrintAllElements(Mesh *mesh);

// ==========================================================

#endif
