#ifndef _MESH_IO_H
#define _MESH_IO_H

class Mesh;
class MeshManager;
class ArgParser;

#include <string>
#include <set>
#include <vector>

class BasicWall;
class Vec3f;
class MeshMaterial;

class MeshIO {

public:

  // LOAD
  static void Load(MeshManager *meshes, const std::string &filename);

  // SAVE
  static void Save(MeshManager *meshes, Mesh *mesh, const std::string &filename);

  //private:

  static void LoadJSON(MeshManager *meshes, const std::string &filename);

  static void LoadNOS(Mesh *mesh, const std::string &filename, ArgParser *args);
  static void LoadMTL(Mesh *mesh, const std::string &filename, ArgParser *args);
  static void LoadLSVMTL(Mesh *mesh, const std::string &filename, ArgParser *args);
  static void LoadOBJ(Mesh *mesh, const std::string &filename, ArgParser *args);
  //, std::vector<Element*> *optional_load_vector = NULL);
  static void LoadWALL(MeshManager *meshes, const std::string &filename);
  static void LoadLED(MeshManager *meshes, const std::string &filename);
  static void LoadARMY(MeshManager *meshes, const std::string &filename);
  static void LoadColorsFile(MeshManager *meshes);

private:

  static BasicWall* MakeLShapedWall(MeshManager *meshes, Vec3f &led_a, Vec3f &led_b, Vec3f &led_c, Vec3f &led_d, std::string &window_option);

 public:
  static void SaveOBJ(MeshManager *meshes, Mesh *m, const std::string &filename,
		      std::vector<std::pair<std::vector<Vec3f>,const MeshMaterial*> > *optional_tri_vec=NULL);

 private:
  static void SaveMTL(MeshManager *meshes, Mesh *m, const std::string &filename, std::set<std::string> used_materials);
  static void SaveLSVMTL(MeshManager *meshes, Mesh *m, const std::string &filename, std::set<std::string> used_materials);

  static void AfterLoading(MeshManager *meshes);
};


#endif

