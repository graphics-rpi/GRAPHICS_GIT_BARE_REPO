#ifndef MESH_H
#define MESH_H
#include <cassert>
#include <map>
#include <set>

#include "vectors.h"
#include "utils.h"
#include "hash.h"
#include "boundingbox.h"
#include "material.h"
#include "hash.h"
#include "element.h"
#include "patch.h"

class Element;
class Vertex;
class Edge;
class Triangle;
class Polygon;
//class ArgParser;
class MeshManager;

#include "material.h"

// ======================================================================
// ======================================================================

class Mesh {

 public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Mesh();
  virtual ~Mesh();

  // =========
  // ACCESSORS
  virtual void Check();

  // vertices
  unsigned int numVertices() const { return vertices.size(); }
  Vertex* getVertex(int i) const {
    assert (i >= 0 && i < (int)numVertices());
    Vertex *v = vertices[i];
    assert (v != NULL);
    return v; }
  int getChildVertex(int parent_a, int parent_b) const;

  void clearParents() { vertex_parents.clear(); }

  // edges
  //unsigned int numEdges() const { return edges->Count(); }
  const edgeshashtype& getEdges() const { return edges; }
  Edge* GetEdge(int a, int b, const Element *e) const;
  const std::vector<Edge*>& GetEdges(int a, int b) const;

  unsigned int numElements() const { return elements.size(); }
  unsigned int numLines() const;
  unsigned int numTriangles() const;
  unsigned int numQuads() const;

  unsigned int numBadElements() const;
  const elementshashtype& getElements() const { return elements; }

  bool IsSimpleBoundaryEdge(Edge *e) const; //, int a, int b) const;
  bool IsManifoldInteriorEdge(Edge *e) const; //, int a, int b) const;
  bool IsNonManifoldEdge(Edge *e) const; //, int a, int b) const;

  bool IsSimpleBoundaryVertex(Element *e, int vert, std::vector<Element*> *elements=NULL) const;
  bool IsManifoldInteriorVertex(Element *e, int vert, std::vector<Element*> *elements=NULL) const;

  // other stats
  BoundingBox* getBoundingBox() const { return bbox; }
  double Area() const {
    assert (modified == false);
    return area; }
  double WeightedArea() const {
    assert (modified == false);
    return weighted_area; }
  double ShortestEdge() const {
    assert (modified == false);
    return shortest_edge; }
  double LongestEdge() const {
    assert (modified == false);
    return longest_edge; }
  double ZeroAreaTolerance() const {
    assert (zero_area_tolerance > 0);
    return zero_area_tolerance; }

  // =========
  // MODIFIERS
  virtual void Clear();
  virtual int addVertex(const Vec3f &pos, int parent_a, int parent_b, double s, double t);
  bool addElement(Element *e);

  void removeElement(Element *e);
  void Modified() { modified = true; }
  void RecomputeStats();

  // helpers
  bool AddEdges(Element *e);
  Edge* AddEdge(int a, int b, Element *e);
  void RemoveEdges(Element *e);
  void RemoveEdge(Edge *e);

  void clearMaterials() {
    materials.clear();
  }

  void addMaterial(const MeshMaterial &m);


  bool materialExists(const std::string &name) const {
    std::map<std::string,MeshMaterial>::const_iterator i = materials.find(name);
    if (i == materials.end()) {
      return false;
    }
    return true;
  }

  const MeshMaterial& getMaterial(const std::string &name) const {
    static MeshMaterial default_material = MeshMaterial();
    if (materials.size() == 0) return default_material;
    std::map<std::string,MeshMaterial>::const_iterator i = materials.find(name);
    if (i == materials.end()) {
      std::cout << "couldn't find material: " << name << std::endl;
      //assert (0);
      // can't find it, just grab the first one!
      assert (materials.size() > 0);
      i = materials.begin();
    }
    assert (i != materials.end());
    return i->second;
  }

  const MeshMaterial& getMaterialFromSimpleName(const std::string &name) const {
    assert (materials.size() > 0);
    std::map<std::string,MeshMaterial>::const_iterator itr = materials.find(name);
    if (itr == materials.end()) {
      itr = materials.find("FILLIN_"+name);
    }
    assert (itr != materials.end());
    return itr->second;
  }


  const std::map<std::string,MeshMaterial>& getMaterials() const {
    return materials;
  }

  std::set<std::string> getSimpleMaterialNames() const;

  void ComputeBlendWeights();
  void ComputeBlendDistanceFromOcclusion();
  void CompressVertices();


  // --------------------------------------------------
  void AssignPatchesAndZones(MeshManager *meshes);
  void SeedPatches(MeshManager *meshes);
  void AddOrDeleteSeeds(MeshManager *meshes);
  void IteratePatches(MeshManager *meshes, int num_iterations);
  void RandomizePatchColors(MeshManager *meshes);
  void RandomizeZoneColors(MeshManager *meshes);
  void AssignZones(MeshManager *meshes);
  // --------------------------------------------------

 private:
  bool ElectNewSeeds(MeshManager *meshes);
  void AnalyzeZones(MeshManager *meshes);
public:
  void CheckPatches() const;

public: // for now, should restructure I/O??
  void AddZone(const std::string &name, const Vec3f &color);
  void AddPatch(const Vec3f &color);

 public:

  // ===========================
  // PATCH & ZONE PUBLIC INTERFACE
  unsigned int numPatches() const { return patches.size(); }
  unsigned int numZones() const { return zones.size(); }


  PatchID getAssignedPatchForElement(ElementID id) const;
  const std::set<ElementID>& getElementsInPatch(PatchID pid) const {
    assert (pid >= 0 && pid < (int)numPatches());
    return patches[pid].getElementsInPatch(); }
  double getPatchArea(PatchID pid) const {
    assert (pid >= 0 && pid < (int)numPatches());
    return patches[pid].getArea(); }
  const Vec3f& getPatchColor(PatchID pid) const {
    assert (pid >= 0 && pid < (int)numPatches());
    return patches[pid].getColor(); }
  const Vec3f& getZoneColor(ZoneID zid) const {
    assert (zid >= 0 && zid < (int)numZones());
    return zones[zid].getColor(); }

  const Patch& getPatch(PatchID pid) const {
    assert (pid != -1);
    assert (pid >= 0 && pid < (int)numPatches());
    return patches[pid]; }

  Patch& getPatch(PatchID pid) {
    assert (pid != -1);
    assert (pid >= 0 && pid < (int)numPatches());
    return patches[pid]; }

  const Zone& getZone(ZoneID zid) const {
    assert (zid != -1);
    assert (zid >= 0 && zid < (int)numZones());
    return zones[zid]; }

  Zone& getZone(ZoneID zid) {
    assert (zid != -1);
    assert (zid >= 0 && zid < (int)numZones());
    return zones[zid]; }


  ZoneID getAssignedZoneForPatch(PatchID id) const;
  ZoneID getAssignedZoneForElement(ElementID id) const;
  const std::set<PatchID>& getPatchesInZone(ZoneID zid) const {
    assert (zid >= 0 && zid < (int)numZones());
    return zones[zid].getPatchesInZone(); }
  // ===========================


 private:
  void ComputeNormals();

 protected:

  // ==============
  // REPRESENTATION
  std::vector<Vertex*> vertices;
  elementshashtype elements;
  edgeshashtype edges;
  BoundingBox *bbox;
  vphashtype vertex_parents;

  std::map<std::string,MeshMaterial> materials;

  // patch & zone data structures
public:
  std::map<ElementID,PatchID> element_to_patch;
private:
  std::vector<Patch> patches;
  std::vector<Zone> zones;

  bool modified;
  double area;
  double weighted_area;
  double zero_area_tolerance;
  double shortest_edge;
  double longest_edge;

  // THIS ORIGINAL IS STORED IN meshes->getWalls();
  double north_angle_copy;
  double longitude_copy;
  double latitude_copy;


 public:

  void setNorthAngleCopy(double na) { 
    std::cout <<"Inside Set North Angle Copy oldvalue:" <<north_angle_copy;
    north_angle_copy = na; 
    std::cout <<" newvalue:" <<north_angle_copy <<std::endl;
  }

  double getNorthAngleCopy()const { 
    std::cout <<"Inside Get North Angle Copy current value:" <<north_angle_copy <<std::endl;
    return north_angle_copy; 
  }

  void setCoordinateCopy(double lon, double lat) { 
    longitude_copy = lon;
    latitude_copy  = lat;
    std::cout << "mesh.h setCoordinateCopy " << lon << "," << lat << std::endl;
  }
  
  double getLongitudeCopy()const{
      return longitude_copy;
  }

  double getLatitudeCopy()const{
      return latitude_copy;
  }

  MeshManager *meshes;

};

// ======================================================================
// ======================================================================

#endif

