//
// DiskImagesCamera.hpp : Camera class reading images from a point grey camera
//
#ifndef POINTGREYCAMERA_HPP_INCLUDED_
#define POINTGREYCAMERA_HPP_INCLUDED_

#include "CameraAPI.hpp"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <stdint.h>
#include <dc1394/dc1394.h>
#include <time.h>
#include <inttypes.h>
#include <sys/times.h>
#include <iostream>

#define SIZE_OF_DMA_BUFFER 2

using std::cout;
using std::endl;
using std::cin;
template<typename Pixel>
class PointGreyCamera: public CameraAPI<Pixel>
{
public:
  PointGreyCamera(int beginx, int beginy, int sizex, int sizey){
	sx=sizex;
	sy=sizey;
	bx=beginx;
	by=beginy;
	total_bytes = 0;
    d = dc1394_new ();
 
    err=dc1394_camera_enumerate (d, &list);


    if (list->num == 0) {
        dc1394_log_error("No cameras found");
    }

    camera = dc1394_camera_new (d, list->ids[0].guid);
    if (!camera) {
        dc1394_log_error("Failed to initialize camera with guid 0x%0lx",
                list->ids[0].guid);

    }
    dc1394_camera_free_list (list);


    /*-----------------------------------------------------------------------
     *  setup capture for format 7
     *-----------------------------------------------------------------------*/
     PixelTraits<Pixel> pt;

    dc1394_video_set_iso_speed(camera, DC1394_ISO_SPEED_400);
    dc1394_video_set_mode(camera, DC1394_VIDEO_MODE_FORMAT7_0);
    if(pt.pnm_maxval()==255)	
    	err = dc1394_format7_set_roi(camera, DC1394_VIDEO_MODE_FORMAT7_0,
                                 DC1394_COLOR_CODING_RGB8,
                                 DC1394_USE_MAX_AVAIL, // use max packet size
                                 bx, by, // left, top
                                 sx, sy);  // width, height

    else
    	err = dc1394_format7_set_roi(camera, DC1394_VIDEO_MODE_FORMAT7_0,
                                 DC1394_COLOR_CODING_RAW16,
                                 DC1394_USE_MAX_AVAIL, // use max packet size
                                 bx, by, // left, top
                                 sx, sy);  // width, height

    err=dc1394_capture_setup(camera, SIZE_OF_DMA_BUFFER, DC1394_CAPTURE_FLAGS_DEFAULT);

    /*-----------------------------------------------------------------------
     *  print allowed and used packet size
     *-----------------------------------------------------------------------*/
    err=dc1394_format7_get_packet_parameters(camera, DC1394_VIDEO_MODE_FORMAT7_0, &min_bytes, &max_bytes);



    err=dc1394_format7_get_packet_size(camera, DC1394_VIDEO_MODE_FORMAT7_0, &actual_bytes);


    err=dc1394_format7_get_total_bytes(camera, DC1394_VIDEO_MODE_FORMAT7_0, &total_bytes);

    /*-----------------------------------------------------------------------
     *  have the camera start sending us data
     *-----------------------------------------------------------------------*/
    err=dc1394_video_set_transmission(camera,DC1394_ON);
    //cout<<"after setting transmission"<<endl;
    if (err!=DC1394_SUCCESS) {
        dc1394_log_error("unable to start camera iso transmission");
        dc1394_capture_stop(camera);
        dc1394_camera_free(camera);
        exit(1);
    }
  }

  ~PointGreyCamera(){
    dc1394_capture_stop(camera);
    dc1394_video_set_transmission(camera, DC1394_OFF);
    //dc1394_camera_free(camera);
    //dc1394_free (d);
  }

  void Flush()
  {
	for(int i=0; i<SIZE_OF_DMA_BUFFER; i++)
	{
	        assert(dc1394_capture_dequeue(camera,DC1394_CAPTURE_POLICY_WAIT, &frame)==0);
		dc1394_capture_enqueue(camera,frame);
	}
  }

  void SetROI(int beginx, int beginy, int sizex, int sizey)
  {
	bx=beginx;
	by=beginy;
	sx=sizex;
	sy=sizey;
	
  }

  Image<Pixel> GetNextFrame(){
    err=dc1394_capture_dequeue(camera, DC1394_CAPTURE_POLICY_WAIT, &frame);
    assert(err==0);
    if (err!=DC1394_SUCCESS) {
      dc1394_log_error("unable to capture");
      dc1394_capture_stop(camera);
      dc1394_camera_free(camera);
      printf("unable to capture\n");
      exit(1);
    }

        /*---------------------------------------------------------------------
         *  output elapsed time
         *---------------------------------------------------------------------*/
  	Image<Pixel>im;


	im=Image<Pixel>(sy,sx);
        Pixel* test=(Pixel*)frame->image;
	//copy image to image.h file
	for(int a=by; a<by+sy; a++)
		for(int b=bx; b<bx+sx; b++)
			im(a-by,b-bx)=test[((a-by)*sx+b-bx)];
  // release buffer
  err=dc1394_capture_enqueue(camera,frame);
  assert(err==0);
	return im;
  }

  Image<Pixel> GetLatestFrame(){

    Flush();
    return GetNextFrame(); 
  }

  //Brightness is set in a range of 0 to 255
  void SetBrightness(int param1)
  {
	dc1394_feature_set_mode(camera,DC1394_FEATURE_BRIGHTNESS,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, DC1394_FEATURE_BRIGHTNESS, param1);
 }


  void SetExposure(int param1)
  {
	dc1394_feature_set_mode(camera,DC1394_FEATURE_EXPOSURE,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, DC1394_FEATURE_EXPOSURE, param1);
	std::cout << "Desired exposure: " << param1 << std::endl;
	uint32_t expo, min, max;
	dc1394_feature_get_value(camera, DC1394_FEATURE_EXPOSURE, &expo);
	dc1394_feature_get_boundaries(camera, DC1394_FEATURE_EXPOSURE, &min, &max);
	std::cout << "Actual exposure: " << expo << std::endl;
	std::cout << "Exposure bounds: (" << min << ", " << max << ")" << std::endl;
	
  }

  void SetWhiteBalance(int blue, int red)
  {
	dc1394_feature_set_mode(camera,DC1394_FEATURE_WHITE_BALANCE,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_whitebalance_set_value(camera,blue,red);
  }

  void SetGamma(int gamma)
  {
	dc1394_feature_set_mode(camera,DC1394_FEATURE_WHITE_BALANCE,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, DC1394_FEATURE_GAMMA, gamma);
  }

  void SetSaturation()
  {
  }

  // Has a range of 0 to 1074
  float SetShutter(int param1)
  {
	dc1394_feature_set_mode(camera,DC1394_FEATURE_SHUTTER,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, DC1394_FEATURE_SHUTTER, param1);
	//std::cout << "Desired shutter: " << param1 << std::endl;
	//uint32_t val, min, max;
	float abs_val;
	//dc1394_feature_get_value(camera, DC1394_FEATURE_SHUTTER, &val);
	dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_SHUTTER, &abs_val);
	//dc1394_feature_get_boundaries(camera, DC1394_FEATURE_SHUTTER, &min, &max);
	//std::cout << "Actual shutter: " << val << std::endl;
	//std::cout << "Absolute shutter: " << abs_val << std::endl;
	//std::cout << "Shutter bounds: (" << min << ", " << max << ")" << std::endl;
	return abs_val;
  }

  float SetShutterAbs(float shutter){
    	dc1394_feature_set_mode(camera,DC1394_FEATURE_SHUTTER,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_absolute_value(camera, DC1394_FEATURE_SHUTTER, shutter);
	float real_shutter;
	dc1394_feature_get_absolute_value(camera, DC1394_FEATURE_SHUTTER, &real_shutter);
	return real_shutter;
  }

  //Has a range of 56 to 739
  void SetGain(int param1)
  {
	dc1394_feature_set_mode(camera,DC1394_FEATURE_GAIN,DC1394_FEATURE_MODE_MANUAL);
	dc1394_feature_set_value(camera, DC1394_FEATURE_GAIN, param1);
  }

  //This isn't going to work yet because it isn't using a circular buffer yet
  void SetFrameRate()
  {
  }

  void SetHue()
  {
  }



private:
  char *filename_template;
  int bx,by,sx,sy;
  int first_idx;
  int last_idx;
  int current_idx;
  FILE* imagefile;
  dc1394camera_t *camera;
  int grab_n_frames;
  struct tms tms_buf;
  clock_t start_time;
  float elapsed_time;
  int i;
  unsigned int min_bytes, max_bytes;
  unsigned int actual_bytes;
  uint64_t total_bytes;
  unsigned int width, height;
  dc1394video_frame_t *frame;
  dc1394_t * d;
  dc1394camera_list_t * list;
  dc1394error_t err;
};

#endif //#ifndef DISKIMAGECAMERA_HPP_INCLUDED_
