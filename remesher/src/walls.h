#ifndef _WALLS_H_
#define _WALLS_H_

#include <vector>
#include <cassert>
#include <string>
#include "vectors.h"
#include "utils.h"
#include "vertex.h"
#include "hash.h"
#include "argparser.h"
#include "vertextriple.h"
#include "wall_fingerprint.h"
#include <map>

class Poly;
class Mesh;
struct ChainNeighbor;
class Skylight;
class Window;
class BasicWall;
struct WallChain;
class Furniture;

class Person {
 public:
  double x,z;
  double height;
  double angle; 
  std::string name;
};

// ========================================================================


#define CIRCLE_RES 60

// 42 inch diameter table, radius in meters is:
//#define TABLE_RADIUS 21 * INCH_IN_METERS; //0.0254

// 36 foot radius room, radius in meters is:
//#define EMPAC_RADIUS 18 * 12 * INCH_IN_METERS; //0.3048


// 36 foot radius room, radius in meters is:
// dimensions for EMPAC, December 2010
#define EMPAC_MINUS_X  12 * 12 * INCH_IN_METERS 
#define EMPAC_POS_X    24 * 12 * INCH_IN_METERS 
#define EMPAC_MINUS_Z  15 * 12 * INCH_IN_METERS 
#define EMPAC_POS_Z    21 * 12 * INCH_IN_METERS 

// also overridden...  ugliness
#define EMPAC_SMALL_MINUS_X  -(          4) * INCH_IN_METERS 
#define EMPAC_SMALL_POS_X     (15 * 12 + 4) * INCH_IN_METERS 
#define EMPAC_SMALL_MINUS_Z   ( 7 * 12 + 3) * INCH_IN_METERS 
#define EMPAC_SMALL_POS_Z     (13 * 12 + 1) * INCH_IN_METERS 


// distance from origin in meters (in the positive x direction)
#define EMPAC_BACK_WALL_DISTANCE 5.2578  

// (in the z direction)
// WIDTH = 21.5 FEET 
// HEIGHT = 18 FEET
#define EMPAC_BACK_WALL_LEFT_EDGE -(21.5/2.0 - 3) * 12 * INCH_IN_METERS
#define EMPAC_BACK_WALL_RIGHT_EDGE (21.5/2.0 + 3) * 12 * INCH_IN_METERS

// (in the y direction)
#define EMPAC_BACK_WALL_BOTTOM_EDGE 9 * INCH_IN_METERS
#define EMPAC_BACK_WALL_TOP_EDGE (3 + 16.5 * 12) * INCH_IN_METERS

/*
#define EMPAC_MINUS_X 9 * 12 * INCH_IN_METERS 
#define EMPAC_POS_X   9 * 12 * INCH_IN_METERS 
#define EMPAC_MINUS_Y 7.5 * 12 * INCH_IN_METERS 
#define EMPAC_POS_Y   10.5 * 12 * INCH_IN_METERS 
*/

//#define EMPAC_RADIUS 40 * 12 * INCH_IN_METERS; //0.3048
// 45 foot radius room, radius in meters is:
//#define EMPAC_RADIUS 45 * 12 * INCH_IN_METERS; //0.3048


// ========================================================================

struct WallCamera {
  Vec3f eye;
  Vec3f horizontal;
  Vec3f direction;
  Vec3f up;
  double height;
  double width;
  double nearPlane;
  double farPlane;
};

extern std::vector<WallCamera> GLOBAL_surface_cameras;

// ==================================================================================
// ==================================================================================
// ==================================================================================

class Walls {
public:

  Walls(ArgParser *_args);
  ~Walls();

  BasicWall* getWallWithName(const std::string& name);

  double getSceneRadius() { return scene_radius; }
  /*
  bool luan_wall_exists() {
    for (unsigned int i = 0; i < walls.size(); i++) {
      if (walls[i].getName() == "luan_wall")
	return true;
    }
    return false;
  }
  */

  void addWall(BasicWall *w) { walls.push_back(w); } 
  void addFurniture(Furniture *f) { furniture.push_back(f); } 
  void addSkylight(Skylight *s) { skylights.push_back(s); }

  int numWalls() const { return walls.size(); }
  BasicWall* getWall(int i) const { assert (i >= 0 && i < numWalls()); return walls[i]; }
  
  
  bool allShort() const;
  bool allTall() const;

  int numWindows() const;
  
  /*
  Vec3f getCentroid() const {
    Vec3f answer(0,0,0);
    if (walls.size() == 0) return answer;
    for (unsigned int i = 0; i < walls.size(); i++) {
      answer += walls[i].getCentroid();
    }
    answer /= double(walls.size());
    return answer;
  }
  */

  double ceilingHeight(Vec3f pt);

  void paintWalls() const;
  void PaintFloorPlanWindows() const ;
  void paintFloor() const;
  void RenderNorthArrow() const;
  void paintWallChains() const;

  void ShuffleWalls();

  bool SegmentIntersectsWall(unsigned int i, Vec3f beginning, Vec3f end) const;
  bool SegmentIntersectsWallTop(unsigned int i, Vec3f beginning, Vec3f end) const;

  void CreateArrangement(Mesh *arrangement);
  void CreateRoofOutline(Mesh *arrangement);
  //void LabelArrangementCellsWallsOnly(Mesh *arrangement);
  void LabelArrangementCells(Mesh *arrangement);
  void ComputeSampledEnclosure(ArgParser *args, Mesh *arrangement, double &tuned_enclosure_threshold);

  void SetBoxMaterials(Vec3f red, Vec3f white, Vec3f blue);
  void CreateTriMesh(Mesh *arrangement, Mesh *mesh); 
  void CreateTriMesh(Mesh *mesh); 

  void CreateSurfaceCameras();

  double ComputePercentEnclosed(const Vec3f &v) const;
  double ComputePercentEnclosed(Poly *p) const;
  // void PrepareForFingerprints();
  void ConnectWallsIntoChains();

  std::vector<WALL_FINGERPRINT> FingerprintPoly(Poly *p) const;
  std::vector<WALL_FINGERPRINT> FingerprintPoint(const Vec3f &v) const;

private:

  // helper functions for ConnectWallsIntoChains
  void CleanupBestNeighbors(std::vector<ChainNeighbor> &best_neighbors);
  void TweakWalls(const std::vector<ChainNeighbor> &best_neighbors);
  void MakeChains(std::vector<ChainNeighbor> &best_neighbors);
  void CheckChainsForOverlap();
  


  // helper functions
  const BasicWall& WallFromFingerprint(Poly *p, Poly *p2);
  void CreateTrianglesForCell(Mesh *arrangement, Mesh *mesh, vphashtype &vphash,
			      Poly *p, int HASH, double height, bool upside_down, std::string material);
  bool InCenterOfSkylight(const Vec3f &v) const;
  bool InSkylight(const Vec3f &v) const;

  // REPRESENTATION
  std::vector<BasicWall*> walls;

  std::vector<Furniture*> furniture;

  std::vector<Skylight*> skylights;
  ArgParser *args;

  std::vector<WallChain> wall_chains;

public:
  std::vector<Person> people;
  
  
  double north_angle;
  double getNorthAngle() const { return north_angle; }
  
  // latitude and longitude
  double latitude;
  double longitude;
  
  std::vector <std::pair<Vec3f,int> > LEDs;
  Vec3f floor_material,ceiling_material;
  Vec3f furniture_material_bed;
  Vec3f furniture_material_desk;
  Vec3f furniture_material_wardrobe;
  std::vector<Vec3f> wall_materials;
  //std::vector<std::pair<Vec3f,double> > sampled_enclosure;

  Vec3f scene_center;
  double scene_radius;
  bool empac;

  static std::string luan_window;
  static std::string canvas_window;
  static std::string right_l_shaped_window;
  static std::string left_l_shaped_window;
  static std::string big_l_shaped_window;
  static std::string curved_window;

  PolygonLabels polygon_labels;
};

// ==================================================================================
#endif
