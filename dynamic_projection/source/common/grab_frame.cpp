#include <cstdlib>
#include <iostream>
#ifdef FIREWIRE_CAMERA
#include <PointGreyCamera.hpp>
#endif // #ifdef FIREWIRE_CAMERA

#ifdef GIGE_CAMERA
#include "GigEVisionCamera.hpp"
#endif // #ifdef GIGE_CAMERA

#include <cstdio>

int main(int argc, char **argv){

  std::cout << "IN GRAB FRAME REVISED" << std::endl;
  if (argc < 2 || argc > 6){
    fprintf(stderr, 
	    "grab_frame <output.ppm> [shutter] [gain] [blue_bal] [red_bal]\n");
    exit(-1);
  }

  std::cout << " want to save to " << argv[1] << std::endl;


#ifdef GIGE_CAMERA
  GigEVisionCamera<GigEVisionPixelTypes::SAMPLE_BAYER16>
    camera;
  long shutter = 800000;  
  int gain = 0;
  int red_bal = 148;
  int blue_bal = 195;

  if (argc > 2){
    shutter = atoi(argv[2]);
  }
  if (argc > 3){
    gain = atoi(argv[3]);
  }
  if (argc > 4){
    blue_bal = atoi(argv[4]);
  }
  if (argc > 5){
    red_bal = atoi(argv[5]);
  }
  int rows = 1456;
  int cols = 1936;
  int row_offset = 0;
  int col_offset = 0;
  camera.SetExposure(shutter);
  camera.SetGain(gain);
  camera.SetWhiteBalance(red_bal, blue_bal);  // red, blue
  camera.SetBinning(1, 1);
  camera.SetROI(row_offset, col_offset, rows, cols);
  camera.InitContinuousCapture();

  Image<Vector3<uint16_t> > image;

  for (int i=0; i<3; i++){
    image = camera.GetNextFrame();
  }

  Image<sRGB> out(image.getRows(), image.getCols());
  for (int row=0; row<out.getRows(); row++){
    for (int col=0; col<out.getCols(); col++){
      out(row, col) = sRGB(image(row, col)/uint16_t(16));
    }
  }

  out.write(argv[1]);
  std::cout << " saved to " << argv[1] << std::endl;

#endif // #ifdef GIGE_CAMERA

#ifdef FIREWIRE_CAMERA
  int rows = 1032;
  int cols = 1392;
  int row_offset = 0;
  int col_offset = 0;
  PointGreyCamera<sRGB> camera(col_offset, row_offset, cols, rows);
  // defaults for just fluorescent tubes + "blanked" projectors
  int shutter = 900;
  int gain = 250;
  int blue_bal = 605;
  int red_bal = 615;

  if (argc > 2){
    shutter = atoi(argv[2]);
  }
  if (argc > 3){
    gain = atoi(argv[3]);
  }
  if (argc > 4){
    blue_bal = atoi(argv[4]);
  }
  if (argc > 5){
    red_bal = atoi(argv[5]);
  }

  camera.SetShutter(shutter);
  camera.SetGain(gain);
  camera.SetGamma(1024);
  camera.SetWhiteBalance(blue_bal, red_bal); // blue, red
  camera.SetBrightness(255);
  Image <Vector3<double> > sum(rows, cols);
  
  Image <sRGB> image;
  for (int i=0; i<3; i++){
    image = camera.GetNextFrame();
  }
  for (int row=0; row<sum.getRows(); row++){
    for (int col=0; col<sum.getCols(); col++){
      sum(row, col) = 0.;
    }
  }

  int num_images = 1;
  for (int i=0; i<num_images; i++){
    usleep(rand()%1000);
    Image <sRGB> image = camera.GetNextFrame();
    for (int row=0; row<sum.getRows(); row++){
      for (int col=0; col<sum.getCols(); col++){
	v3d pix =  convertVector3(image(row, col), 0.);
	sum(row, col) += pix;
      }
    }
  }

  Image <sRGB> out(rows, cols);
  for (int row=0; row<sum.getRows(); row++){
    for (int col=0; col<sum.getCols(); col++){
      out(row, col) = convertVector3(sum(row,col)*(1./num_images), byte(0));
    }
  }
   
  out.write(argv[1]);
  std::cout << " saved to " << argv[1] << std::endl;
#endif

  std::cout << "done withGRAB FRAME REVISED" << std::endl;

  return 0;
}
