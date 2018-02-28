#ifndef _MESH_MANAGER_H_
#define _MESH_MANAGER_H_

#include <vector>
#include "remesher.h"
#include "argparser.h"
#include "walls.h"
#include "cut_plane.h"

// ======================================================================
// ======================================================================

class MeshManager {

public:

  // CONSTRUCTOR & DESTRUCTOR
  MeshManager(const std::vector<std::string> &command_line_args) {
    args = new ArgParser(command_line_args);
    mesh = new Mesh(); 
    mesh->meshes = this;
    arrangement = new Mesh(); 
    walls = new Walls(args);
  }
  void DeleteMemory() {
    delete mesh; 
    delete arrangement; 
    delete args;
    delete walls;
    mesh = NULL;
    arrangement = NULL;
    args = NULL;
    walls = NULL;
  }

  ~MeshManager() {
    DeleteMemory();
  }

  // ACCESSORS
  Mesh* getMesh() const { return mesh; }
  Mesh *getArrangement() const { return arrangement; }
  Walls* getWalls() const { return walls; }

  Mesh* stealMesh() { Mesh* tmp = mesh; mesh = NULL; DeleteMemory(); return tmp; }

  // MODIFIERS
  void Clear() {
    mesh->Clear();
    arrangement->Clear();
    delete walls;
    walls = new Walls(args);
  }  

private:

  // REPRESENTATION
public:
  ArgParser *args;
private:
  Mesh *mesh; 
  Mesh *arrangement;
  Walls *walls;

public:
  // hack!
  double min_x, min_y, max_x, max_y;
  int grid;

  std::vector<CutPlane> planes;

};

// ======================================================================
// ======================================================================

#endif
