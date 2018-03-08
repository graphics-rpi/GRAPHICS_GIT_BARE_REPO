//
// GigEVisionCamera.hpp : Camera class reading images from a GigE Vision camera
//
#ifndef GIGEVISIONCAMERA_HPP_INCLUDED_
#define GIGEVISIONCAMERA_HPP_INCLUDED_



#define _LINUX
#define _x64

#ifdef _WIN32
#include <PvApi.h>
#else
#include "GigEVision/PvApi.h"
#endif
#include <arpa/inet.h>
#include "CameraAPI.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include <sys/times.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include "Image.h"
#include "GigEVisionUtil.h"


#include <string>

inline  std::string identify_tPvErr(tPvErr e) {
  if      (e == ePvErrSuccess)        { return "ePvErrSuccess        = No error"; }
  else if (e == ePvErrCameraFault)    { return "ePvErrCameraFault    = Unexpected camera fault"; }
  else if (e == ePvErrInternalFault)  { return "ePvErrInternalFault  = Unexpected fault in PvApi or driver"; }
  else if (e == ePvErrBadHandle)      { return "ePvErrBadHandle      = Camera handle is invalid"; }
  else if (e == ePvErrBadParameter)   { return "ePvErrBadParameter   = Bad parameter to API call"; }
  else if (e == ePvErrBadSequence)    { return "ePvErrBadSequence    = Sequence of API calls is incorrect"; }
  else if (e == ePvErrNotFound)       { return "ePvErrNotFound       = Camera or attribute not found"; }
  else if (e == ePvErrAccessDenied)   { return "ePvErrAccessDenied   = Camera cannot be opened in the specified mode"; }
  else if (e == ePvErrUnplugged)      { return "ePvErrUnplugged      = Camera was unplugged"; }
  else if (e == ePvErrInvalidSetup)   { return "ePvErrInvalidSetup   = Setup is invalid (an attribute is invalid)"; }
  else if (e == ePvErrResources)      { return "ePvErrResources      = System/network resources or memory not available"; }
  else if (e == ePvErrBandwidth)      { return "ePvErrBandwidth      = 1394 bandwidth not available"; }
  else if (e == ePvErrQueueFull)      { return "ePvErrQueueFull      = Too many frames on queue"; }
  else if (e == ePvErrBufferTooSmall) { return "ePvErrBufferTooSmall = Frame buffer is too small"; }
  else if (e == ePvErrCancelled)      { return "ePvErrCancelled      = Frame cancelled by user"; }
  else if (e == ePvErrDataLost)       { return "ePvErrDataLost       = The data for the frame was lost"; }
  else if (e == ePvErrDataMissing)    { return "ePvErrDataMissing    = Some data in the frame is missing"; }
  else if (e == ePvErrTimeout)        { return "ePvErrTimeout        = Timeout during wait"; }
  else if (e == ePvErrOutOfRange)     { return "ePvErrOutOfRange     = Attribute value is out of the expected range"; }
  else if (e == ePvErrWrongType)      { return "ePvErrWrongType      = Attribute is not this type (wrong access function) "; }
  else if (e == ePvErrForbidden)      { return "ePvErrForbidden      = Attribute write forbidden at this time"; }
  else if (e == ePvErrUnavailable)    { return "ePvErrUnavailable    = Attribute is not available at this time"; }
  else if (e == ePvErrFirewall)       { return "ePvErrFirewall       = A firewall is blocking the traffic (Windows only)"; }
  else if (e == __ePvErr_force_32)    { return "__ePvErr_force_32  "; }
  else return "identify_tPvErr: UNKNOWN ERROR!";
}









// specialization for MONO8
template<>
struct GigEVisionPixelTraits<GigEVisionPixelTypes::SAMPLE_MONO8>
  : public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  typedef uint8_t base_pixel_t;
  typedef uint8_t pixel_t;
  const char *FormatString(){
    return pixel_formats[GigEVisionPixelTypes::SAMPLE_MONO8];
  }
  SampleFormat Format(){
    return GigEVisionPixelTypes::SAMPLE_MONO8;
  }
};

// specialization for BAYER8
template<>
struct GigEVisionPixelTraits<GigEVisionPixelTypes::SAMPLE_BAYER8>
  : public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  typedef uint8_t base_pixel_t;
  typedef Vector3<uint8_t> pixel_t;
  const char *FormatString(){
    return pixel_formats[GigEVisionPixelTypes::SAMPLE_BAYER8];
  }
  SampleFormat Format(){
    return GigEVisionPixelTypes::SAMPLE_BAYER8;
  }
};

// specialization for MONO16
template<>
struct GigEVisionPixelTraits<GigEVisionPixelTypes::SAMPLE_MONO16>
  : public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  typedef uint16_t base_pixel_t;
  typedef uint16_t pixel_t;
  const char *FormatString(){
    return pixel_formats[GigEVisionPixelTypes::SAMPLE_MONO16];
  }
  SampleFormat Format(){
    return GigEVisionPixelTypes::SAMPLE_MONO16;
  }
};

// specialization for BAYER16
template<>
struct GigEVisionPixelTraits<GigEVisionPixelTypes::SAMPLE_BAYER16>
  : public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  typedef uint16_t base_pixel_t;
  typedef Vector3<uint16_t> pixel_t;
  const char *FormatString(){
    return pixel_formats[GigEVisionPixelTypes::SAMPLE_BAYER16];
  }
  SampleFormat Format(){
    return GigEVisionPixelTypes::SAMPLE_BAYER16;
  }
};

// specialization for RGB24
template<>
struct GigEVisionPixelTraits<GigEVisionPixelTypes::SAMPLE_RGB24>
  : public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  typedef uint8_t base_pixel_t;
  typedef Vector3<uint8_t> pixel_t;
  const char *FormatString(){
    return pixel_formats[GigEVisionPixelTypes::SAMPLE_RGB24];
  }
  SampleFormat Format(){
    return GigEVisionPixelTypes::SAMPLE_RGB24;
		//std::cout << "got to format" << std::endl;
  }
};

// specialization for RGB48
template<>
struct GigEVisionPixelTraits<GigEVisionPixelTypes::SAMPLE_RGB48>
  : public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  typedef uint16_t base_pixel_t;
  typedef Vector3<uint16_t> pixel_t;
  const char *FormatString(){
    return pixel_formats[GigEVisionPixelTypes::SAMPLE_RGB48];
  }
  SampleFormat Format(){
    return GigEVisionPixelTypes::SAMPLE_RGB48;
  }
};


template<GigEVisionPixelTypes::SampleFormat SampleFormat>
class GigEVisionCamera
  : public CameraAPI<typename GigEVisionPixelTraits<SampleFormat>::pixel_t>
{
  typedef typename GigEVisionPixelTraits<SampleFormat>::pixel_t Pixel;
  typedef typename GigEVisionPixelTraits<SampleFormat>::base_pixel_t base_pixel_t;
  
  //========================================================================================
  //FrameWrapper
  //========================================================================================
  //========================================================================================
  //========================================================================================
  //========================================================================================
  
  
  
  struct FrameWrapper {
    tPvFrame frame;
    unsigned int framesize;
    FrameWrapper(unsigned int FrameSize){
      framesize = FrameSize;
      frame.ImageBuffer = new uint8_t[framesize];
    }
    FrameWrapper(const FrameWrapper &a){
      framesize = a.framesize;
      frame.ImageBuffer = new uint8_t[framesize];
      frame.ImageBufferSize = framesize;
      frame.AncillaryBuffer = NULL;
      frame.AncillaryBufferSize = 0;
      
    }
    ~FrameWrapper(){

      delete [] (uint8_t*)frame.ImageBuffer;

    }
  };
  
public:
  
  static unsigned long INITIALIZE_CAMERAS() {
    
    static bool first = true;
    static unsigned long camera_count = 0;
    if (!first) { return camera_count; }
    
    printf("INITIALIZE_CAMERAS\r\n");
    static tPvErr err = PvInitialize();
    if (err != ePvErrSuccess) {
      std::cout << "WARNING: could not PvInitialize " << std::endl;
      std::cout << err << " " << identify_tPvErr(err) << std::endl;
      throw(0);
    }
    assert (err == ePvErrSuccess);

    // detect cameras on the network
    //unsigned long camera_count = 0;
    const int count_retries = 1000;
    for (int i=0;i<count_retries;i++){
      camera_count = PvCameraCount();
      if (camera_count){
	break;
      }
      usleep(100000);
    }

    std::cout << "initialize finished with " << camera_count << " cameras" << std::endl;
    return camera_count;
  }

  
  // =================================================================================================
  // =================================================================================================
  GigEVisionCamera(GigEVisionCameraTypes::CameraModel model = GigEVisionCameraTypes::CAMERA_ANY, unsigned long ip=0){

    cached_ip = 1;
    
    INITIALIZE_CAMERAS();
    if (ip == 0) { 
      //printf("NO IP SPECIFIED\r\n"); 
    }

    // DETECT CAMERAS ON THE NETWORK
    unsigned long camera_count = 0;
    const int count_retries = 10;
    for (int i=0;i<count_retries;i++){
      camera_count = PvCameraCount();
      if (camera_count){ break; }
      usleep(10000);
    }
    if (!camera_count){
      fprintf(stderr, "no cameras found\n");
      exit(0);
    }
    std::cout << "Found " << camera_count << " cameras!\r\n";

    
    // LOOK FOR THE SPECIFIC IP REQUESTED
    tPvCameraInfo *camera_info = new tPvCameraInfo[camera_count];
    PvCameraList(camera_info, camera_count, NULL);    
    //The IP settings for a camera
    tPvIpSettings ip_settings;
    unsigned int camera_idx;
    for (camera_idx=0; (unsigned int)camera_idx<camera_count; camera_idx++) {
      //If the IP is not specified
      if(ip==0)	{	  
	printf("UNSPECIFIED IP\r\n");
	if (model == GigEVisionCameraTypes::CAMERA_ANY) {
	  break;
	}
  else
    std::cout <<" NOT CAMERA_ANY \r\n";
	if (!strcmp(camera_info[camera_idx].DisplayName, 
		    GigEVisionCameraTypes::display_names[model])){
	  break;
	}
      } 
      
      // if the IP is specified
      else { 
	tPvErr tmp = PvCameraIpSettingsGet(camera_info[camera_idx].UniqueId, &ip_settings);
	if (tmp != ePvErrSuccess) {
	  std::cout << "CRAP! ERROR IN IP SETTINGS1" << tmp << " " << identify_tPvErr(tmp) << std::endl;
	  assert(0);
	  exit(0);
        }
        if (ip_settings.CurrentIpAddress==ip) break;
        else {
	  std::cout << "this camera is available: " << ip_settings.CurrentIpAddress << std::endl;
	}
      }
    }


    tPvErr tmp = PvCameraIpSettingsGet(camera_info[camera_idx].UniqueId, &ip_settings);
    std::cout << "RET VAL " << tmp << " " << ePvErrSuccess << " " << ePvErrNotFound << std::endl;
    if (tmp != ePvErrSuccess) {
      std::cout << "CRAP! ERROR IN IP SETTINGS2" << tmp << " " << identify_tPvErr(tmp) << std::endl;
      assert (tmp == ePvErrNotFound); 
      // assert(0);
      exit(0);
      std::cout << "should have exited" << std::endl;
    }


    //    if (PvCameraIpSettingsGet(camera_info[camera_idx].UniqueId, &ip_settings) != ePvErrSuccess) {
    //std::cout << "CRAP! ERROR IN IP SETTINGS" << std::endl;
    // exit(0);
    //}
    
    if(camera_idx==camera_count) {
      fprintf(stderr, "Camera with specified ip not found\n");
      throw(ip);
    }
    
    //else
    // copy chosen camera info
    memcpy(&Info, &camera_info[camera_idx], sizeof(tPvCameraInfo));
    delete [] camera_info;


    std::cout << "SET CACHED IP " << ip_settings.CurrentIpAddress << std::endl;
    cached_ip = ip_settings.CurrentIpAddress;
    //exit(0);
    
    // get the IP settings for the selected camera
    tmp = PvCameraIpSettingsGet(Info.UniqueId, &ip_settings);
    if (tmp != ePvErrSuccess) {
      fprintf(stderr, "Failed to obtain ip settings\n");
      std::cout << tmp << " " << identify_tPvErr(tmp) << std::endl;
      exit(0);
    }
    
    //#define DUMP_CAMERA_IP_INFO
#ifdef  DUMP_CAMERA_IP_INFO
    fprintf(stderr, "ConfigMode        = %d\r\n", ip_settings.ConfigMode);
    fprintf(stderr, "ConfigModeSupport = %ld\r\n", ip_settings.ConfigModeSupport);
    fprintf(stderr, "CurrentIpAddress  = 0x%ld\r\n", ip_settings.CurrentIpAddress);
    fprintf(stderr, "CurrentIpSubnet   = 0x%ld\r\n", ip_settings.CurrentIpSubnet);
    fprintf(stderr, "CurrentIpGateway  = 0x%ld\r\n", ip_settings.CurrentIpGateway);
    fprintf(stderr, "PersistentIpAddress  = 0x%ld\r\n", ip_settings.PersistentIpAddr);
    fprintf(stderr, "PersistentIpSubnet   = 0x%ld\r\n", ip_settings.PersistentIpSubnet);
    fprintf(stderr, "PersistentIpGateway  = 0x%ld\r\n", ip_settings.PersistentIpGateway);
    fprintf(stderr, "Specified IP %ld\r\n", ip);
#endif // #ifdef  DUMP_CAMERA_IP_INFO
    
    tPvErr whatistheerror;
    // open the already identified camera
    //std::cout << "here1" << std::endl;
    whatistheerror = PvCameraOpen(Info.UniqueId,ePvAccessMaster,&cam);
    
    if(whatistheerror != ePvErrSuccess){
      std::cout << "PvCameraOpen error code " << whatistheerror << std::endl;

      std::cout << identify_tPvErr(whatistheerror) << std::endl;

      fprintf(stderr, "Failed to open camera\n"); 
      throw(ip);
      //exit(0);
    }
    
    //Adjust the packet size to the required for the system
#ifndef NDEBUG
    int ret = 
#endif
      PvAttrUint32Set(cam,"PacketSize",1500);
    assert(0 == ret);
		
    PvAttrEnumSet(cam,"ExposureMode","Manual");
    PvAttrEnumSet(cam,"WhitebalMode","Manual");
    PvAttrEnumSet(cam,"GainMode","Manual");
    PvAttrEnumSet(cam,"PixelFormat", traits.FormatString());
		//std::cout << "here3" << std::endl;
    allocate_frame_buffers();
		//std::cout << "back in constructor" << std::endl;
  }

  ~GigEVisionCamera(){
    if(PvCaptureEnd(cam)!=ePvErrSuccess){
      assert(0);
    }
        
    PvCameraClose(cam);
    PvUnInitialize();
  }

  unsigned long cached_ip;
  unsigned long getIP() const { return cached_ip; }//return Info }

  // capture individual frames to on-board memory when triggered by software
  void InitMultipleFrameCapture(){
#ifndef NDEBUG
    int ret = 
#endif      
      PvAttrEnumSet(cam,"AcquisitionMode","SingleFrame");
    assert(ePvErrSuccess == ret);    
#ifndef NDEBUG
    ret = 
#endif
      PvAttrEnumSet(cam,"FrameStartTriggerMode","Software");
    assert(ePvErrSuccess == ret);    
    if(PvCaptureStart(cam)!=ePvErrSuccess){
      assert(0);
    }
  }
  
  // continous capture mode (using ping-pong buffers)
  void InitContinuousCapture(float frame_rate = 32.79){
    while (1) {

      
      
      tPvErr ret = ePvErrSuccess;
      
      float min,max;

      // BARB: NOT SURE WHY THE BETTER FRAME RATE DOESN'T WORK (getting an out of bounds error)
      ret = PvAttrRangeFloat32(cam,"FrameRate", &min,&max); 
      printf("trying to set the frame rate %f %f\r\n", min, max);

      ret = PvAttrFloat32Set(cam,"FrameRate", max); 




      //ret = PvAttrFloat32Set(cam,"FrameRate", frame_rate);

      if (ret != ePvErrSuccess) { std::cout << "continuous capture init error a " << identify_tPvErr(ret) << std::endl; continue; }
      ret = PvAttrEnumSet(cam,"AcquisitionMode","Continuous");
      if (ret != ePvErrSuccess) { std::cout << "continuous capture init error b " << identify_tPvErr(ret) << std::endl; continue; }
      ret = PvAttrEnumSet(cam,"FrameStartTriggerMode","Freerun");
      if (ret != ePvErrSuccess) { std::cout << "continuous capture init error c " << identify_tPvErr(ret) << std::endl; continue; }
      ret = PvCaptureStart(cam);
      if (ret != ePvErrSuccess) { std::cout << "continuous capture init error d " << identify_tPvErr(ret) << std::endl; continue; }
      // otherwise, success!
      break;
    }
  }

  void Flush(){
    // not implemented
  }

  Image<Pixel> GetNextFrame(){

    static unsigned int good = 0;
    static unsigned int bad = 0;

    if (read_idx == write_idx){
      read_idx = 0;
      write_idx = 1;
      if(PvCaptureQueueFrame(cam,&frames[read_idx].frame,NULL)!=ePvErrSuccess){
	assert(0);
      }
      if(PvCommandRun(cam,"AcquisitionStart")!=0){
	assert(0);
      }
      return GetNextFrame();
    }

    bool good_frame = false;
    int count = 0;
    do {
      count++;
      PvCaptureWaitForFrameDone(cam,&frames[read_idx].frame,PVINFINITE);
      tPvErr ret = frames[read_idx].frame.Status1;
      if (ret == ePvErrSuccess){
	good_frame = true;
      } else {
	//fprintf(stderr, "Camera error: returned %d", ret);

	/*
	std::cerr << "GetNextFrame() error=" << ret << " " << identify_tPvErr(ret);
	std::cout << "   good: " << good << "   bad: " <<  bad << "    (" << std::fixed << std::setprecision(2) << bad*100 / double (good+bad) << "% bad frames)" << std::endl;
	*/

	//tPvErr err = PvCaptureQueueClear(cam);
	//std::cerr << "clear attempt: " << err << " " << identify_tPvErr(err) << std::endl; 
	//tPvErr err;
	//err = PvCommandRun(cam,"AcquisitionStop");
	//std::cerr << "acquisition stop: " << err << " " << identify_tPvErr(err) << std::endl; 
	//usleep(200000);
	//err = PvCaptureQueueClear(cam);
	//std::cerr << "clear attempt: " << err << " " << identify_tPvErr(err) << std::endl; 
	//usleep (100000);
	bad++;
	good_frame = false;
	//continue;
	
      }

      read_idx = 1 - read_idx;
      write_idx = 1 - write_idx;    

      tPvErr err = PvCaptureQueueFrame(cam,&frames[write_idx].frame,NULL);
      if (err !=ePvErrSuccess){
	std::cout << "GetNextFrame() error in capture queue frame " << err << identify_tPvErr(err) << std::endl;
	std::cout << "good: " << good << "   bad: " <<  bad << std::endl;
	good_frame = false;
	usleep(10000);
	bad++;
	continue;
      }      
    } while(!good_frame);
    if (count > 1) { 
      //  std::cout << "getframe attempts > 1   " << count << std::endl; 
    }

    good++;
    //std::cout << "good: " << good << "   bad: " <<  bad << std::endl;

    Image<Pixel> image(rows, cols);

    if (isBayer()){
      unsigned long linepadding = ULONG_PADDING(cols*3);
      unsigned long pixelpadding = 2;
      PvUtilityColorInterpolate(&frames[write_idx].frame,
				(base_pixel_t*)image.getData() + 0,
				(base_pixel_t*)image.getData() + 1,
				(base_pixel_t*)image.getData() + 2,
				pixelpadding,
				linepadding);      
    } else {
      memcpy(image.getData(), frames[write_idx].frame.ImageBuffer, rows*cols*sizeof(Pixel));
    }
    return image;
  }


  // captures frame to camera memory, but doesn't download
  void CaptureFrame(){
    PvAttrEnumSet(cam,"StreamHoldEnable","On");
    if(PvCaptureQueueFrame(cam,&frames[write_idx].frame,NULL)!=ePvErrSuccess){
      assert(0);
    }

    circular_increment(write_idx);

    if(PvCommandRun(cam,"AcquisitionStart")!=0){
      assert(0);
    }

    // software trigger
    if(PvCommandRun(cam,"FrameStartTriggerSoftware")!=0){
      assert(0);
    }
  }

  // downloads frames captured to camera internal memory
  Image<Pixel> DownloadFrame(){
    Image<Pixel> image(rows, cols);
    PvAttrEnumSet(cam,"StreamHoldEnable","Off");
    PvCaptureWaitForFrameDone(cam,&frames[read_idx].frame,PVINFINITE);
    assert(frames[read_idx].frame.Status1 == ePvErrSuccess);

    if (isBayer()){
      unsigned long linepadding = ULONG_PADDING(cols*3);
      unsigned long pixelpadding = 2;
      PvUtilityColorInterpolate(&frames[write_idx].frame,
				(base_pixel_t*)image.getData() + 0,
				(base_pixel_t*)image.getData() + 1,
				(base_pixel_t*)image.getData() + 2,
				pixelpadding,
				linepadding);      
    } else {
      //std::cout << "imgbuf 3" << std::endl;
      memcpy(image.getData(), frames[write_idx].frame.ImageBuffer, 
	     rows*cols*sizeof(Pixel));
      //		 std::cout << "imgbuf 4" << std::endl;
    }

    circular_increment(read_idx);
    return image;
  }

  Image<Pixel> GetLatestFrame(){
    int retry_count = 0;
    do {
      if(retry_count>0){
        std::cout<<"WAITING-----------------------"<<std::endl;
      }
      int ret;
      ret = PvAttrEnumSet(cam,"FrameStartTriggerMode","Software");
      assert(ret==0);
      if (ret !=  0) exit(0);
      //Makes the camera start acquiring images  
      if(PvCommandRun(cam,"AcquisitionStart")!=ePvErrSuccess){
	std::cout<<"retry count: "<<retry_count<<std::endl;
	assert(0);
      }
     
      if(PvCaptureQueueFrame(cam,&frames[write_idx].frame,NULL)!=ePvErrSuccess){
	assert(0);
     }
      
      // software trigger
      int err;
      
      err=PvCommandRun(cam,"FrameStartTriggerSoftware");
      if(err!=ePvErrSuccess){
	
        std::cout<<"retry count: "<<retry_count<<" "<<err<<std::endl;
	assert(0);
      }
      
      /*std::cout<<"PVcapturewait: "<<*/PvCaptureWaitForFrameDone(cam,&frames[write_idx].frame,PVINFINITE);//<<std::endl;
      retry_count++;
    } while (frames[write_idx].frame.Status1 != ePvErrSuccess&&!usleep(100000));
    
    if (retry_count > 1){
      //      printf("retries = %d\n", retry_count);
    }
    
    PvCommandRun(cam,"AcquisitionStop");
    
    Image<Pixel> image(rows, cols);
    
    if (isBayer()){
      unsigned long linepadding = ULONG_PADDING(cols*3);
      unsigned long pixelpadding = 2;
      PvUtilityColorInterpolate(&frames[write_idx].frame,
				(base_pixel_t*)image.getData() + 0,
				(base_pixel_t*)image.getData() + 1,
				(base_pixel_t*)image.getData() + 2,
				pixelpadding,
				linepadding);      
    } else {
      //std::cout << "imgbuf 5" << std::endl;
      memcpy(image.getData(), frames[write_idx].frame.ImageBuffer, 
	     rows*cols*sizeof(Pixel));
      //		 std::cout << "imgbuf 6" << std::endl;
    }
    return image;
  }

  void SetROI(int row_offset, int col_offset, int rows, int cols){
    this->rows = rows;
    this->cols = cols;
    regionX = col_offset;
    regionY = row_offset;
    PvAttrUint32Set(cam,"Height", rows);
    PvAttrUint32Set(cam,"Width", cols);
    PvAttrUint32Set(cam,"RegionX", regionX);
    PvAttrUint32Set(cam,"RegionY", regionY);
    allocate_frame_buffers();
  }

  void SetBinning(int row_bins, int col_bins){
    this->binningX = col_bins;
    this->binningY = row_bins;
    PvAttrUint32Set(cam, "BinningX", binningX);
    PvAttrUint32Set(cam, "BinningY", binningY);
    allocate_frame_buffers();
  }

  void SetExposure(long usecs){
#ifndef NDEBUG
    int ret = 
#endif
      PvAttrUint32Set(cam,"ExposureValue", usecs);
    assert(ePvErrSuccess == ret);
  }

  void SetWhiteBalance(int red_pct, int blue_pct){
    PvAttrUint32Set(cam, "WhitebalValueBlue", blue_pct);
    PvAttrUint32Set(cam, "WhitebalValueRed", red_pct);	
  }

  void SetGain(long gain_dB){
#ifndef NDEBUG    
    int ret =       
#endif
      PvAttrUint32Set(cam, "GainValue", gain_dB);
    assert(ePvErrSuccess == ret);
    
  }

private:
  tPvUint32 rows, cols;
  tPvUint32 regionX, regionY;
  tPvUint32 binningX, binningY;
  unsigned long FrameSize;
  tPvUint32 MemoryCapacity;
  tPvHandle cam;
  std::vector<FrameWrapper> frames;
  int read_idx;
  int write_idx;
  tPvCameraInfo Info;
  GigEVisionPixelTraits<SampleFormat> traits;

  //  my_ip;

  int circular_increment(int n){
    return (n+1) % MemoryCapacity;
  }

  template <typename T>
  T ULONG_PADDING(T x){
    return (((x+3) & ~3) - x);
  }

  bool isBayer(){
    switch (traits.Format()){
    case GigEVisionPixelTypes::SAMPLE_MONO8:
    case GigEVisionPixelTypes::SAMPLE_MONO16:
    case GigEVisionPixelTypes::SAMPLE_RGB24:
    case GigEVisionPixelTypes::SAMPLE_RGB48:
      return false;
      break;
    case GigEVisionPixelTypes::SAMPLE_BAYER8:
    case GigEVisionPixelTypes::SAMPLE_BAYER16:
      return true;
      break;
    }
    return false;
  }

  void allocate_frame_buffers(){
    //Obtains the frame size
    PvAttrUint32Get(cam,"TotalBytesPerFrame",&FrameSize);
    PvAttrUint32Get(cam,"Height",&rows);

#ifndef NDEBUG
    int ret = 
#endif      
      PvAttrUint32Get(cam, "StreamHoldCapacity", &MemoryCapacity);

    
    assert(ePvErrSuccess == ret);
    
    frames.clear();
    frames.resize(MemoryCapacity, FrameWrapper(FrameSize));
		
    read_idx = 0;
    write_idx = 0;
		
  }
};

#include "GigEVisionCamera.cpp"

#endif //#ifndef GIGEVISIONCAMERA_HPP_INCLUDED_
