#ifndef __OBJ_LOADER_HPP__
#define __OBJ_LOADER_HPP__

#include <cstring>

#include <ImageLoader.hpp>
#include <optix_world.h>
#include <optixu/optixu_aabb.h>
#include <sstream>
#include <vector>
#include "GLM.hpp"
  using std::vector;
using namespace optix;
class AwesomeObjLoader {
public:
  AwesomeObjLoader(const std::string&, optix::Context, optix::GeometryGroup, vector<float3>&);
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

  //void createMaterial();void createMaterialQuick();
  void createGeometryInstances(GLMmodel*, optix::Program, optix::Program);
  void loadVertexData(GLMmodel*);
  void createMaterialParams(GLMmodel*);
  void loadMaterialParams(optix::GeometryInstance, unsigned int);

  std::string            mPathname;
  std::string            mFilename;
  optix::Context        mContext;
  optix::GeometryGroup  mGeometryGroup;
  optix::Buffer         mVertexBuffer;
  optix::Buffer         mNormalBuffer;
  optix::Buffer         mTexcoordBuffer;
  optix::Material      mMaterial;
  optix::Aabb            mAABB;
  std::vector<MatParams> mMaterialParams;
  int numberOfTriangles;
  vector<float3>&         mCentroids;
};


inline int
AwesomeObjLoader::getNumberOfTriangles()
{
  return numberOfTriangles;
}

inline
AwesomeObjLoader::AwesomeObjLoader(
  const std::string& filename,
  optix::Context context,
  optix::GeometryGroup geomGroup,
  vector<float3>& centroids
)
  :
    numberOfTriangles(0),
    mFilename(filename),
    mContext(context),
    mGeometryGroup(geomGroup),
    mVertexBuffer(0),
    mNormalBuffer(0),
    mTexcoordBuffer(0),
    mMaterial(0),
    mCentroids(centroids),

    mAABB() {
  mPathname = mFilename.substr(0, mFilename.find_last_of("/\\") + 1);

}

inline
void
AwesomeObjLoader::load(  optix::Material&       material) {
  mMaterial=material;
  GLMmodel* model = glmReadOBJ(mFilename.c_str());
  if(!model) {
    std::stringstream ss;
    ss << "AwesomeObjLoader::loadImpl - glmReadOBJ( '" << mFilename << "' ) failed" << std::endl;
    throw Exception(ss.str());
  }

  // Create a single material to be shared by all GeometryInstances
//  createMaterialQuick();

  // Create vertex data buffers to be shared by all Geometries
  loadVertexData(model);

  std::string path("triangleMesh.cu.ptx");
  Program meshIntersect = mContext->createProgramFromPTXFile( path, "meshIntersect" );
  Program meshBbox      = mContext->createProgramFromPTXFile( path, "meshBounds" );

  // Create a GeometryInstance and Geometry for each obj group
  createMaterialParams(model);
  createGeometryInstances(model, meshIntersect, meshBbox);

  glmDelete(model);
}

inline
optix::Aabb
AwesomeObjLoader::getSceneBBox() const {
  return mAABB;
}





inline
void
AwesomeObjLoader::loadVertexData(GLMmodel* model) {
  unsigned int numVertices  = model->numvertices;
  unsigned int numNormals   = model->numnormals;
  unsigned int numTexcoords = model->numtexcoords;

  // Create vertex buffer
  mVertexBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
  float3* vertexBufferData = static_cast<float3*>(mVertexBuffer->map());

  // Create normal buffer
  mNormalBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numNormals);
  float3* normalBufferData = static_cast<float3*>(mNormalBuffer->map());

  // Create texcoord buffer
  mTexcoordBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, numTexcoords);
  float2* texcoordBufferData = static_cast<float2*>(mTexcoordBuffer->map());

  // Copy vertex, normal and texcoord arrays into buffers
  memcpy(
    static_cast<void*>(vertexBufferData),
    static_cast<void*>(&(model->vertices[3])),
    sizeof(float) * numVertices * 3
  );
  memcpy(
    static_cast<void*>(normalBufferData),
    static_cast<void*>(&(model->normals[3])),
    sizeof(float) * numNormals * 3
  );
  memcpy(
    static_cast<void*>(texcoordBufferData),
    static_cast<void*>(&(model->texcoords[2])),
    sizeof(float) * numTexcoords * 2
 );

  mVertexBuffer->unmap();
  mNormalBuffer->unmap();
  mTexcoordBuffer->unmap();

  // Calculate bbox of model
  for(unsigned int i = 1; i <= numVertices; ++i) {
    unsigned int index = i * 3;
    float3 t;
    t.x = model->vertices[index + 0];
    t.y = model->vertices[index + 1];
    t.z = model->vertices[index + 2];

    mAABB.include(t);
  }
}

inline
void
AwesomeObjLoader::createGeometryInstances(
  GLMmodel* model,
  Program meshIntersect,
  Program meshBBox
) {
  std::vector<GeometryInstance> instances;

  // Loop over all groups -- grab the triangles and material props from each group
  unsigned int triangleCount = 0;
  unsigned int groupCount = 0;
  for(GLMgroup* obj_group = model->groups;
      obj_group != 0;
      obj_group = obj_group->next, groupCount++
  )
  {
    unsigned int numTriangles = obj_group->numtriangles;
    if(numTriangles == 0) { continue; }

    // Create vertex index buffers
    Buffer vertexIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, numTriangles);
    int3* vertexIndexBufferData   = static_cast<int3*>(vertexIndexBuffer->map());

    Buffer texcoordIndexBuffer    = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, numTriangles);
    int3* texcoordIndexBufferData = static_cast<int3*>(texcoordIndexBuffer->map());

    Buffer normalIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, numTriangles);
    int3* normalIndexBufferData   = static_cast<int3*>(normalIndexBuffer->map());

    Buffer centroidIndexBuffer      = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT, numTriangles);
    int* centroidIndexBufferData   = static_cast<int*>(centroidIndexBuffer->map());

    // This is where I left off. Continue here!

    // TODO: Create empty buffer for mat indices, have obj_material check for zero length
    Buffer materialBuffer = mContext->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT, numTriangles);
    optix::uint* materialBufferData = static_cast<optix::uint*>(materialBuffer->map());

    // Create the mesh object
    Geometry mesh = mContext->createGeometry();
    mesh->setPrimitiveCount(numTriangles);
    mesh->setIntersectionProgram(meshIntersect);
    mesh->setBoundingBoxProgram(meshBBox);
    mesh[ "vertexBuffer"        ]->setBuffer(mVertexBuffer);
    mesh[ "normalBuffer"        ]->setBuffer(mNormalBuffer);
    mesh[ "texcoordBuffer"      ]->setBuffer(mTexcoordBuffer);
    mesh[ "vertexIndexBuffer"   ]->setBuffer(vertexIndexBuffer);
    mesh[ "texcoordIndexBuffer" ]->setBuffer(texcoordIndexBuffer);
    mesh[ "normalIndexBuffer"   ]->setBuffer(normalIndexBuffer);
    mesh[ "materialBuffer"      ]->setBuffer(materialBuffer);
    mesh[ "centroidIndexBuffer"      ]->setBuffer(centroidIndexBuffer);

    // Create the geom instance to hold mesh and material params
    GeometryInstance instance = mContext->createGeometryInstance(mesh, &mMaterial, &mMaterial + 1);
    loadMaterialParams(instance, obj_group->material);
    bool clear     = false;
    bool emmissive = false;
    if(fmaxf(mMaterialParams[obj_group->material].Ts + mMaterialParams[obj_group->material].Td) > 0) {
      clear = true;
    }
    if(fmaxf(mMaterialParams[obj_group->material].Ke) > 0) {
      emmissive = true;
    }
    instances.push_back(instance);

    for(unsigned int i = 0; i < obj_group->numtriangles; ++i, ++triangleCount)
    {
      unsigned int tindex = obj_group->triangles[i];
      int3 vindices;
      numberOfTriangles++;
      vindices.x = model->triangles[ tindex ].vindices[0] - 1;
      vindices.y = model->triangles[ tindex ].vindices[1] - 1;
      vindices.z = model->triangles[ tindex ].vindices[2] - 1;
        std::vector<float3> tri;
      if(clear) {

        tri.push_back(make_float3(
                        model->vertices[3 * (vindices.x + 1) + 0],
                        model->vertices[3 * (vindices.x + 1) + 1],
                        model->vertices[3 * (vindices.x + 1) + 2]
                      ));
        tri.push_back(make_float3(
                        model->vertices[3 * (vindices.y + 1) + 0],
                        model->vertices[3 * (vindices.y + 1) + 1],
                        model->vertices[3 * (vindices.y + 1) + 2]
                      ));
        tri.push_back(make_float3(
                        model->vertices[3 * (vindices.z + 1) + 0],
                        model->vertices[3 * (vindices.z + 1) + 1],
                        model->vertices[3 * (vindices.z + 1) + 2]
                      ));
        mClearTriangles.push_back(tri);
      }
      if(emmissive) {

        tri.push_back(make_float3(
                        model->vertices[3 * (vindices.x + 1) + 0],
                        model->vertices[3 * (vindices.x + 1) + 1],
                        model->vertices[3 * (vindices.x + 1) + 2]
                      ));
        tri.push_back(make_float3(
                        model->vertices[3 * (vindices.y + 1) + 0],
                        model->vertices[3 * (vindices.y + 1) + 1],
                        model->vertices[3 * (vindices.y + 1) + 2]
                      ));
        tri.push_back(make_float3(
                        model->vertices[3 * (vindices.z + 1) + 0],
                        model->vertices[3 * (vindices.z + 1) + 1],
                        model->vertices[3 * (vindices.z + 1) + 2]
                      ));
        mEmmissiveTriangles.push_back(tri);
      }
      //printf("pushing back\n");
      mCentroids.push_back(make_float3(
          model->vertices[3 * (vindices.x + 1) + 0]
          +model->vertices[3 * (vindices.y + 1) + 0]
          +model->vertices[3 * (vindices.z + 1) + 0]
          /3,

          model->vertices[3 * (vindices.x + 1) + 1]
          +model->vertices[3 * (vindices.y + 1) + 1]
          + model->vertices[3 * (vindices.z + 1) + 1]
          /3,

          model->vertices[3 * (vindices.x + 1) + 2]
          +model->vertices[3 * (vindices.y + 1) + 2]
          + model->vertices[3 * (vindices.z + 1) + 2]
          /3
          )//make float3
          );//push_back*/
          printf("mcenteroids size %i \n", mCentroids.size());
      int3 nindices;
      nindices.x = model->triangles[ tindex ].nindices[0] - 1;
      nindices.y = model->triangles[ tindex ].nindices[1] - 1;
      nindices.z = model->triangles[ tindex ].nindices[2] - 1;

      int3 tindices;
      tindices.x = model->triangles[ tindex ].tindices[0] - 1;
      tindices.y = model->triangles[ tindex ].tindices[1] - 1;
      tindices.z = model->triangles[ tindex ].tindices[2] - 1;

      vertexIndexBufferData[ i ]   = vindices;
      normalIndexBufferData[ i ]   = nindices;
      centroidIndexBufferData [i]  = mCentroids.size()-1; //CHANGEME
      texcoordIndexBufferData[ i ] = tindices;
      materialBufferData[ i ] = 0; // See above TODO
    }

    vertexIndexBuffer->unmap();
    texcoordIndexBuffer->unmap();
    centroidIndexBuffer->unmap();
    normalIndexBuffer->unmap();
    materialBuffer->unmap();
  }

  assert(triangleCount == model->numtriangles);

  // Set up group
  mGeometryGroup->setChildCount(static_cast<unsigned int>(instances.size()));
  Acceleration acceleration = mContext->createAcceleration("Sbvh","Bvh");
  acceleration->setProperty("vertex_buffer_name", "vertexBuffer");
  acceleration->setProperty("index_buffer_name", "vertexIndexBuffer");
  mGeometryGroup->setAcceleration(acceleration);
  acceleration->markDirty();

  for(unsigned int i = 0; i < instances.size(); ++i)
  {
    mGeometryGroup->setChild(i, instances[i]);
  }
}

inline
void
AwesomeObjLoader::loadMaterialParams(
  GeometryInstance gi,
  unsigned int index
) {
  // If no materials were given in model use reasonable defaults
  if(mMaterialParams.empty()) {
    std::cerr << " AwesomeObjLoader not setup to use material override yet!" << std::endl;
    return;
  }

  // Load params from this material into the GI
  if(index < mMaterialParams.size()) {
    MatParams& mp = mMaterialParams[index];
    gi[ "Kd" ]->setFloat(mp.Kd);
    gi[ "Ks" ]->setFloat(mp.Ks);
    gi[ "Td" ]->setFloat(mp.Td);
    gi[ "Ts" ]->setFloat(mp.Ts);
    gi[ "Ke" ]->setFloat(mp.Ke);
    return;
  }

  // Should never reach this point
  std::cerr << "WARNING -- AwesomeObjLoader::loadMaterialParams given index out of range: "
            << index << std::endl;
}

inline
void
AwesomeObjLoader::createMaterialParams(GLMmodel* model) {
  mMaterialParams.resize(model->nummaterials);
  for(unsigned int i = 0; i < model->nummaterials; ++i) {
    GLMmaterial& mat = model->materials[i];
    MatParams& params = mMaterialParams[i];

    float3 Kd = make_float3(mat.diffuse[0],   mat.diffuse[1],   mat.diffuse[2]  );
    float3 Ks = make_float3(mat.specular[0],  mat.specular[1],  mat.specular[2] );
    float3 Td = make_float3(mat.transdiff[0], mat.transdiff[1], mat.transdiff[2]);
    float3 Ts = make_float3(mat.transspec[0], mat.transspec[1], mat.transspec[2]);
    float3 Ke = make_float3(mat.emmissive[0], mat.emmissive[1], mat.emmissive[2]);

    params.Kd   = Kd;
    params.Ks   = Ks;
    params.Td   = Td;
    params.Ts   = Ts;
    params.Ke   = Ke;
    params.name = mat.name;
  }
}

#endif
