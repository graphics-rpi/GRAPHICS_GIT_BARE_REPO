#define NOMINMAX
#include<SimpleScene.hpp>
#include<Photon.hpp>
#include <GLUTDisplay.hpp>
#include <fstream>
#include <ScreenReader.h>
#include <OrthoParser.hpp>
#include <sun_angles.h>
#include <fstream>
#include <stdio.h>
#include "PinholeCamera.hpp"


//#include "test.hpp"
int  momentMonth[3] = { 21,21,21};
int  momentDay[3]   = { 12,3,6};
int  momentHours[9] = { 9,  10, 11, 12, 13, 14, 15, 16, 17};
int  momentMins[9]  = { 0,  0,  0,  0,  0,  0,  0,  0,  0};


SimpleScene::SimpleScene(const Mesh* meshIn,State & stateParam, int inIndex,bool& doneParam)
  : mFrameCount(0),_quick_render(state.debugMode),_eye_pass_init(0), _importon_pass_init(0),meshProvided(true) , index(inIndex), state(stateParam), mesh(meshIn), sunBrightness(0), done(doneParam)
  {
  currentMoment=0;
  numFramesPerMoment=-1;
  returnPatches=false;
  currentOrthoCamera=0;
  }

 void SimpleScene::loadObj(SampleScene::InitialCameraData& cameraData) {
  printf("lobj 0\n");
  //Initialize timing variables
  creatingTree=0;
  notCreatingTree=0;
  afterKDTree=clock();
  beforeKDTree=clock();

  //Optix Bounding box
  optix::Aabb aabb;

  //Do we need this here?
  optix::GeometryGroup geometryGroup = _context->createGeometryGroup();
  printf("lobj 1\n");
  //Loads the Mesh from the mesh passed in from the remesher
  printf("North Tossed into Mesh Loader: %f", north);
  loader = new MeshLoader(_context, geometryGroup, mCentroids,mNormals,
                          mPatchAreas,mPatchStartIndexes, mPatchSizes,mTriAreas,mNeighbors, mNeighborWeights,mesh, north, longitude, latitude);


    printf("lobj 2\n");

  //Sets all the material shaders
  createMaterial();
    printf("lobj 3\n");
      printf("lobj 3.1\n");
  loader->load(mMaterial);
    printf("lobj 3.5\n");
  aabb = loader->getSceneBBox();
  printf("lobj 4\n");
  // Set camera
  float  maxDim = fmaxf(aabb.extent(0), aabb.extent(1));
  float  eps    = 1e-4f * maxDim;
  float3 eye    = aabb.center();
  eye.z += 2 * maxDim;
  cameraData = SampleScene::InitialCameraData(eye, aabb.center(), make_float3(0, 1, 0), 30);
  printf("lobj 5\n");
  printf("bounding box extent %f %f %f\n", aabb.extent(0), aabb.extent(1),length(aabb.extent()));

  //Need to initialize all variables used in optix shaders (values set later just making sure they exist here)
  _context[ "numPatches"      ]->setInt(mesh->numPatches());
  _context[ "topObject"       ]->set(geometryGroup);
  _context[ "topShadower"     ]->set(geometryGroup);
  _context[ "eye"             ]->setFloat(make_float3(0, 0, 0));
  _context[ "bg_color"        ]->setFloat(make_float3(0, 0, 1));
  _context[ "bad_color"       ]->setFloat(make_float4(1, 0, 0,0));
  _context[ "U"               ]->setFloat(make_float3(0, 0, 0));
  _context[ "V"               ]->setFloat(make_float3(0, 0, 0));
  _context[ "W"               ]->setFloat(make_float3(0, 0, 0));
  _context[ "sceneEpsilon"    ]->setFloat(eps);
  _context[ "sceneCenter"     ]->setFloat(aabb.center());
  _context[ "sceneMaxDim"     ]->setFloat(length(aabb.extent()));
  _context[ "skyType"         ]->setInt(index);
  _context[ "exPer"           ]->setInt(state.exposurePercent);
  _context[ "cameraType"           ]->setInt(state.cameraType);
  _context[ "factor"          ]->setFloat(0);
  _context[ "sunBrightness"   ]->setFloat(0);
  _context[ "sunDirection"    ]->setFloat(make_float3(0));
 _context[ "NUM_MOMENTS"      ]->setInt(NUM_MOMENTS);

  //Copies the window triangle from the remesher structure to the structure for use in OptiX
  for(unsigned int i = 0; i < loader->mClearTriangles.size(); ++i) {
    // Get a normal
    // Get area of triangle
    std::vector<float3>& tri = loader->mClearTriangles[i];
    float3 a  = tri[0];
    float3 b  = tri[1];
    float3 c  = tri[2];
    float3 ab = b - a;
    float3 ac = c - a;
    float3 x  = cross(ab, ac);
    float  A  = length(x) / 2;
    float3 n  = -normalize(x);  // TODO: Flip this correctly.

    // Store
    ClearWindowInfo nfo;
    nfo.a = a;
    nfo.b = b;
    nfo.c = c;
    mClearWindowInfos.push_back(nfo);

    ClearWindowLaunchInfo info;
    info.area   = A;
    info.normal = n;
    info.eps    = eps;
    info.windowIndex = mClearWindowLaunchInfos.size();
    mClearWindowLaunchInfos.push_back(info);
  }

	printf("lobj 6\n");

  //Copies emissive (area lights) triangles so OptiX can use them
  for(unsigned int i = 0; i < loader->mEmmissiveTriangles.size(); ++i) {
    // Get a normal
    // Get area of triangle

		printf("lobj 7\n");
    std::vector<float3>& tri = loader->mEmmissiveTriangles[i];
    float3 a  = tri[0];
    float3 b  = tri[1];
    float3 c  = tri[2];
    float3 ab = b - a;
    float3 ac = c - a;
    float3 x  = cross(ab, ac);

    float  A  = length(x) / 2;
    float3 n  = -normalize(x);  // TODO: Flip this correctly.

    // Store
    ClearWindowInfo nfo;
    nfo.a = a;
    nfo.b = b;
    nfo.c = c;
    mAreaLightInfos.push_back(nfo);

    AreaLightLaunchInfo info;

    info.area   = A;
		cout<<"here2"<<endl;
    info.normal = n;
    info.eps    = eps;
    info.windowIndex = mAreaLightLaunchInfos.size();
    mAreaLightLaunchInfos.push_back(info);
  }
}

//debug
// #include <unistd.h>
#include <iostream>
// #define GetCurrentDir getcwd

 void SimpleScene::initRayPrograms()
{

  // Ray types
  //Declares all the types of rays used
  _context->setRayTypeCount(9);
  _context[ "importonType"      ]->setUint(0);
  _context[ "shadowRayType"     ]->setUint(1);
  _context[ "windowRayType"     ]->setUint(2);
  _context[ "lightRayType"      ]->setUint(3);
  _context[ "skyRayType"        ]->setUint(4);
  _context[ "photonRayType"     ]->setUint(5);
  _context[ "quickRayType"      ]->setUint(6);
  _context[ "quickShadowRayType"]->setUint(7);
  _context[ "eyeRayType"        ]->setUint(8);

  // Pass types (ray generation programs)
  _context->setEntryPointCount(13);
  _context[ "importonPass"      ]->setUint(0);
  _context[ "importonPass1D"    ]->setUint(1);
  _context[ "eyeRayPass"        ]->setUint(2);
  _context[ "eyeRayPass1D"      ]->setUint(3);
  _context[ "windowPass"        ]->setUint(4);
  _context[ "areaLightPass"     ]->setUint(5);
  _context[ "photonPass"        ]->setUint(6);
  _context[ "gatherPass"        ]->setUint(7);
  _context[ "regatherPass"      ]->setUint(8);
  _context[ "patchGatherPass"   ]->setUint(9);
  _context[ "quickPass"         ]->setUint(10);
  _context[ "momentTransferPass"]->setUint(11);
  _context[ "momentGatherPass"  ]->setUint(12);

  //Paths of ray generation shaders
  std::string importonPassPath   = "importonPass.cu.ptx";
  std::string eyeRayPassPath     = "eyeRayPass.cu.ptx";
  std::string photonPassPath     = "photonPass.cu.ptx";
  std::string gatherPassPath     = "gatherPass.cu.ptx";
  std::string quickPassPath      = "pinhole_camera.cu.ptx";
  std::string normalPath         = "normal_shader.cu.ptx";
  std::string regatherPath       = "regatherPass.cu.ptx";
  std::string momentTransferPath = "momentTransferPass.cu.ptx";


	// // debug
  // char buff[FILENAME_MAX];
  // GetCurrentDir( buff, FILENAME_MAX );
  // std::string current_working_dir(buff);
  // cout<<current_working_dir<<endl;


  // Ray generation program
  Program importonGenProgram      = _context->createProgramFromPTXFile("importonPass.cu.ptx", "importonPassCamera");
  Program importonGen1DProgram    = _context->createProgramFromPTXFile(importonPassPath, "importonPassCamera1D");
  Program eyeRayGenProgram        = _context->createProgramFromPTXFile(eyeRayPassPath, "eyePassCamera");
  Program eyeRayGen1DProgram      = _context->createProgramFromPTXFile(eyeRayPassPath, "eyePassCamera1D");
  Program photonGenProgram        = _context->createProgramFromPTXFile(photonPassPath, "photonPassGenerator");
  Program windowGenProgram        = _context->createProgramFromPTXFile(photonPassPath, "windowPassGenerator");
  Program lightGenProgram         = _context->createProgramFromPTXFile(photonPassPath, "areaLightPassGenerator");
  Program gatherGenProgram        = _context->createProgramFromPTXFile(gatherPassPath, "gatherPass");
  Program quickGenProgram         = _context->createProgramFromPTXFile(quickPassPath, "pinhole_camera");
  Program regatherGenProgram      = _context->createProgramFromPTXFile(regatherPath, "regatherPass");
  Program patchGatherGenProgram   = _context->createProgramFromPTXFile(regatherPath, "patchGatherPass");
  Program MomentTranferProgram    = _context->createProgramFromPTXFile(momentTransferPath, "momentTransferPass");
  Program MomentGatherProgram     = _context->createProgramFromPTXFile(regatherPath, "momentGatherPass");

  //Miss programs
  Program importonMiss                 = _context->createProgramFromPTXFile(importonPassPath, "importonPassMiss");
  Program skyMiss                 = _context->createProgramFromPTXFile(photonPassPath, "skyPassMiss");
  Program normalMiss              = _context->createProgramFromPTXFile(normalPath, "miss");
    Program eyeRayMiss              = _context->createProgramFromPTXFile(eyeRayPassPath, "eyePassMiss");

  //Loading programs into OptiX context
  _context->setRayGenerationProgram(_context["importonPass"       ]->getUint(), importonGenProgram);
  _context->setRayGenerationProgram(_context["importonPass1D"     ]->getUint(), importonGen1DProgram);
  _context->setRayGenerationProgram(_context["eyeRayPass"         ]->getUint(), eyeRayGenProgram);
  _context->setRayGenerationProgram(_context["eyeRayPass1D"       ]->getUint(), eyeRayGen1DProgram);
  _context->setRayGenerationProgram(_context["photonPass"         ]->getUint(), photonGenProgram);
  _context->setRayGenerationProgram(_context["windowPass"         ]->getUint(), windowGenProgram);
  _context->setRayGenerationProgram(_context["areaLightPass"      ]->getUint(), lightGenProgram);
  _context->setRayGenerationProgram(_context["gatherPass"         ]->getUint(), gatherGenProgram);
  _context->setRayGenerationProgram(_context["quickPass"          ]->getUint(), quickGenProgram);
  _context->setRayGenerationProgram(_context["regatherPass"       ]->getUint(), regatherGenProgram);
  _context->setRayGenerationProgram(_context["patchGatherPass"    ]->getUint(), patchGatherGenProgram);
  _context->setRayGenerationProgram(_context["momentTransferPass" ]->getUint(), MomentTranferProgram);
  _context->setRayGenerationProgram(_context["momentGatherPass"   ]->getUint(), MomentGatherProgram);
  _context->setMissProgram(_context["importonType"                ]->getUint(), importonMiss);
  _context->setMissProgram(_context["skyRayType"   ]->getUint(), skyMiss);
  _context->setMissProgram(_context["quickRayType"   ]->getUint(), normalMiss);
    _context->setMissProgram(_context["eyeRayType"   ]->getUint(), eyeRayMiss);

 // // Exception program
 // //  _context->setExceptionProgram( 0, exceptionProgram) ;
 // //

}

void SimpleScene::momentInit()
{
  printf("in moment init\n");
  sleep(5);
  state.month=momentMonth[0];
  state.day=momentDay[0];
  state.hour=momentHours[0];
  state.minute=momentMins[0];
  //printf("shouldn't be here yet\n");
  //sleep(10);
  state.renderRes=TRIANGLES;
  MomentBuffer = _context->createBuffer(RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT,mCentroids.size()*NUM_MOMENTS);
  _context[ "MomentBuffer"    ]->set(MomentBuffer);
  currentMoment=0;
  _context[ "curMoment"        ]->setUint(currentMoment);
  numFramesPerMoment=10;
  _context[ "numFramesPerMoment"        ]->setUint(numFramesPerMoment);
}


bool SimpleScene::momentIterator()
{
  ++mFrameCount;
  if(mFrameCount==numFramesPerMoment)
  {
    //Do a transfer pass

    _context->launch(_context["momentTransferPass"]->getUint(), loader->getNumberOfTriangles(),1 );
    //Returns true if we are done calculating moments
    if(++currentMoment>=NUM_MOMENTS)
    {
      state.renderRes=TRIANGLE_MOMENTS;
      return true;
    }

    else
    {
      int dateIndex=currentMoment/3;
      int timeIndex=currentMoment%9;
      state.month=momentMonth[dateIndex];
      state.day=momentDay[dateIndex];
      state.hour=momentHours[timeIndex];
      state.minute=momentMins[timeIndex];
      mFrameCount=0;
      _eye_pass_init=false;
      _importon_pass_init=false;
    }
  }
  _context[ "curMoment"        ]->setUint(currentMoment);

  return false;



}

void SimpleScene::setOrtho(string filename)
{
	printf("hajksdf\n");
    _context[ "useOrthoCamera"   ]->setUint(1);
    printf("forcing to hybrid rendering for ortho mode\n");
    state.renderRes=HYBRID;
    OrthoParser oParser(filename);


    _context[ "nearPoint1"         ]->setFloat(oParser.nearPoint1);
    _context[ "nearPoint2"         ]->setFloat(oParser.nearPoint2);
    _context[ "nearPoint3"         ]->setFloat(oParser.nearPoint3);
    _context[ "nearPoint4"         ]->setFloat(oParser.nearPoint4);
    //width height?
    float3 cameraDir=oParser.cameraDir;
    _context[ "cameraDir"             ]->setFloat(cameraDir);
}

//View number is for the 6 views of a cube map.  0 is the default, 1 is right/left, 2 is reversed
//                                               3 is left/right , 4 is up, 5 is down,
void SimpleScene::setPersonCamera(Person person, int viewNumber)
{
    _context[ "useOrthoCamera"   ]->setUint(0);
    printf("forcing to hybrid rendering for people mode\n");
    state.renderRes=HYBRID;

    float3 camera_position=make_float3(person.a1,person.b2, person.a2);
    float3 standard_camera_dir=normalize(make_float3(-cos(person.b1), 0, -sin(person.b1)));
    float3 standard_camera_lookat=camera_position+standard_camera_dir;
    float3 world_up=make_float3(0,1,0);
    float3 ortho_to_up_and_lookat_dir=normalize(cross(standard_camera_dir, world_up));
    float3 up;
    float3 lookat;
    if(viewNumber<4)
      up=world_up;
    else
      up=standard_camera_dir;
    //A switch statement to set the lookat for different cameras
    switch (viewNumber)
    {
      case 0:
        lookat=camera_position+standard_camera_dir;
        break;
      case 1:
        lookat=camera_position+ortho_to_up_and_lookat_dir;
        break;
      case 2:
        lookat=camera_position-standard_camera_dir;
        break;
      case 3:
        lookat=camera_position-ortho_to_up_and_lookat_dir;
        break;
      case 4:
        lookat=camera_position+world_up;
        break;
      case 5:
        lookat=camera_position-world_up;
        break;
    }
    //Create a pinhole camera with specified stuff from ".wall" file
    PinholeCamera pc=PinholeCamera(
            camera_position,
            lookat, //Lookat
            up,//up
            state.viewAngle,state.viewAngle,
            PinholeCamera::KeepVertical );

    float3 eye, U, V,W;
    //Get the camera parameters for optix from pinhole camera class
    pc.getEyeUVW(eye,  U,  V, W);
    U=normalize(U);
    V=normalize(V);
    W=normalize(W);

    printf("eye %f %f %f \n", eye.x, eye.y, eye.z);
    printf("U   %f %f %f \n", U.x, U.y, U.z);
    printf("V   %f %f %f \n", V.x, V.y, V.z);
    printf("W   %f %f %f \n", W.x, W.y, W.z);
    float3 negneg =(-U-V+W);
    float3 posneg =(U-V+W);
    float3 pospos =(U+V+W);
    float3 negpos =(-U+V+W);
    printf("-U-V   %f %f %f \n", negneg.x, negneg.y, negneg.z);
    printf("+U-V   %f %f %f \n", posneg.x, posneg.y, posneg.z);
    printf("+U+V   %f %f %f \n", pospos.x, pospos.y, pospos.z);
    printf("-U+V   %f %f %f \n", negpos.x, negpos.y, negpos.z);

    //Set the camera parameters in Optix (need to make sure they are not overridden in trace
    _context[ "eye" ]->setFloat(eye);
    _context[  "U"  ]->setFloat(U);
    _context[  "V"  ]->setFloat(V);
    _context[  "W"  ]->setFloat(W);
}

void SimpleScene::initScene(InitialCameraData& cameraData) {
  printf("IS0\n");
  if(state.useOrthoCamera)
  {
    state.renderRes=HYBRID;
  }
  _context[ "res" ]->setInt(state.renderRes);
  _context[ "bounce" ]->setInt(state.bounceType);
  _context[ "viewpoint" ]->setInt(state.viewpoint);
  _context[ "screen" ]->setInt(state.screen);
  _context[ "greyscale" ]->setInt(state.greyscale);
  _context[ "toodim" ]->setFloat(state.toodim);
  _context[ "toobright" ]->setFloat(state.toobright);
  _context->setPrintEnabled(true);
  _context->setStackSize(1000);
  printf("IS1\n");
	initRayPrograms();

  printf("IS2\n");

  // Buffers
  Buffer eyeHitBuffer = _context->createBuffer(RT_BUFFER_OUTPUT);
  eyeHitBuffer->setFormat(RT_FORMAT_USER);
  eyeHitBuffer->setElementSize(sizeof(HitRecord));
  eyeHitBuffer->setSize(WIDTH, HEIGHT);

  Buffer directBuffer = _context->createBuffer(RT_BUFFER_OUTPUT);
  directBuffer->setFormat(RT_FORMAT_FLOAT3);
  directBuffer->setSize(WIDTH, HEIGHT);

  mSkyPhotons = _context->createBuffer(RT_BUFFER_OUTPUT);
  mSkyPhotons->setFormat(RT_FORMAT_USER);
  mSkyPhotons->setElementSize(sizeof(PhotonRecord));
  mSkyPhotons->setSize(NUM_PHOTONS);

  mAreaLightPhotons = _context->createBuffer(RT_BUFFER_OUTPUT);
  mAreaLightPhotons->setFormat(RT_FORMAT_USER);
  mAreaLightPhotons->setElementSize(sizeof(PhotonRecord));
  mAreaLightPhotons->setSize(NUM_PHOTONS);
  printf("IS3\n");
  mRandBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT2, PHOTONS_PER_PASS);
  uint2* randData = static_cast<uint2*>(mRandBuffer->map());
  for(unsigned int i = 0; i < PHOTONS_PER_PASS; ++i) {
    randData[i] = make_uint2(gen(), gen());
  }
  mRandBuffer->unmap();

  mWindowLaunchBuffer = _context->createBuffer(RT_BUFFER_INPUT);
  mWindowLaunchBuffer->setFormat(RT_FORMAT_USER);
  mWindowLaunchBuffer->setElementSize(sizeof(ClearWindowLaunchInfo));
  mWindowLaunchBuffer->setSize(PHOTONS_PER_PASS);

  mAreaLightLaunchBuffer = _context->createBuffer(RT_BUFFER_INPUT);
  mAreaLightLaunchBuffer->setFormat(RT_FORMAT_USER);
  mAreaLightLaunchBuffer->setElementSize(sizeof(AreaLightLaunchInfo));
  mAreaLightLaunchBuffer->setSize(PHOTONS_PER_PASS);

  mPhotonMapBuffer = _context->createBuffer(RT_BUFFER_INPUT);
  mPhotonMapBuffer->setFormat(RT_FORMAT_USER);
  mPhotonMapBuffer->setElementSize(sizeof(PhotonRecord));
  mPhotonMapBuffer->setSize(NUM_PHOTONS);


  _context[ "eyeHitBuffer"          ]->set(eyeHitBuffer);
  _context[ "skyPhotonBuffer"       ]->set(mSkyPhotons);
  _context[ "areaLightPhotonBuffer" ]->set(mAreaLightPhotons);
  _context[ "photonMapBuffer"       ]->set(mPhotonMapBuffer);
  _context[ "randBuffer"            ]->set(mRandBuffer);
  _context[ "windowBuffer"          ]->set(mWindowLaunchBuffer);
  _context[ "areaLightBuffer"       ]->set(mAreaLightLaunchBuffer);
  _context[ "directBuffer"       ]->set(directBuffer);
  _context[ "outputBuffer"          ]->set(createOutputBuffer( RT_FORMAT_FLOAT4, WIDTH, HEIGHT));

  float3 sunDirection; // = normalize(make_float3(-0.5, -2, -1));

  //40.783 , -73.967
  // float lat= 40.783 * M_PI/180.;
  // float lon= -73.967 * M_PI/180.;
  float lon = longitude * M_PI/180.;
  float lat = latitude * M_PI/180.;
  int dayOfYear=1;
  float timeOfDay=16;
  printf("north in init scene: %f",north);
  printf("lon lat in init scene: %f %f",lon,lat);
  calcSunPosition(sunDirection, lat,lon,  dayOfYear,  timeOfDay, altitude, north);

  float  sunZenith    = acos(-sunDirection.y);
  float  sunAzimuth   = atan2(-sunDirection.x, -sunDirection.z);
  float2 sunAngles    = make_float2(sunZenith, sunAzimuth);
  printf("IS4\n");
  // Lights
  DirectionalLight dirLights[] = {
    { sunDirection, make_float3(0.75f, 0.746f, 0.676f), 1 },
  };
  dirLightBuffer = _context->createBuffer(RT_BUFFER_INPUT);
  dirLightBuffer->setFormat(RT_FORMAT_USER);
  dirLightBuffer->setElementSize(sizeof(DirectionalLight));
  dirLightBuffer->setSize(sizeof(dirLights) / sizeof(dirLights[0]));
  memcpy(dirLightBuffer->map(), dirLights, sizeof(dirLights));
  dirLightBuffer->unmap();

  PointLight lights[] = {
     { make_float3(-500, 500, 0), make_float3(0.7f, 0.7f, 0.65f), 300, 1, 1 },
     { make_float3(1900, 500, 0), make_float3(0.7f, 0.7f, 0.65f), 300, 1, 1 },
  };
  Buffer lightBuffer = _context->createBuffer(RT_BUFFER_INPUT);
  lightBuffer->setFormat(RT_FORMAT_USER);
  lightBuffer->setElementSize(sizeof(PointLight));
  lightBuffer->setSize(sizeof(lights) / sizeof(lights[0]));
  memcpy(lightBuffer->map(), lights, sizeof(lights));
  lightBuffer->unmap();
  _context[ "lightBuffer"       ]->set(lightBuffer);
  _context[ "dirLightBuffer"    ]->set(dirLightBuffer);
  printf("IS5\n");

  // Random other variables
  _context[ "sunAngles"         ]->setFloat(sunAngles);
  _context[ "skyType"           ]->setInt(state.skyType);
  _context[ "maxPhotonCount"    ]->setUint(MAX_PHOTON_COUNT);
  _context[ "maxDepth"          ]->setUint(state.bounce);//MAX_PHOTON_COUNT);
  _context[ "totalPhotonsFired" ]->setUint(0);
  _context[ "photonsPerPass"    ]->setUint(PHOTONS_PER_PASS);
  _context[ "normalInversion"   ]->setInt(-1);
  _context[ "backfaceCulling"   ]->setUint(1);
  _context[ "windowCorrection"  ]->setFloat(1);
  // Load model
  loadObj(cameraData);

  _context[ "numTriangles"  ]->setUint(loader->getNumberOfTriangles());
  if(state.useOrthoCamera && state.orthoFiles.size()>0)
  {
		printf("%s\n", state.orthoFiles[0]);
    setOrtho(state.orthoFiles[0]);
  }
  else
  {
    _context[ "cameraDir"             ]->setFloat(make_float3( 0.f, 0.f, 0.f ));
    _context[ "useOrthoCamera"   ]->setUint(0);
    _context[ "nearPoint1"         ]->setFloat(make_float3( 0.f, 0.f, 0.f ));
    _context[ "nearPoint2"         ]->setFloat(make_float3( 0.f, 0.f, 0.f ));
    _context[ "nearPoint3"         ]->setFloat(make_float3( 0.f, 0.f, 0.f ));
    _context[ "nearPoint4"         ]->setFloat(make_float3( 0.f, 0.f, 0.f ));
  }
  MomentBuffer = _context->createBuffer(RT_BUFFER_OUTPUT,RT_FORMAT_FLOAT,0);
  _context[ "MomentBuffer"    ]->set(MomentBuffer);

  if(state.moment)
    momentInit();
  else
  {
    _context[ "curMoment"        ]->setUint(0);

    _context[ "numFramesPerMoment"        ]->setUint(0);

  }



  if(state.dumpPatches)
  {
    state.renderRes=PATCHES;
  }
  if(state.dumpTris)
  {
    state.renderRes=TRIANGLES;
  }
  copyBuffersToGPU();


  _context->validate();
  _context->compile();
  //gettimeofday(&start, NULL);
}

void SimpleScene::copyBuffersToGPU()
{
  Buffer centroidBuffer = _context->createBuffer(RT_BUFFER_INPUT,RT_FORMAT_FLOAT3,mCentroids.size());
  memcpy(centroidBuffer->map(), &mCentroids[0], sizeof(float3)*mCentroids.size());
  centroidBuffer->unmap();
  _context[ "centroidBuffer"    ]->set(centroidBuffer);

  Buffer normalBuffer = _context->createBuffer(RT_BUFFER_INPUT,RT_FORMAT_FLOAT3, mNormals.size());
  memcpy(normalBuffer->map(), &mNormals[0], sizeof(float3)*mNormals.size());
  normalBuffer->unmap();
  _context[ "normalBuffer"    ]->set(normalBuffer);

  Buffer patchAreaBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, mPatchAreas.size());
  memcpy(patchAreaBuffer->map(), &mPatchAreas[0], sizeof(float)*mPatchAreas.size());
  patchAreaBuffer->unmap();
  _context[ "patchAreaBuffer"    ]->set(patchAreaBuffer);

  Buffer patchStartIndexesBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT,mPatchStartIndexes.size());
  memcpy(patchStartIndexesBuffer->map(), &mPatchStartIndexes[0], sizeof(int)*mPatchStartIndexes.size());
  patchStartIndexesBuffer->unmap();
  _context[ "patchStartIndexesBuffer"    ]->set(patchStartIndexesBuffer);

  Buffer patchSizesBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT,mPatchSizes.size());
  memcpy(patchSizesBuffer->map(), &mPatchSizes[0], sizeof(int)*mPatchSizes.size());
  patchSizesBuffer->unmap();
  _context[ "patchSizesBuffer"    ]->set(patchSizesBuffer);

  Buffer triAreasBuffer = _context->createBuffer(RT_BUFFER_INPUT,RT_FORMAT_FLOAT,mTriAreas.size());
  memcpy(triAreasBuffer->map(), &mTriAreas[0], sizeof(float)*mTriAreas.size());
  triAreasBuffer->unmap();
  _context[ "triAreasBuffer"    ]->set(triAreasBuffer);

  Buffer neighborsBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT, mNeighbors.size());
  memcpy(neighborsBuffer->map(), &mNeighbors[0], sizeof(int)*mNeighbors.size());
  neighborsBuffer->unmap();
  _context[ "neighborsBuffer"    ]->set(neighborsBuffer);

  Buffer neighborWeightsBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT,mNeighborWeights.size());
  memcpy(neighborWeightsBuffer->map(), &mNeighborWeights[0], sizeof(float)*mNeighborWeights.size());
  neighborWeightsBuffer->unmap();
  _context[ "neighborWeightsBuffer"    ]->set(neighborWeightsBuffer);

  //Used for triangles
  intermediateBuffer = _context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, mCentroids.size());
  _context[ "intermediateBuffer"    ]->set(intermediateBuffer);



  patchValueBuffer = _context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, mesh->numPatches());
  _context[ "patchValueBuffer"    ]->set(patchValueBuffer);

  ScreenReader sr(state.screenFile);

  int width=sr.getWidth();
  int height=sr.getHeight();

  Buffer screenBuffer = _context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_BYTE, width*height);
  memcpy(screenBuffer->map(), sr.getImageBuf(), sizeof(char)*width*height);
  screenBuffer->unmap();

  _context[ "screenBuffer"    ]->set(screenBuffer);
  _context[ "screenWidth"     ]->setInt(width);
  _context[ "screenHeight"    ]->setInt(height);


}

 bool SimpleScene::activeDisplay()
 {
    if(_quick_render)
    {
      if(_camera_changed||_mode_changed)
        return true;
    }
    else
      return true;

    return false;
 }


void SimpleScene::fillPatchBuffer()
{
  int numPatches=mesh->numPatches();
  float4* patchData = static_cast<float4*>(patchValueBuffer->map());

  for(int i=0; i<numPatches; i++)
  {

    patchVec.push_back(patchData[i]);
  }
  patchValueBuffer->unmap();
}

void SimpleScene::dumpPatchBuffer()
{
  int numPatches=patchVec.size();
  if(numPatches>0)
  {
    std::ofstream patchFile;
    patchFile.open(state.patchesFile.c_str());
    for(int i=0; i<numPatches; i++)
    {

      float4 patch=(patchVec)[i];
      patchFile<<"patch["<<i<<"]: "<<patch.x<<" "<<patch.y<<" "<<patch.z<<std::endl;

    }
    patchFile.close();
  }
  else
  {
    printf("error no patches exist\n");
  }


}

void SimpleScene::dumpTriBuffer()
{
  std::ofstream out;
  printf("Writing %s \n", state.trisFile.c_str());
  out.open(state.trisFile.c_str());

  float4* iB = static_cast<float4*>(intermediateBuffer->map());
  int size= mCentroids.size();
  for(unsigned int i = 0; i < size; ++i) {
    float4 tri=iB[i];
     out<<"tri["<<i<<"]: "<<tri.x<<" "<<tri.y<<" "<<tri.z<<std::endl;
  }
  intermediateBuffer->unmap();
  out.close();
}

void SimpleScene::updateDate(float3& sunDirection)
{
  int day=jdate(state.month, state.day);
  // float lat=  40.783 * M_PI/180.;
  // float lon= -73.967 * M_PI/180.;

  float lon = longitude * M_PI/180.;
  float lat = latitude * M_PI/180.;

  float timeOfDay=state.hour+state.minute/60.;

  printf("north in update scene: %f",north);
  printf("lon lat in update scene: %f %f",lon,lat);
  calcSunPosition(sunDirection,  lat, lon, day, timeOfDay, altitude, north);
  //distrFactor=getSkyDistrFactor(sunDirection, altitude);
  sunBrightness=getSunNormalizedIllum(sunDirection);
  float  sunZenith    = acos(-sunDirection.y);
  float  sunAzimuth   = atan2(-sunDirection.x, -sunDirection.z);
  float2 sunAngles    = make_float2(sunZenith, sunAzimuth);

  float	turbidity = 2.75;
  float zenithBrightness = (1.376*turbidity-1.81)*tan(altitude)+0.38;

  float phiZ=1.;
  float fSun=1.;
if(state.skyType==CLEAR)
{
  const float a = -1.0;
  const float b = -0.32;
  const float c = 10;
  const float d = -3.0;
  const float e = 0.45;

  //phi ( PI/2)
  phiZ = 1 + a * exp(b / 1);//cos(skyPayload.angles.x));

  //float fChi = 1 + c * (exp(d * chi) - exp(d * M_PI / 2)) + e * cos(chi) * cos(chi);
  // float fSun = 1 + c * (exp(d * sunAngles.x) - exp(d * M_PI / 2)) + e * powf(cos(sunAngles.x), 2);
  fSun = .91 + c * exp(d * sunAngles.x) + e * powf(cos(sunAngles.x), 2);
  factor=zenithBrightness/(fSun*phiZ);
}
else if(state.skyType==TURBID)
{
  const float a = -1.0;
  const float b = -0.32;
  const float c = 16;
  const float d = -3.0;
  const float e = 0.3;
  phiZ = 1 + a * exp(b / 1);//cos(skyPayload.angles.x));
  fSun = .856 + c * exp(d * sunAngles.x) + e * powf(cos(sunAngles.x), 2);
  factor=zenithBrightness/(fSun*phiZ);
}
else if(state.skyType==INTERMEDIATE)
{
  float a=(1.35f*(sin(3.59*M_PI/2.f-.009)+2.31)*sin(2.6*(M_PI/2.f-sunZenith)+.316)+M_PI/2.f+4.799)/2.326;
  float b=-.563*((M_PI/2.f+1.059)*(M_PI/2.f-sunZenith-.008)+.812);
  factor=a*exp(sunZenith*b);
}
else
{
  phiZ=1.;
  fSun=1.;
  factor=zenithBrightness/(fSun*phiZ);
}

  //skyPayload.energy = make_float3(fChi * phiZ);
  //}

  // Lights
  DirectionalLight dirLights[] = {

    { sunDirection, make_float3(sunBrightness), 10 },
  };

  memcpy(dirLightBuffer->map(), dirLights, sizeof(dirLights));
  dirLightBuffer->unmap();
  _context[ "dirLightBuffer"    ]->set(dirLightBuffer);
  // Random other variables
  _context[ "sunAngles"         ]->setFloat(sunAngles);
  _context[ "skyType"           ]->setInt(state.skyType);
}

//When something in the scene changes so rendering parameters need to be reloaded
void SimpleScene::startOver(const RayGenCameraData& cameraData)
{

  if(state.useOrthoCamera)
  {
    state.renderRes=HYBRID;
  }
    printf("in start over\n");
    _context[ "factor" ]->setFloat(factor);


    state.changed=false;
//    _eye_pass_init=false;
//    _importon_pass_init=false;



    if(!state.dumpPeople)
    {

    printf("eye %f %f %f \n", cameraData.eye.x, cameraData.eye.y, cameraData.eye.z);
    printf("U   %f %f %f \n", cameraData.U.x, cameraData.U.y, cameraData.U.z);
    printf("V   %f %f %f \n", cameraData.V.x, cameraData.V.y, cameraData.V.z);
    printf("W   %f %f %f \n", cameraData.W.x, cameraData.W.y, cameraData.W.z);
      _context[ "eye" ]->setFloat(cameraData.eye);
      _context[  "U"  ]->setFloat(cameraData.U);
      _context[  "V"  ]->setFloat(cameraData.V);
      _context[  "W"  ]->setFloat(cameraData.W);
    }

    if(_importon_pass_init==false)
    {
     _context[ "totalPhotonsFired" ]->setUint(0);
        mFrameCount = 0;
      double runningTotal = 0;
      mWindowProbabilities.clear();
      for(unsigned int i = 0; i < mClearWindowInfos.size(); ++i)
      {
        runningTotal += mClearWindowLaunchInfos[i].area;
        mWindowProbabilities.push_back(runningTotal);
      }
      _context[ "windowCorrection"  ]->setFloat(runningTotal);

      runningTotal = 0;
      mAreaLightProbabilities.clear();
      for(unsigned int i = 0; i < mAreaLightInfos.size(); ++i) {
        runningTotal += mAreaLightLaunchInfos[i].area;
        mAreaLightProbabilities.push_back(runningTotal);
      }
    }
    printf("end start over\n");
}

void SimpleScene::traceEyePass( RTsize bufferWidth, RTsize bufferHeight)

{
  if(state.dumpImage)
  {
    state.renderRes=HYBRID;
  }
    printf("begin trace eyte pass \n");
    if(_quick_render==true&&(_mode_changed||_camera_changed) )
    {
      printf("before first launch\n");
      _context->launch(_context["quickPass"]->getUint(), static_cast<unsigned int>(bufferWidth),
        static_cast<unsigned int>(bufferHeight));
        printf("after first launch\n");
    }
    else if(_quick_render==true&&_frames_since_mod<200)

    {
      printf("idling\n");
      _frames_since_mod++;
    }
    else
    {
      if(_importon_pass_init==false)
      {
        if(state.renderRes==FULL)
          printf("buffer height %u buffer width %u \n", static_cast<unsigned int>(bufferHeight), static_cast<unsigned int>(bufferWidth));
          _context->launch(_context["importonPass"]->getUint(), static_cast<unsigned int>(bufferWidth),
            static_cast<unsigned int>(bufferHeight));

        if(state.renderRes==PATCHES||state.renderRes==TRIANGLES||
           state.renderRes==HYBRID||state.renderRes==TRIANGLE_NOINT)
        {
          _context->launch(_context["importonPass1D"]->getUint(),loader->getNumberOfTriangles(),static_cast<unsigned int>(1));
        }

      }
      if(_eye_pass_init==false)
      {

          if(state.renderRes==FULL)
          {
            _context->launch(_context["eyeRayPass"]->getUint(), static_cast<unsigned int>(bufferWidth),
            static_cast<unsigned int>(bufferHeight));


          }
          else if(state.renderRes==PATCHES||state.renderRes==TRIANGLES||state.renderRes==TRIANGLE_NOINT)
          {

            _context->launch(_context["eyeRayPass1D"]->getUint(),loader->getNumberOfTriangles(),1);
          }
          else //Hybrid
          {
            _context->launch(_context["eyeRayPass"]->getUint(), static_cast<unsigned int>(bufferWidth),
            static_cast<unsigned int>(bufferHeight));
          }



        _importon_pass_init=true;
        _eye_pass_init=true;

      }

    }
    printf("end trace eye pass\n");
}

void SimpleScene::trace(const RayGenCameraData& cameraData) {
  //printf("frame: %d \n", mFrameCount);
  bool almostdone=false;
  float3 sunDirection;
  updateDate(sunDirection);
  //printf("screen %d \n", state.screen);
  //sleep(1);
  //state.renderRes=FULL; 4.2
  //state.renderRes=TRIANGLES; 4.1
  //state.renderRes=PATCHES; 4.1
  //state.renderRes=HYBRID; 4.1
  _context[ "res"          ]->setInt(state.renderRes);
  _context[ "bounce"       ]->setInt(state.bounceType);
  _context[ "viewpoint"    ]->setInt(state.viewpoint);
  _context[ "screen"       ]->setInt(state.screen);
  _context[ "exPer"        ]->setInt(state.exposurePercent);
  _context[ "cameraType"        ]->setInt(state.cameraType);
  _context[ "sunBrightness"]->setFloat(sunBrightness);
  _context[ "sunDirection" ]->setFloat(sunDirection);
  _context[ "maxDepth"     ]->setUint(state.bounce);

  _quick_render=state.debugMode;
  if(_camera_changed==1)
  {
    _eye_pass_init=0;
    if(state.renderRes==FULL)
    {
      _importon_pass_init=0;
      _frames_since_mod=0;
    }
  }

  Buffer buffer = getOutputBuffer();
  RTsize bufferWidth, bufferHeight;
  buffer->getSize(bufferWidth, bufferHeight);

  //If the camera changed or if for some reason the
  //if(_camera_changed||(_eye_pass_init==0&&_quick_render==0)) {
  if(state.changed)
  {
    _importon_pass_init=0;
    _eye_pass_init=0;
  }
  if(_importon_pass_init==0||_eye_pass_init==0||state.changed)
    startOver(cameraData);

  if(_mode_changed||_camera_changed||!_eye_pass_init||state.changed)
  {
    printf("tracing eye pass\n");
    traceEyePass( bufferWidth, bufferHeight);

  }



  //Any time the camera, time, or rendering mode changes
  if(_eye_pass_init&&!(_quick_render&&(_mode_changed||_camera_changed)))
  {
    printf("eye pass inited\n");

    //Generate random numbers for use on the GPU
    uint2* rands = static_cast<uint2*>(mRandBuffer->map());
    for(unsigned int i = 0; i < PHOTONS_PER_PASS; ++i) {
      rands[i] = make_uint2(gen(), gen());
    }
    mRandBuffer->unmap();

    //Generate photon starting points
    generateWindowProbabilities();
    generateAreaLightProbabilities();

    if((state.useOrthoCamera==false&&state.dumpPeople==false)||mFrameCount<30)
    {
      printf("about to enter windowpass\n");
      //Launch the photon passes
      _context->launch(_context["windowPass"]->getUint(), PHOTONS_PER_PASS);
      _context->launch(_context["areaLightPass"]->getUint(), PHOTONS_PER_PASS);

      //Put photons in a KD tree
      createPhotonMap();
      _context["totalPhotonsFired"]->setUint(_context["totalPhotonsFired"]->getUint() + 1);
      printf("exitted window pass\n");

    }





    //If rendering for standard camera
    if(state.renderRes==TRIANGLE_MOMENTS)
    {
    printf("gather for trimoments\n");
      _context->launch(_context["momentGatherPass"]->getUint(), static_cast<unsigned int>(bufferWidth),
      static_cast<unsigned int>(bufferHeight));

    }
    else if(state.renderRes==FULL)
    {

        printf("gather for pixels\n");
        _context->launch(_context["gatherPass"]->getUint(),static_cast<unsigned int>(bufferWidth),
          static_cast<unsigned int>(bufferHeight) );


    }

    else //rendering for triangles or patches
    {
      //Need to gather for triangles on both patches and triangles
      if(state.renderRes==TRIANGLES)
      {
        printf("gather for triangles\n");
        _context->launch(_context["gatherPass"]->getUint(), loader->getNumberOfTriangles(),1 );
      }
      if(state.renderRes==PATCHES)
      {
        printf("launch patchGather\n");
        _context->launch(_context["patchGatherPass"]->getUint(), mesh->numPatches(),1 );
      }
      printf("regathering\n");
      _context->launch(_context["regatherPass"]->getUint(), static_cast<unsigned int>(bufferWidth),
          static_cast<unsigned int>(bufferHeight));

    }

  }

  if(state.moment)
  {
		printf("still allive\n" );
    //done=momentIterator();
    momentIterator();
  }
  else if(state.dumpImage)
  {
    printf("befre sutil diaply file\n");
    //If we are just dumping an image file (for contraption)
    if(mFrameCount==30) {
      sutilDisplayFilePPM(state.imageFile.c_str(), getOutputBuffer()->get());
      done=true;
      printf("just saved file\n");

    }
    printf("after display file\n");
  }
  else if(state.dumpPatches)
  {
		printf("still alliv2e\n" );
    //If we are just dumping an image file (for contraption)
    if(mFrameCount==30) {
       printf("before dumping patches\n");
       fillPatchBuffer();
      dumpPatchBuffer();
      done=true;
      printf("just dumped patches\n");

    }



  }
  else if(state.useOrthoCamera==true&&mFrameCount>=30)
  {
		printf("still alliv3e\n" );
      string cameraLoc=state.orthoFiles[currentOrthoCamera];

      //Strips the directory off of cameraLoc and adds .ppm extension to create ppmLoc
      int lastSlash=cameraLoc.find_last_of("/");
      if(lastSlash==cameraLoc.size())
        lastSlash=0;
      //string ppmLoc=cameraLoc.substr(lastSlash+1,cameraLoc.size()-(7+lastSlash))+"_texture.ppm";
      string ppmLoc=cameraLoc.substr(0,cameraLoc.size()-(6))+"_texture.ppm";
      printf("saving to %s \n", ppmLoc.c_str());
      sutilDisplayFilePPM(ppmLoc.c_str(), getOutputBuffer()->get());
			// cout<<getOutputBuffer()->getSize()<<endl;
      //If we just finished the last ortho camera
      if(++currentOrthoCamera==state.orthoFiles.size())
      {
        if(state.dumpPeople==true)
        {
          state.useOrthoCamera=false;
          state.currentPerson=0;
          setPersonCamera(state.people[state.currentPerson],0);
          state.cameraType=FISHEYECAMERA;
          //Reset camera stuff
          _eye_pass_init=0;

        }
        else
        done=true;
      }
      else
      {
        setOrtho(state.orthoFiles[currentOrthoCamera]);
        _eye_pass_init=0;
      }
  }
  else if(state.dumpTris)
  {
		printf("still al2live\n" );
    //If we are just dumping an image file (for contraption)
    if(mFrameCount==30) {
       printf("before dumping tris\n");

      dumpTriBuffer();
      done=true;
      printf("just dumped tris\n");

    }

  }
  else if(state.dumpPeople==true&&state.useOrthoCamera==false&&mFrameCount>=30)
  {
		printf("still allive7\n" );
    static int personCameraCounter=0;

    //Dump image
    char filename[50];
    printf("dumping person %d \n", state.currentPerson);
    //Default view for texture
    if(0==personCameraCounter)
      sprintf(filename,"surface_camera_VIEW_person%d.ppm",state.currentPerson);
//      sprintf(filename,"surface_camera_VIEW_person%d.ppm",state.currentPerson);
    else
      sprintf(filename,"surface_camera_VIEW_person%d_alternate%d.ppm",state.currentPerson, personCameraCounter);
    sutilDisplayFilePPM(filename,getOutputBuffer()->get());
    //6 Views for each person, if we have done all 5, go to the next person
    {
      personCameraCounter=0;
      //if we've already done all views for all people we are done
      if(++state.currentPerson==state.people.size())
        done=true;
    }
    //else set it to the next one
    if(!done)
    {
      setPersonCamera(state.people[state.currentPerson], personCameraCounter);
      //Reset camera stuff
      _eye_pass_init=0;
    }


  }
  printf("frame count %d \n", mFrameCount++);
  if(state.dumpPeople==true&&state.useOrthoCamera==false)
    printf("should be dumping people\n");
  _camera_changed = false;
  _mode_changed = false;
  if(almostdone)
  {
    fillPatchBuffer();
    //cleanUp();
    done=true;
#ifdef _WIN32
    //Sleep(1);
#else
    //sleep(100);
#endif
  }
}

void SimpleScene::generateWindowProbabilities()
{

    if(mWindowProbabilities.size() > 0) {
      ClearWindowLaunchInfo* windows = static_cast<ClearWindowLaunchInfo*>(mWindowLaunchBuffer->map());

      WindowDistribution real01;
      WindowVariateGenerator rand(windowGen, real01);

      WindowDistribution dist(0, mWindowProbabilities.back());
      WindowVariateGenerator roll(windowGen, dist);
      for(unsigned int i = 0; i < PHOTONS_PER_PASS; ++i) {
        //A complicated way of saying it picks a light based on the probability of that light
        unsigned int index = std::lower_bound(mWindowProbabilities.begin(), mWindowProbabilities.end(), roll()) - mWindowProbabilities.begin();
        const ClearWindowInfo& t = mClearWindowInfos[index];
        ClearWindowLaunchInfo info = mClearWindowLaunchInfos[index];
        //a and b are two random floats.  Sum should average to 1
        double a = rand();
        double b = rand();

        //Makes it so the sum of a and b is never greater than 1.
        //If XYZ is a triangle.  think of a as XY-> and b as XZ->
        //If a+b>1 then flip it back over YZ into the triangle
        if(a + b > 1) {
          a = 1 - a;
          b = 1 - b;
        }
        float3 centroid=(t.a+t.b+t.c)/3.;
        AXIS axis=X;
        float distx=(centroid.x-t.a.x)*(centroid.x-t.a.x)+
                    (centroid.x-t.b.x)*(centroid.x-t.b.x)+
                    (centroid.x-t.c.x)*(centroid.x-t.c.x);
        float disty=(centroid.y-t.a.y)*(centroid.y-t.a.y)+
                    (centroid.y-t.b.y)*(centroid.y-t.b.y)+
                    (centroid.y-t.c.y)*(centroid.y-t.c.y);
        float distz=(centroid.z-t.a.z)*(centroid.z-t.a.z)+
                    (centroid.z-t.b.z)*(centroid.z-t.b.z)+
                    (centroid.z-t.c.z)*(centroid.z-t.c.z);
        float mindist=distx;
        axis=X;
        if (disty<mindist)
        {
            mindist=disty;
            axis=Y;
        }
        if (distz<mindist)
        {
            mindist=distz;
            axis=Z;
        }
        //printf("axis %d \n", axis);
        info.axis=axis;
        info.start = t.a * a + t.b * b + t.c * (1 - a - b) //random point on triangle
             - 2 * info.eps * info.normal;                 //Slightly offsetting the triangle
        windows[i] = info;
      }//endfor

      mWindowLaunchBuffer->unmap();

    }//end if(mWindowProbabilities.size() > 0)

}
void SimpleScene::generateAreaLightProbabilities()
{
  if(mAreaLightProbabilities.size() > 0) {
      AreaLightLaunchInfo* areaLights = static_cast<AreaLightLaunchInfo*>(mAreaLightLaunchBuffer->map());

      WindowDistribution real01;
      WindowVariateGenerator rand(windowGen, real01);

      WindowDistribution dist(0, mAreaLightProbabilities.back());
      WindowVariateGenerator roll(windowGen, dist);
      for(unsigned int i = 0; i < PHOTONS_PER_PASS; ++i) {
        unsigned int index = std::lower_bound(mAreaLightProbabilities.begin(), mAreaLightProbabilities.end(),
            roll()) - mAreaLightProbabilities.begin();
        const ClearWindowInfo& t = mAreaLightInfos[index];
        AreaLightLaunchInfo info = mAreaLightLaunchInfos[index];
        double a = rand();
        double b = rand();
        if(a + b > 1)
        {
          a = 1 - a;
          b = 1 - b;
        }
        info.start = t.a * a + t.b * b + t.c * (1 - a - b) - 2 * info.eps * info.normal;
        areaLights[i] = info;
      }
      mAreaLightLaunchBuffer->unmap();

    }
}



void SimpleScene::createPhotonMap() {
  PhotonRecord* skyPhotons       = static_cast<PhotonRecord*>(mSkyPhotons->map());
  PhotonRecord* areaLightPhotons = static_cast<PhotonRecord*>(mAreaLightPhotons->map());
  PhotonRecord* photonMapData    = static_cast<PhotonRecord*>(mPhotonMapBuffer->map());

  float3 bboxMin, bboxMax;

  unsigned int count = 0;
  PhotonRecord** tempPhotons = new PhotonRecord*[2 * NUM_PHOTONS];
  for(unsigned int i = 0; i < NUM_PHOTONS; ++i) {
    if(fmaxf(skyPhotons[i].energy) > 0) {
      if(count == 0)
      {
        bboxMin = bboxMax = skyPhotons[i].position;
      }
      else
      {
        const float3 position = skyPhotons[i].position;
        bboxMin = fminf(bboxMin, position);
        bboxMax = fmaxf(bboxMax, position);
      }

      tempPhotons[count++] = &skyPhotons[i];
    }
  }

  for(unsigned int i = 0; i < NUM_PHOTONS; ++i)
  {
    if(fmaxf(areaLightPhotons[i].energy) > 0)
    {
      if(count == 0)
      {
        bboxMin = bboxMax = areaLightPhotons[i].position;
      }
      else
      {
        const float3 position = areaLightPhotons[i].position;
        bboxMin = fminf(bboxMin, position);
        bboxMax = fmaxf(bboxMax, position);
      }

      tempPhotons[count++] = &areaLightPhotons[i];
    }
  }


  beforeKDTree=clock();
  notCreatingTree+=(beforeKDTree-afterKDTree);

  buildKDTree(tempPhotons, 0, count, 0, photonMapData, 0, bboxMin, bboxMax);

  afterKDTree=clock();
  creatingTree+=(afterKDTree-beforeKDTree);


  delete[] tempPhotons;
  mPhotonMapBuffer->unmap();
  mAreaLightPhotons->unmap();
  mSkyPhotons->unmap();
}

int maxComponent(const float3& v)
{
  if(v.x > v.y)
    if(v.x > v.z)
      return 0;
    else
      return 2;
  else
    if(v.y > v.z)
      return 1;
    else
      return 2;
}

void
SimpleScene::buildKDTree(
  PhotonRecord** photons,
  const int start, const int end,
  unsigned int depth,
  PhotonRecord* kdTree, unsigned int root,
  const float3& bboxMin, const float3& bboxMax)
{
  if(end - start == 0)
  {
    kdTree[root].axis   = PhotonRecord::NILL;
    kdTree[root].energy = make_float3(0);
    return;
  }

  if(end - start == 1)
  {
    photons[start]->axis = PhotonRecord::LEAF;
    kdTree[root] = *photons[start];
    return;
  }

  const float3 diagonal = bboxMax - bboxMin;
  const int axis = maxComponent(diagonal);
  const int median = (start + end) / 2;

  switch(axis)
  {
  case 0:
    std::sort(&photons[start], &photons[end], ComparePhotons<0>());
    // select(photons, start, end, median, ComparePhotons<0>());
    break;
  case 1:
    std::sort(&photons[start], &photons[end], ComparePhotons<1>());
    // select(photons, start, end, median, ComparePhotons<1>());
    break;
  case 2:
    std::sort(&photons[start], &photons[end], ComparePhotons<2>());
    // select(photons, start, end, median, ComparePhotons<2>());
    break;
  }

  float3 rightMin = bboxMin;
  float3 leftMax  = bboxMax;
  float3 midPoint = photons[median]->position;

  switch(axis)
  {
  case 0:
    photons[median]->axis = PhotonRecord::X;
    rightMin.x = leftMax.x = midPoint.x;
    break;
  case 1:
    photons[median]->axis = PhotonRecord::Y;
    rightMin.y = leftMax.y = midPoint.y;
    break;
  case 2:
    photons[median]->axis = PhotonRecord::Z;
    rightMin.z = leftMax.z = midPoint.z;
    break;
  }

  kdTree[root] = *photons[median];
  buildKDTree(photons, start,   median, depth + 1, kdTree, 2 * root + 1, bboxMin,  leftMax);
  buildKDTree(photons, median + 1, end, depth + 1, kdTree, 2 * root + 2, rightMin, bboxMax);
}

optix::Buffer SimpleScene::getOutputBuffer()
{
  return _context["outputBuffer"]->getBuffer();
}

void SimpleScene::cleanUp() {SampleScene::cleanUp();}

void SimpleScene::toggleDetail()
{
   _quick_render=!_quick_render;
   _mode_changed=true;
}

void SimpleScene::createMaterial()
{
  mMaterial = _context->createMaterial();

  std::string importonPassPath  = "importonPass.cu.ptx";
  std::string photonPassPath    = "photonPass.cu.ptx";
  std::string normalShaderPath  = "normal_shader.cu.ptx";
  std::string eyeRayPassPath    = "eyeRayPass.cu.ptx";

  Program importonPassAnyHit     = _context->createProgramFromPTXFile(importonPassPath, "importonPassAnyHit");
  Program importonPassClosestHit = _context->createProgramFromPTXFile(importonPassPath, "importonPassClosestHit");
  Program eyePassAnyHit     = _context->createProgramFromPTXFile(eyeRayPassPath, "eyeRayPassAnyHit");
  Program eyePassClosestHit = _context->createProgramFromPTXFile(eyeRayPassPath, "eyeRayPassClosestHit");
  Program shadowAnyHit      = _context->createProgramFromPTXFile(eyeRayPassPath, "shadowAnyHit");
  Program windowClosestHit  = _context->createProgramFromPTXFile(photonPassPath, "windowPassClosestHit");
  Program skyClosestHit     = _context->createProgramFromPTXFile(photonPassPath, "skyPassClosestHit");
  Program photonClosestHit  = _context->createProgramFromPTXFile(photonPassPath, "photonClosestHit");
  Program quickAnyHitProgram = _context->createProgramFromPTXFile(normalShaderPath, "any_hit_shadow");
  Program quickClosestHitProgram = _context->createProgramFromPTXFile(normalShaderPath, "closest_hit");
  Program quickAnyHit2Program = _context->createProgramFromPTXFile(normalShaderPath, "any_hit");
  mMaterial->setAnyHitProgram(    _context["importonType"   ]->getUint(), importonPassAnyHit);
  mMaterial->setClosestHitProgram(_context["importonType"   ]->getUint(), importonPassClosestHit);
  mMaterial->setAnyHitProgram(    _context["eyeRayType"   ]->getUint(), eyePassAnyHit);
  mMaterial->setClosestHitProgram(_context["eyeRayType"   ]->getUint(), eyePassClosestHit);
  mMaterial->setAnyHitProgram(    _context["shadowRayType"]->getUint(), shadowAnyHit);
  mMaterial->setClosestHitProgram(_context["windowRayType"]->getUint(), windowClosestHit);
  mMaterial->setAnyHitProgram(_context["windowRayType"]->getUint(), importonPassAnyHit);
  mMaterial->setClosestHitProgram(_context["skyRayType"   ]->getUint(), skyClosestHit);
  mMaterial->setClosestHitProgram(_context["photonRayType"]->getUint(), photonClosestHit);
  mMaterial->setAnyHitProgram(_context["photonRayType"]->getUint(), importonPassAnyHit);
  mMaterial->setAnyHitProgram(    _context["quickRayType"   ]->getUint(), quickAnyHit2Program);
  mMaterial->setClosestHitProgram(    _context["quickRayType"   ]->getUint(), quickClosestHitProgram);
  mMaterial->setAnyHitProgram(    _context["quickShadowRayType"   ]->getUint(), quickAnyHitProgram);
}
