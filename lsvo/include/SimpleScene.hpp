
#pragma once
//#define LO_MEM
#include <algorithm>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
//#include <WinSock2.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

#include <boost/bind.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>

#include <optix_world.h>
#include <sutil.h>
#include <SampleScene.hpp>
#include "TemplateHelpers.hpp"
#include "MeshLoader.hpp"
#include "Hit.hpp"
#include "Light.hpp"
#include "Photon.hpp"
#include <stdio.h>
#include <time.h>
#include "mesh.h"
#include "state.hpp"
#include <OrthoParser.hpp>

#ifdef USE_OBJLOADER
#include "ObjLoader.hpp"
#endif

#define NUM_MOMENTS 27

typedef boost::mt19937        Generator;
typedef boost::mt19937        WindowGenerator;
typedef boost::uniform_real<> WindowDistribution;
typedef boost::variate_generator<WindowGenerator&, WindowDistribution> WindowVariateGenerator;

class SimpleScene : public SampleScene {
public:
 // SimpleScene();
  SimpleScene(const Mesh* meshIn,State &, int inIndex,  bool& done);

  void           initScene(InitialCameraData& cameraData);
  void           trace(const RayGenCameraData& cameraData);
  optix::Buffer getOutputBuffer();
  void           cleanUp();
  void		 toggleDetail();
   bool activeDisplay();
   bool isDone(){return done;}

private:
State & state;
Buffer intermediateBuffer;
#ifdef USE_OBJLOADER
  AwesomeObjLoader* loader;
#else
  MeshLoader* loader;
#endif
  int _frames_since_mod;
  bool _mode_changed;
  void createMaterial();
  void createMaterialQuick();
  void initRayPrograms();
  float3 transformPoint(float3 point, OrthoParser& parser);
  void loadObj(SampleScene::InitialCameraData&);
  void createPhotonMap();
   void updateDate(float3& sunDirection);
  void buildKDTree(
    PhotonRecord**, const int, const int, unsigned int,
    PhotonRecord*, unsigned int, const float3&, const float3&
  );
  void writeLightingValues(std::string);
  void copyBuffersToGPU();
  void fillPatchBuffer();
  void dumpPatchBuffer();
  void dumpTriBuffer();
  void generateWindowProbabilities();
  void generateAreaLightProbabilities();
  void startOver(const RayGenCameraData& cameraData);
  void traceEyePass( RTsize bufferWidth, RTsize bufferHeight);
  void setOrtho(std::string filename);
  void setPersonCamera(Person person, int viewNumber);
  void doResize(unsigned int width, unsigned int height) {
  //FIXME
    printf("in do resize %d %d \n", width, height);
   // sleep(1);
  }
  void momentInit();
  bool momentIterator();

  unsigned int currentMoment;
  int numFramesPerMoment;

  //optix::Context   _context;
  optix::Buffer    mOutputBuffer;
  optix::Buffer    mSkyPhotons;
  optix::Buffer    mAreaLightPhotons;
  optix::Buffer    mPhotonMapBuffer;
  optix::Buffer    mRandBuffer;
  optix::Buffer    mWindowLaunchBuffer;
  optix::Buffer    mAreaLightLaunchBuffer;
  optix::Buffer dirLightBuffer;
  optix::Material  mMaterial;
  optix::Buffer patchValueBuffer;
  optix::Buffer MomentBuffer;

  std::vector<ClearWindowInfo>       mClearWindowInfos;
  std::vector<ClearWindowLaunchInfo> mClearWindowLaunchInfos;
  std::vector<double>                mWindowProbabilities;

  std::vector<ClearWindowInfo>       mAreaLightInfos;
  std::vector<AreaLightLaunchInfo>   mAreaLightLaunchInfos;
  std::vector<double>                mAreaLightProbabilities;

  std::vector<float3>                mCentroids;
  std::vector<float3>                mNormals;
  std::vector<float>                 mPatchAreas;
    std::vector<float>                 mTriAreas;
  std::vector<int>                   mPatchStartIndexes;
  std::vector<int>                   mPatchSizes;
    std::vector<int>                 mNeighbors;
    std::vector<float>                 mNeighborWeights;

  unsigned int mFrameCount;

  timeval start;
  bool _quick_render;
  bool _eye_pass_init;
  bool _importon_pass_init;
#ifdef LO_MEM
  static const unsigned int WIDTH =256;
  static const unsigned int HEIGHT = 256;
  static const unsigned int MAX_PHOTON_COUNT = 2;
  static const unsigned int PHOTONS_PER_PASS = 1000;
  
#else
  static const unsigned int WIDTH =1000;
  static const unsigned int HEIGHT = 1000;
  static const unsigned int MAX_PHOTON_COUNT = 10;
  static const unsigned int PHOTONS_PER_PASS = 32768;

#endif
  //static const unsigned int PHOTONS_PER_PASS = 32768;
  static const unsigned int NUM_PHOTONS= PHOTONS_PER_PASS * MAX_PHOTON_COUNT;
  Generator gen;
  WindowGenerator windowGen;
  clock_t creatingTree;
  clock_t notCreatingTree;
  clock_t afterKDTree;
  clock_t beforeKDTree;
  bool meshProvided;
  const Mesh* mesh;
  int index;
  float altitude;
  float distrFactor;
  float factor;
  float sunBrightness;
  bool returnPatches;
  bool& done;
  vector<float4>  patchVec;

  double north;
  double longitude;
  double latitude;

  int currentOrthoCamera;

  //std::vector<float3>& patchVals;


};


//const unsigned int SimpleScene::HEIGHT           = 1200;
//const unsigned int SimpleScene::PHOTONS_PER_PASS = 32768;
//const unsigned int SimpleScene::MAX_PHOTON_COUNT = 10;
//const unsigned int SimpleScene::NUM_PHOTONS      = PHOTONS_PER_PASS * MAX_PHOTON_COUNT;




