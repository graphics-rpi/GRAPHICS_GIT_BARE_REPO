#ifndef STATE_HPP
#define STATE_HPP
//#include "wallfile_parser.hpp"
#include "person.hpp"


enum RenderResEnum
{
  FULL=0,
  PATCHES=1,
  TRIANGLES=2,
  HYBRID=3,
  TRIANGLE_MOMENTS=4,
  TRIANGLE_NOINT=5
};

enum SkyTypes
{
  CLEAR=0,
  TURBID=1,
  INTERMEDIATE=2,
  OVERCAST=3
  
};

enum DebugModeEnum
{
  STANDARD=0,
  NORMALS=1
};

enum CameraType
{
  STANDARDCAMERA=0,
  FISHEYECAMERA=1
};

#define DIRECT_PIXEL 0
#define DIRECT_TRI 1

#define INDIRECT_PIXEL 1
#define INDIRECT_TRI 0

class State
{
  public:
     State(int& argcParam, char**& argvParam):argc(argcParam),argv(argvParam)
     {

        renderRes=PATCHES;


        debugMode=0;
        bounce=10;
        changed=0;
        month=1;
        day=1;
        hour=12;
        minute=0;
        exposurePercent=30;
        outputfile_specified=false;
        useQT=true;
        useOrthoCamera=false;
        dumpImage=false;
        dumpPatches=false;
        dumpTris=false;
        screenFile="../window_screens/PagodaScreen_tileable.ppm";
        screenSpecified=false;
        moment=false;
        direct=0;
        indirect=0;
        dumpPeople=false;
        screen=0;
        viewAngle=90;
        greyscale=0;
        //Below this threshold we get the dim visualization
        toodim=-1;
        //Above this threshold we get the bright visualization
        toobright=999;
        cameraType=STANDARDCAMERA;
        
     }

     inline int getRenderRes(){return renderRes;};
     inline int getDebugMode(){return debugMode;};
     inline int getSkyType()  {return skyType;};
     int& argc; char**&argv;
     int renderRes;
     int debugMode;
     int skyType;
     int bounceType;
     int viewpoint;
     int screen;
     int exposurePercent;
     bool changed;
     int month,day,hour,minute;
     float latitude,longitude;
     int bounce;
     int direct;
     int indirect;
     bool outputfile_specified;
     bool useQT;
     bool dumpImage;
     bool dumpPatches;
     bool dumpTris;     
     bool screenSpecified;
     std::string outputfile;
     std::string inputfile;
     std::vector<std::string> orthoFiles;
     std::string imageFile;
     std::string patchesFile;
     std::string trisFile;
     std::string screenFile;
     bool useOrthoCamera;
     bool moment;
     bool dumpPeople;

     int brightvis;
     int currentPerson;
     int viewAngle;
     std::vector<Person> people;
     int greyscale;
     float toodim;
     float toobright;
     int cameraType;
  private:


};

#endif
