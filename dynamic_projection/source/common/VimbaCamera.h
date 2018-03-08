#include "VimbaCPP/Include/VimbaCPP.h"
#include <iostream>
#include <string>
#include <assert.h>  
#include "Image.h"
#include <boost/type_traits/is_same.hpp>

using namespace AVT;
using namespace VmbAPI;


template <typename Format>
class VimbaCamera
{
  public:
    //Constructor-
    VimbaCamera():system(VimbaSystem :: GetInstance ()),
      mono(false), height(-1), width(-1), exposure (200000)
    {
      cameraStartup();
    }
  
    //Public methods
    void cameraStartup();
    void takePicture(std::string, float);
    void listCameras(CameraPtrVector cameras);
    void setExposure(float);
    void setOffset(int, int);
    void setSize(int, int);
    void getWidthAndHeight(VmbInt64_t&, VmbInt64_t&);
    void getSensorWidthAndHeight(VmbInt64_t&, VmbInt64_t&);

    ~VimbaCamera()
    {
      system.Shutdown();
    }

        
  private:

    //Private methods
    void SetVimbaFormat(FeaturePtr&);
    //Private variables
    CameraPtrVector cameras ;
    CameraPtr camera;
    VimbaSystem & system;
    int res;
    bool mono;
    VmbInt64_t height;
    VmbInt64_t width;
    VmbInt64_t sensorHeight;
    VmbInt64_t sensorWidth;
    VmbInt64_t offsetX;
    VmbInt64_t offsetY;
    float exposure;

    Image<Format> image;

};

template <typename Format>
//Starts up Vimba with the first camera found
void VimbaCamera<Format>::cameraStartup()
{

  //Startup Vimba
  assert ( VmbErrorSuccess == system.Startup () && "Error starting VIMBA" );

  
  //Get a list of cameras
  assert ( VmbErrorSuccess == system.GetCameras ( cameras ) 
            && "Error getting cameras");

  listCameras(cameras);
  
  //Automatically use the first camera found
  assert(!cameras.empty());
  camera = cameras .at(0);

  assert ( VmbErrorSuccess == camera->Open( VmbAccessModeFull )
      &&"Error opening first camera");
  
  std :: cout << "Camera opened " << std :: endl;


  // Set the GeV packet size to the highest possible value
  // (In this example we do not test whether this cam actually is a GigE cam)
  FeaturePtr pCommandFeature;
  assert ( VmbErrorSuccess == camera->GetFeatureByName( 
      "GVSPAdjustPacketSize", pCommandFeature ));

  assert ( VmbErrorSuccess == pCommandFeature->RunCommand() );
  
  bool bIsCommandDone = false;
  do
  {
      if ( VmbErrorSuccess != pCommandFeature->IsCommandDone( bIsCommandDone ))
      {
          break;
      }
  } while ( false == bIsCommandDone );

  //We must set these here so the get functions can use the member variables
  //  instead of querying the camera (the set functions update the variables).
  getSensorWidthAndHeight(sensorWidth, sensorHeight);
  setOffset(0,0);
  setSize(sensorWidth, sensorHeight);



  setExposure(exposure);
}

template<typename Format>
void VimbaCamera<Format>::SetVimbaFormat(FeaturePtr& pFormatFeature)
{
  // Set pixel format. For the sake of simplicity we only support Mono and RGB in this example.
  VmbInt64_t pixelFormat;
  res = camera->GetFeatureByName( "PixelFormat", pFormatFeature );
  assert ( VmbErrorSuccess == res && "ERROR SETTING PIXEL FORMAT");
  
  // Try to set RGB
  if(boost::is_same<Format,sRGB>::value)
  {

    res = pFormatFeature->SetValue( VmbPixelFormatRgb8 );
    assert(VmbErrorSuccess == res&&"FAILED TO MAKE IT COLOR");
  }
  else
  {
    // Mono
    std::cout<<"Falling back to mono"<<std::endl;
    res = pFormatFeature->SetValue( VmbPixelFormatMono8 );
    mono=true;
  }

  // Read back the currently selected pixel format
  res=pFormatFeature->GetValue( pixelFormat );

  assert ( VmbErrorSuccess == res && "ERROR READING PIXEL FORMAT" );


}

//Sets the exposure (in microseconds)
template <typename Format>
void VimbaCamera <Format>::getWidthAndHeight(VmbInt64_t& pWidth, VmbInt64_t& pHeight)
{
  FeaturePtr pFormatFeature;

  // Save the current width
  int res = camera->GetFeatureByName( "Width", pFormatFeature );
  assert ( VmbErrorSuccess == res && "ERROR SELECTING WIDTH FEATURE ");
  
  res = pFormatFeature->GetValue( pWidth );
  assert ( VmbErrorSuccess == res && "ERROR GETTING WIDTH VALUE");
  
  // Save the current height
  res = camera->GetFeatureByName( "Height", pFormatFeature );
  assert ( VmbErrorSuccess == res && "ERROR SELECTING HEIGHT FEATURE" );
  
  pFormatFeature->GetValue( pHeight );
  assert ( VmbErrorSuccess == res && "ERROR GETTING HEIGHT VALUE");
 

}

//Sets the exposure (in microseconds)
template <typename Format>
void VimbaCamera <Format>::getSensorWidthAndHeight(VmbInt64_t& pSensorWidth, VmbInt64_t& pSensorHeight)
{
  FeaturePtr pFormatFeature;

  // Save the current width
  int res = camera->GetFeatureByName( "SensorWidth", pFormatFeature );
  assert ( VmbErrorSuccess == res && "ERROR SELECTING WIDTH FEATURE ");
  
  res = pFormatFeature->GetValue( pSensorWidth );
  assert ( VmbErrorSuccess == res && "ERROR GETTING WIDTH VALUE");
  
  // Save the current height
  res = camera->GetFeatureByName( "SensorHeight", pFormatFeature );
  assert ( VmbErrorSuccess == res && "ERROR SELECTING HEIGHT FEATURE" );
  
  pFormatFeature->GetValue( pSensorHeight );
  assert ( VmbErrorSuccess == res && "ERROR GETTING HEIGHT VALUE");
}

//Sets the exposure (in microseconds)
template <typename Format>
void VimbaCamera <Format>::setExposure(float exp)
{
  static bool first=true;
  if(first||exp!=exposure)
  {
    exposure=exp;
    FeaturePtr exposureFeature;
    res = camera->GetFeatureByName( "ExposureAuto", exposureFeature );
    assert ( VmbErrorSuccess == res && "ERROR GETTING AUTO EXPOSURE SWITCH");
    res = exposureFeature->SetValue( "Off" );
    assert( VmbErrorSuccess == res && "Error making exposure manual");

    //Choose to set the exposure time
    res = camera->GetFeatureByName( "ExposureTimeAbs", exposureFeature );
    assert ( VmbErrorSuccess == res && "Error getting exposure time feature");

   //Sets exposure time in microseconds
    res = exposureFeature->SetValue( exposure);
    std::cout<<"setting exposure to "<<exposure<<std::endl;
    assert ( VmbErrorSuccess == res && "Error setting exposure time");
    first=false;
  }
  else
    std::cout<<"Exposure is already properly set"<<std::endl;
}

//Sets the Offset (from right (X) and top (Y) )
template <typename Format>
void VimbaCamera <Format>::setOffset(int offX, int offY)
{

  assert(offX<sensorWidth);
  assert(offY<sensorHeight);

  FeaturePtr offsetXFeature;
  FeaturePtr offsetYFeature;

  res = camera->GetFeatureByName( "OffsetX", offsetXFeature );
  assert ( VmbErrorSuccess == res && "ERROR GETTING OFFSET X");
  res = offsetXFeature->SetValue( offX );
  assert( VmbErrorSuccess == res && "ERROR SETTING OFFSET X");
  offsetX=offX;

  res = camera->GetFeatureByName( "OffsetY", offsetYFeature );
  assert ( VmbErrorSuccess == res && "ERROR GETTING OFFSET Y");
  res = offsetYFeature->SetValue( offY );
  assert( VmbErrorSuccess == res && "ERROR SETTING OFFSET Y");
  offsetY=offY;

}


//Sets the Size of the image
template <typename Format>
void VimbaCamera <Format>::setSize(int X, int Y)
{

  assert(X+offsetX<=sensorWidth);
  assert(Y+offsetY<=sensorHeight);

  FeaturePtr sizeXFeature;
  FeaturePtr sizeYFeature;

  res = camera->GetFeatureByName( "Width", sizeXFeature );
  assert ( VmbErrorSuccess == res && "ERROR GETTING OFFSET X");
  res = sizeXFeature->SetValue( X );
  assert( VmbErrorSuccess == res && "ERROR SETTING SIZE X");
  width=X;

  res = camera->GetFeatureByName( "Height", sizeYFeature );
  assert ( VmbErrorSuccess == res && "ERROR GETTING SIZE Y");
  res = sizeYFeature->SetValue( Y );
  assert( VmbErrorSuccess == res && "ERROR SETTING SIZE Y");
  height=Y;

}

//Takes a single picture and puts it in "filename"
template <typename Format>
void VimbaCamera <Format>::takePicture(std::string filename, float exposureParam=2000)
{
  FramePtr frame;

  setExposure(exposureParam);

  // Acquire
  res = camera->AcquireSingleImage( frame, exposureParam );
  assert ( VmbErrorSuccess == res && "Error acquiring image");

  VmbUchar_t *pBuffer;
  
  assert ( VmbErrorSuccess == frame->GetImage( pBuffer ) );
  if(sizeof(Format)>1)
  {
    assert(sizeof(Format)!=1);
    image = Image<Format>((int) height, (int)width, (Format *)pBuffer);
    image.write(filename);
  }

  else
  {
    image= Image<Format>((int) height, (int) width, (Format*) pBuffer);
    image.write(filename);
  }
  std::cout<<"Writing to "<<filename<<"..."<<std::endl;
}

//List the available camera models
template <typename Format>
void VimbaCamera<Format>::listCameras(CameraPtrVector cameras)
{
  int count=0;
  std :: string name;
  for (
  CameraPtrVector :: iterator iter = cameras .begin ();
  cameras .end () != iter;
  ++ iter )
  {
    std::cout<<"count: "<<count<<std::endl;
    if ( VmbErrorSuccess == (* iter)-> GetName ( name ) )
      std :: cout << name << std :: endl;
  }
}
