#ifndef _REMESH_H_
#define _REMESH_H_

#include "vectors.h"
#include "element.h"
#include "cut_plane.h"
#include <vector>
#include <set>

extern int GLOBAL_DESIRED_COUNT;
extern double GLOBAL_LONGEST_EDGE;
extern double GLOBAL_SHORTEST_EDGE;
extern double GLOBAL_DESIRED_LENGTH;
extern double EXTRA_LENGTH_MULTIPLIER;
extern double SSS_EXTRA_LENGTH_MULTIPLIER;

class MeshManager;
class Mesh;
class Triangle;
class ArgParser;


struct QuadThing {
  QuadThing(const Vec3f &a, const Vec3f &b, const Vec3f &c, const Vec3f &d, const Vec3f &n)  {
    pts[0] = a; pts[1] = b; pts[2] = c; pts[3] = d; normal = n; }
  Vec3f pts[4];
  Vec3f normal;
};


class ReMesh {

public:

  static double WorstAngleOnProjectionTriangle(MeshManager *meshes);

  static void BadTrianglesStatus(MeshManager *meshes);
  static bool EliminateBadTriangles(MeshManager *meshes);
  static void LoopSubdivision(MeshManager *meshes);
  static int SplitEdges(MeshManager *meshes,double length_threshold = -1);
  static int CollapseEdges(MeshManager *meshes,double length_threshold = -1); 
  static int FlipEdges(MeshManager *meshes);
  static int MoveVertices(MeshManager *meshes);

  static void CutThroughPlanes(MeshManager *meshes);
  static void CheatFakeMaterials(MeshManager *meshes);
  static bool TryToCheat(MeshManager *meshes, Element *elem, int vert);

  static void FixSeams(MeshManager *meshes);
  static void Triangulate(MeshManager *meshes);
  static void MoveVerticesRandomly(MeshManager *meshes);
  static void CompressVertices(MeshManager *meshes);

  static void CutEdges(MeshManager *meshes);
  static void Evaluate(MeshManager *meshes);

private:
  
  // SUBDIVISION HELPERS  
  static void SubdivisionHelperEdgeVertex(Mesh *mesh, int a, int b, int m, int n, Vec3f &v);
  static void SubdivisionHelperAddEdgeVertices(Mesh *mesh);

  // SPLIT EDGES HELPERS
  static int SplitLongestEdge(Element *e, double length_threshold); //, double max_edge_length);
  static int SplitEdge(Element *e, int i, int a, int b, double fraction, std::set<ElementID> &removed, std::set<ElementID> &added);

  static int SplitEdge(Mesh *m, int a, int b, double fraction, std::set<ElementID> &removed, std::set<ElementID> &added);
  
  static int TriSect(Mesh *m, Triangle *t, Vec3f pt, std::set<ElementID> &removed, std::set<ElementID> &added);

  // COLLAPSE HELPERS
  static bool TryCollapse(ArgParser *args, Element *e, Edge *ed, bool collapse_opposite, double length_threshold, bool bad_triangle_flag=false); 

  // FLIP HELPER
  static int Flipped(int i, Triangle *t, Triangle *t2, ArgParser *args);

  // MOVE VERTEX HELPER
  static int MoveVertex(ArgParser *args, Element *e, int vert);

  static bool MoveVertexHelper(Mesh *mesh, ArgParser *args, std::vector<Element*> &element_vec, int vert, std::vector<int> &verts,
			       int &edge_v0, int &edge_v1, double &shortest_edge, const Vec3f &normal, 
			       bool zero_area_flag);

  // CUT THROUGH HELPERS
  static void ConstructPlanes(MeshManager *meshes, std::vector<Vec3f> &projector_centers);
  static void ConstructPlanes2(MeshManager *meshes, std::vector<Vec3f> &projector_centers);
  static void CutOnPlanes(MeshManager *meshes,int num_projectors); 
  static void CutOnPlanes2(MeshManager *meshes);
  static void AssignFakeMaterials(MeshManager *meshes, std::vector<Vec3f> &proj);
  static void AssignFakeMaterials2(MeshManager *meshes, std::vector<Vec3f> &proj);

  static void RemoveIfUnderWall(MeshManager *meshes);

};

// COLLAPSE HELPERS
void DoCollapse(Mesh *m, std::vector<Element*> &element_vec, int del_vert, int repl_vert);
bool IsBoundaryCollapse(Mesh *m, std::vector<Element*> &element_vec, const int del_vert, const int repl_vert, 
			int &other_edge_vert); //edge_vert1, int &edge_vert2); //, bool &non_manifold_flag);
bool IsOkBoundaryCollapse(Mesh *m, std::vector<Element*> &element_vec, int del_vert, int repl_vert, int &other_edge_vert, ArgParser *args); //edge_vert1, int &edge_vert2, ArgParser *args);
bool SimilarTriangleRegion(Mesh *m, std::vector<Element*> &element_vec, int del_vert, int repl_vert, ArgParser *args);
bool SimilarTrajectory(const Vec3f &a, const Vec3f &b, const Vec3f &c);
bool SortIntoTwoMaterialPiles(std::vector<Element*> &element_vec, const int middle_vert, std::vector<Element*> &pile1, std::vector<Element*> &pile2, int &left_edge_vert, int &right_edge_vert, ArgParser *args, bool bad_triangle_flag);
bool SortIntoTwoNormalPiles(std::vector<Element*> &element_vec, const int middle_vert, std::vector<Element*> &pile1, std::vector<Element*> &pile2, int &left_edge_vert, int &right_edge_vert, ArgParser *args,bool bad_triangle_flag);

#endif
