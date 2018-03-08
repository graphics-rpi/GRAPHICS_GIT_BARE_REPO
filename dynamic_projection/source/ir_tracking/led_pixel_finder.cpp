#include "../common/GigEVisionCamera.hpp"
#include <X11/Xlib.h>
#include <sys/time.h>
#include <cstdio>
#include "../common/Vector3.h"
#include "FindLeds.hpp"
#include <string>
#include "viewer.h"
#include <fstream>
#include "../common/directory_locking.h"
#include <iomanip>
#include <sstream>

#define IR_STATE_DIRECTORY          "../state/ir_tracking"

#define RAW_IR_IMAGE_FILENAME              "../state/ir_tracking/ir_image.pgm"
#define DETECTED_IR_POINTS_IMAGE_FILENAME  "../state/ir_tracking/ir_image_detected_ir_points.pgm"
#define MICRO_IMAGE_PATH                   "../state/ir_tracking/micro_images/"
#define FOUND_IR_POINTS_FILENAME           "../state/ir_tracking/found_ir_points.txt"

// ======================
// some global variables
// ======================
//int shutter = 10* 1000; //10ms
//int shutter = 30* 1000; //10ms
int shutter = 10* 1000; //10ms
//unsigned long ip_b=16842409;
//unsigned long ip_a=3199467177;

//int num_known_ips = 5;
//unsigned long known_ips[5] = { 16842409, 1868824233, 1244987049, 2565865129, 3199467177 };

//int num_known_ips = 1;
//unsigned long known_ips[1] = { 689962665 }; //, 3199467177 };
//unsigned long known_ips[1] = { 1774649001 }; //1559756457 }; //3199467177 };

int viewer_which_camera = 0;
int micro_image_count = 0;
bool save_micro_image = false;

std::vector<unsigned long> ips; // known_ips[2] = { 16842409, 3199467177 };
//std::vector<unsigned long> ips; // known_ips[2] = { 16842409, 3199467177 };

std::string image_filename = "";

extern int brightness_threshold_seed;// = 25;


#if 0
// previous value...  works in lab
//int gain = 6;  // 6 dB
//int gain = 12;
int gain = 17;
// in lab values?
///extern int brightness_threshold_seed = 25;
int brightness_threshold_cc = 15;
//int brightness_threshold_cc = 7;

#else
// found it needed to be higher in EMPAC theater
//int gain = 10;  // 6 dB
int gain = 20;  // 6 dB
//int gain = 300;  // 6 dB
//int gain = 40;  // 6 dB
//  in empac values?
//int brightness_threshold_seed = 25;
//int brightness_threshold_seed = 40;
int brightness_threshold_cc = 10;
#endif


//int min_ir_point_radius = 10;
int min_ir_point_radius = 3;
//int min_ir_point_radius = 1;
//int max_ir_point_radius = 20;
int max_ir_point_radius = 10;



/*
// previous value...  
int gain = 6;  // 6 dB
// found it needed to be higher in EMPAC theater
//int gain = 17;  // 6 dB

std::string image_filename = "";
int brightness_threshold_seed = 25;
int brightness_threshold_cc = 15;

int min_ir_point_radius = 4;
int max_ir_point_radius = 10;
*/

bool detect = true;
int frame_count = 0;
std::vector<Image<byte>*> images;
std::vector<Image<sRGB>*> debug_images;

int since_last_print = 0;

#if 1
// image is 1280x940
int mask_min_row = 60; //120;
int mask_max_row = 790;
int mask_min_col = 40;
int mask_max_col = 1240;
#else
int mask_min_row = 0;
int mask_max_row = 940 - 1;
int mask_min_col = 0;
int mask_max_col = 1280 - 1;
#endif
 
std::ofstream *trail_ostr = NULL;
DirLock dirlock(IR_STATE_DIRECTORY);
#if __APPLE__
//std::vector<GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>*> cameras;
#else
std::vector<GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>*> cameras;
//changed by Tyler for Human Paintbrush
//std::vector<GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>*> cameras;
// *camera_a;
//GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8> *camera_b;
#endif


// ===============================
// helper functions
// ===============================
void parse_args(int argc, char **argv);
GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* setup_camera(unsigned long ip=0);
//changed by Tyler for Human Paintbrush
//GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* setup_camera(unsigned long ip);
void my_idle();


int total_msec = 0;
int total_frames = 0;
int dropped_frames = 0;
int max_msec = 0;
int min_msec = 100;

long timevaldiff(const struct timeval &starttime, const struct timeval &finishtime) {
  long msec;

  msec=(finishtime.tv_sec-starttime.tv_sec)*1000;
  msec+=(finishtime.tv_usec-starttime.tv_usec)/1000;

  return msec;
}


void printelapsedtime() {
  static timeval last_tval;
  timeval tval;  
  gettimeofday(&tval,NULL);
  //unsigned int sec = tval.tv_sec;
  //unsigned int usec = tval.tv_usec;
  int msec = timevaldiff(last_tval,tval);
  std::cout << "diff " << msec << std::endl;
  last_tval = tval;
}



// ===============================
// MAIN
// ===============================
int main(int argc, char **argv){
  parse_args(argc,argv);
#ifndef MYNDEBUG
  //  if (viewer) {
    viewer_start(argc,argv);    
    //}
#endif

  // delete all old micro images
#ifndef MYNDEBUG
    //if (save_images) {
    dirlock.Lock();

    std::stringstream ss;
    ss << "rm -f " << MICRO_IMAGE_PATH << "*.pgm " << MICRO_IMAGE_PATH << "*.hist";
    int failed = system(ss.str().c_str());
    assert (failed == 0);

    std::stringstream ss2;
    ss2 << "rm -f " << MICRO_IMAGE_PATH << "tagged*.ppm " << MICRO_IMAGE_PATH << "*.hist";
    failed = system(ss2.str().c_str());
    assert (failed == 0);


    dirlock.Unlock();
    //}
#endif

  if (image_filename == "") {

#if __APPLE__
    unsigned long camera_count = 0; 
#else
    unsigned long camera_count = GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>::INITIALIZE_CAMERAS();

    //unsigned long camera_count_1 = GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO16>::INITIALIZE_CAMERAS();
    //unsigned long camera_count_2 = GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER8>::INITIALIZE_CAMERAS();
    //unsigned long camera_count_3 = GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER16>::INITIALIZE_CAMERAS();
    //unsigned long camera_count_4 = GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_RGB24>::INITIALIZE_CAMERAS();
    //unsigned long camera_count_5 = GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_RGB48>::INITIALIZE_CAMERAS();

    std::cout << "CAMERA COUNT ON SYSTEM : " << camera_count << std::endl;
    //std::cout << "CAMERA COUNT ON SYSTEM1 : " << camera_count_1 << std::endl;
    //std::cout << "CAMERA COUNT ON SYSTEM2 : " << camera_count_2 << std::endl;
    //std::cout << "CAMERA COUNT ON SYSTEM3 : " << camera_count_3 << std::endl;
    //    std::cout << "CAMERA COUNT ON SYSTEM4 : " << camera_count_4 << std::endl;
    //std::cout << "CAMERA COUNT ON SYSTEM5 : " << camera_count_5 << std::endl;

    //exit(0);

    if (camera_count == 0) {
      std::cout << "ERROR! no cameras!" << std::endl;
    }

    while (1) {
      //  for (int i = 0; i < num_known_ips; i++) {
      
      GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* camera = setup_camera(0); //known_ips[i]);
      if (camera != NULL) {
	std::cout << "SUCCESS!" << std::endl;
	cameras.push_back(camera);
	ips.push_back(camera->getIP()); 
	//assert(cameras.size() == 1);  //known_ips[i]);
	//ips.push_back(known_ips[i]);
	images.push_back(new Image<byte>);
	//	debug_images.push_back(new Image<sRGB>(rows,cols));
	//break;
      } else {
	std::cout << "FAILURE" << std::endl;
	break;
      }
    }

    if (cameras.size() == 0) {
      std::cout << "failed to initialize any cameras" << std::endl;
      exit(0);
    }

#endif

    std::cout << "check " << images.size() << " " << camera_count << std::endl;

    if ((int)images.size() != (int)camera_count) {
      std::cout << "DID NOT LOAD ALL CAMERAS " << images.size() << " " << camera_count << std::endl;
      if (images.size() == 0) {
	std::cout << "NO CAMERAS LOADED" << std::endl;
	exit(0);
      }
      //exit(0);
    }
    //knmo
    //setup_camera(ip_a,camera_a);
    //setup_camera(ip_b,camera_b);

    //    if (camera_a != NULL) image_a = new Image<byte>;
    //if (camera_b != NULL) image_b = new Image<byte>;

#if __APPLE__
#else
    std::cout << "finished setup cameras " << cameras.size() << std::endl;
#endif

    if (camera_count == 0) {
      std::cout << "no cameras installed" << std::endl;
      exit(0);
    }

    //exit(0);

  }


#ifndef MYNDEBUG
    viewer_loop();
#else
    while(1) {
      my_idle();
    }
#endif
}



// ===============================
// PARSE ARGS
// ===============================
void parse_args(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    if (argv[i] == std::string("-shutter")) {
      assert (i+1 < argc);
      i++;
      shutter = atoi(argv[i]);
      /*}else if (argv[i] == std::string("-ip")) {
      assert (i+1 < argc);
      i++;
      ip_a = atol(argv[i]);
    */
    } else if (argv[i] == std::string("-gain")) {
      assert (i+1 < argc);
      i++;
      gain = atoi(argv[i]);
    } else if (argv[i] == std::string("-load_image")) {
      assert (i+1 < argc);
      i++;
      image_filename = argv[i];
    } else if (argv[i] == std::string("-save_trail")) {
      assert (i+1 < argc);
      i++;
      trail_ostr = new std::ofstream(argv[i]);
      if (trail_ostr->fail()) {
	std::cout << "ERROR could not open file for writing: " << argv[i] << std::endl;
	exit(0);
      }
      //    } else if (argv[i] == std::string("-viewer")) {
      //#ifdef MYNDEBUG
      //std::cout << "ERROR: MUST RECOMPILE W/O -DMYNDEBUG TO HAVE VIEWER" << std::endl;
      //exit(0);
      //#endif
      //save_images = true;
      // viewer = true;
    } else if (argv[i] == std::string("-no_detect")) {
      detect = false;
    } else if (argv[i] == std::string("-threshold")) {
      assert (i+1 < argc);
      i++;
      brightness_threshold_seed = atoi(argv[i]);
      brightness_threshold_cc = brightness_threshold_seed / 2;
      assert (brightness_threshold_cc >= 2 && brightness_threshold_seed < 255);
    } else if (argv[i] == std::string("-ir_point_radius")) {
      assert (i+2 < argc);
      i++;
      min_ir_point_radius = atoi(argv[i]);
      i++;
      max_ir_point_radius = atoi(argv[i]);
      assert (min_ir_point_radius > 0 && min_ir_point_radius < max_ir_point_radius);
    } else {
      std::cout << "ERROR: unknown command line arg: " << argv[i] << std::endl;
      exit(1);
    }    
  }
}

// ===============================
// SETUP CAMERA
// ===============================
//Changed by Tyler for Human Paintbrush
//GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* setup_camera(unsigned long ip) {
GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* setup_camera(unsigned long ip) {

#if __APPLE__
  return NULL;
#else  


  //int count

  std::cout << "in setup camera " << ip << std::endl;

	//Changed by Tyler for Human Paintbrush
	//GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* answer;
  GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>* answer;

  try {
    answer = new GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>(GigEVisionCameraTypes::CAMERA_GC1290M, ip);
		//answer = new GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8>(GigEVisionCameraTypes::CAMERA_GC1290M, ip);
		//Changed by Tyler for human paintbrush
  } 
  catch (unsigned long x) {
    std::cout << "catch!" <<std::endl;
    //assert (ip == x);
    std::cerr << "WARNING: unable to open camera " << ip << std::endl;     
    return NULL;
  }
  std::cout << "camera initialized" << std::endl;

  // preview mode
  //GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_MONO8> 
  // camera(GigEVisionCameraTypes::CAMERA_GC1290M);
  //GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER16>
  //  camera(GigEVisionCameraTypes::CAMERA_GC1290C);
  answer->SetBinning(1, 1);
  answer->SetROI(0, 0, 960, 1280);

  answer->SetExposure(shutter);
  answer->SetGain(gain);
  answer->InitContinuousCapture();
  
  int rows = -1;
  int cols = -1;
  // toss 10 frames at startup
  for (int i=0; i<10; i++){
    Image<byte> junk = answer->GetNextFrame();		
    rows = junk.getRows();
    cols = junk.getCols();
  }
  if (rows <= 0) exit(0);
  if (cols <= 0) exit(0);

  //  if (viewer) {
#ifndef MYNDEBUG
  //debug_image = new Image<sRGB>(rows,cols);
  //for (unsigned int i = 0; i < images.size(); i++) {
  debug_images.push_back(new Image<sRGB>(rows,cols));
  std::cout << "initialized debug image " << std::endl;
  // }
#endif
  //}
return answer;
#endif  

}


// ===============================
// ===============================
// ===============================
// ===============================

void capture_image() {
  if (image_filename == "") {
    // CAPTURE NEW IMAGE FROM CAMERA
#if __APPLE__
    std::cout << "ERROR: CAMERA NOT AVAILABLE FOR MAC!" << std::endl;
    exit(0);
#else  
    for (unsigned int i = 0; i < cameras.size(); i++) {
      assert (cameras[i] != NULL);
      *images[i] = cameras[i]->GetNextFrame();
    }
#endif
  } else {
    // LOAD IMAGE FROM FILE
    std::cout << "NEED TO UPDATE THIS CODE!" << std::endl;
    assert(0);
    exit(0);
    images[0]->load(image_filename.c_str());
    //debug_image = new Image<sRGB>(image.getRows(),image.getCols());
  }
  //  if (viewer) {
#if 0
  //#ifndef MYNDEBUG
  if (debug_image != NULL) {
    //std::cout << "initialize debug image" << std::endl;
    for (int row=0; row<image.getRows(); row ++) {
      for (int col=0; col<image.getCols(); col++) {	  
	(*debug_image)(row,col).r() = image(row,col);
	(*debug_image)(row,col).g() = image(row,col);
	(*debug_image)(row,col).b() = image(row,col);
	if (row < mask_min_row || row >= mask_max_row || col < mask_min_col || col >= mask_max_col) {
	  (*debug_image)(row,col).r() = 100;
	  (*debug_image)(row,col).g() = 100;
	}
      } 
    }
  }
#endif    
}


void save_image() {
  dirlock.Lock();
  assert (images.size() > 0);
  viewer_which_camera %= images.size();
  images[viewer_which_camera]->write(RAW_IR_IMAGE_FILENAME);
  dirlock.Unlock();
}


void write_images(std::vector<IR_Data_Point> &Points) {
    dirlock.Lock();
#if __APPLE__
#else
    for (unsigned int i = 0; i < cameras.size(); i++) {
      images[i]->write(DETECTED_IR_POINTS_IMAGE_FILENAME);
    }
#endif
    dirlock.Unlock();
    
    if (save_micro_image) {
      for (unsigned int i = 0; i < Points.size(); i++) {
	std::stringstream ss;
	ss << MICRO_IMAGE_PATH << "tagged_" << micro_image_count << "_frame_" << frame_count << "_point_" << i << ".pgm";
	
	Points[i].write_led_image(ss.str().c_str()); 
	std::cout << "SAVING" << ss.str().c_str() << std::endl;


	  std::stringstream ss2;
	  ss2 << MICRO_IMAGE_PATH << "tagged_" << micro_image_count << "_frame_" << frame_count << "_point_FC_" << i << ".ppm";
	  
	  Points[i].write_false_color_led_image(ss2.str().c_str()); 
	  std::cout << "SAVING" << ss2.str().c_str() << std::endl;


	  // Save out histogram here

	  std::stringstream hs;
	  hs << MICRO_IMAGE_PATH << "tagged_" << micro_image_count << "_frame_" << frame_count << "_point_" << i << "_histogram.hist";
	  std::ofstream out( hs.str().c_str() );
	  std::cout << "SAVING" << hs.str().c_str() << std::endl;
	  Points[ i ].print(out); 
	}
	std::cout << "micro_image request finished" << std::endl;
	micro_image_count++;
	save_micro_image = false;
      }


}

// ===============================
// IDLE (CAPTURE/LOAD IMAGE & DETECT LEDS
// ===============================

void my_idle() {

  LEDFinder finder;

  // =============
  // CAPTURE IMAGE
  capture_image();
  
  // =============
  // TIMING STUFF
  timeval tval;  
  static timeval last_tval;  
  gettimeofday(&tval,NULL);
  unsigned int sec = tval.tv_sec;
  unsigned int usec = tval.tv_usec;
  int msec = timevaldiff(last_tval,tval);
  if (msec > 100) {
    std::cout << "WARNING: diff msec = " << msec << std::endl;
    dropped_frames++;
  }
  if (msec >= 1000) {
    std::cout << "WARNING: msec > 10000" << msec << std::endl;
  }
  if (msec < 1000 && msec > 0) {
    //if (msec > 0) {
    //std::cout << "msec " << msec << "\n";
    total_frames++;
    min_msec = min(msec,min_msec);
    max_msec = max(msec,max_msec);
    total_msec += msec;
  }
  last_tval = tval;
  frame_count++;

  // =============
  // SAVE IMAGE
#ifndef MYNDEBUG
  save_image();
#endif  

  // =============
  // DETECT
  if (detect) {
    // find the IR POINT blobs
    //double height = 2.5019;
    //height = 0.;

    std::vector<IR_Data_Point> Points;
    for (unsigned int i = 0; i < images.size(); i++) {
      Image<sRGB> *debug_image = NULL; 
      if (debug_images.size() != 0) {
	debug_image = debug_images[i];
      }
      finder.FindLEDsPixelsOnly(ips[i],images[i], Points, debug_image,
				brightness_threshold_seed,brightness_threshold_cc,
				min_ir_point_radius,max_ir_point_radius,
				mask_min_row,mask_max_row,
				mask_min_col,mask_max_col
				);
    }

#ifndef MYNDEBUG
    write_images(Points);
#endif

    // write the IR POINTS to file
    dirlock.Lock();
    { /* SCOPE FOR ostr */
      std::ofstream ostr(FOUND_IR_POINTS_FILENAME);
      if (!ostr) {
	std::cout << "ERROR OPENING IR POINTS FILE FOR WRITING" << std::endl;
	exit(1);
      }
      ostr << "FRAME " << frame_count << "\n";
      ostr << "TIME " << sec << " " << usec << "\n";
      ostr << "NUM_POINTS " << Points.size() << "\n";
      since_last_print++;
      if (frame_count % 10 == 0 || since_last_print >= 10) {
	std::cout << "FOUND " << Points.size() << " ir points!   "; 
	std::cout << "avg msec per frame: " << std::fixed << std::setprecision(2) << total_msec / double(total_frames);
	std::cout << "   avg framerate: " << std::fixed << std::setprecision(2) << total_frames * 1000 / double(total_msec) << std::endl;
	//	std::cout << "   dropped frames " << std::fixed << std::setw(10) << dropped_frames << std::endl;
	since_last_print = 0;
      }

      for (unsigned i=0; i<Points.size(); i++){
	ostr << Points[i] << "\n";
      }

    } /* SCOPE for ostr */
    dirlock.Unlock();

  } else {
    if (frame_count%10 == 0) {
      std::cout << "tick" << std::endl;
    }
  }
  
  // LIVE IMAGE DISPLAY
#ifndef MYNDEBUG
  //  if (viewer) {
#if __APPLE__
#else
  assert (cameras.size() > 0); //_a != NULL);
  assert (images[0]->getRows() >= IMAGE_HEIGHT);
  assert (images[0]->getCols() >= IMAGE_WIDTH);
  ///assert (debug_images[viewer_which_image] != NULL);
  //#ifndef MYNDEBUG
  
  //image_to_texture(*debug_images[viewer_which_camera]);
  image_to_texture(*debug_images[viewer_which_camera]);
#endif
  //  }
#endif

  //std::cout << "ALL DONE IDLE" << std::endl;
}




