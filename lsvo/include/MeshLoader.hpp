#ifndef __MESH_LOADER_HPP__
#define __MESH_LOADER_HPP__

//#include <cstring>

#include <ImageLoader.hpp>
#include <optix_world.h>
#include <optixu/optixu_aabb.h>
#include <sstream>
#include <vector> 
#include "mesh.h"
#include "vertex.h"
#include "vectors.h"
#include "edge.h"
#include "element.h"
#include <map>
#include "num_neighbors.h"

#define GLASS      0
#define NONGLASS   1

#define MaterialMap const std::map<std::string,MeshMaterial>&

  using std::vector;
using namespace optix;
class MeshLoader {
public:
  MeshLoader(  
      optix::Context, 
      optix::GeometryGroup, vector<float3>&, 
      vector<float3>&,  
      vector<float>&,vector<int>&,
      vector<int>&,vector<float>&,
      vector<int>&,vector<float>&,
      const Mesh* meshIn, 
      double& northParam,
      double& longitude,
      double& latitude
      );

  optix::Aabb getSceneBBox() const;
  void load(  optix::Material&       material);

  std::vector< std::vector<float3> > mClearTriangles;
  std::vector< std::vector<float3> > mEmmissiveTriangles;
  int getNumberOfTriangles();

private:
  struct MatParams {
    std::string name;
    float3 Kd;
    float3 Ks;
    float3 Td;
    float3 Ts;
    float3 Ke;
    optix::TextureSampler ambientMap;
    optix::TextureSampler diffuseMap;
    optix::TextureSampler specularMap;

  };

  void createGeometryInstances(const Mesh*, optix::Program, optix::Program);
  void loadVertexData(const Mesh*);
  void createMaterialParams(const Mesh*);
  void loadMaterialParams(optix::GeometryInstance, unsigned int);

  std::string            mPathname;
  std::string            mFilename;
  optix::Context        mContext;
  optix::GeometryGroup  mGeometryGroup;
  optix::Buffer         mVertexBuffer;
  optix::Buffer         mPatchAreaBuffer;
  optix::Buffer         mNormalBuffer;
  optix::Buffer         mTexcoordBuffer;

  optix::Material      mMaterial;
  optix::Aabb            mAABB;
  std::vector<MatParams> mMaterialParams;
  int numberOfTriangles;
  vector<float3>&         mCentroids;
  vector<float3>&         mNormals;
  vector<float>&         mPatchAreas;
  vector<int>&          mPatchStartIndexes;
  vector<int>&          mPatchSizes;
    vector<float>&          mTriAreas;
  vector<int>&         mNeighbors;
  vector<float>&         mNeighborWeights;
  const Mesh* mesh;
  std::map<int, int>         remesh_to_optix_translation;
  std::vector<int>    optix_to_remesh_translation;
  std::vector<std::vector<std::pair<ElementID,double> > > neighbors_with_weights;
    int size_in_loadVertexData;
  int size_in_createGeometryInstances;
  double& north;
  double& longitude;
  double& latitude;

};

inline int
MeshLoader::getNumberOfTriangles()
{
  return numberOfTriangles;
}

inline
optix::Aabb
MeshLoader::getSceneBBox() const {
  return mAABB;
}



#endif
