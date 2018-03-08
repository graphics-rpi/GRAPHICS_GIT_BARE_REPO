#include "VimbaCamera.h"


int main(int argc, char** argv)
{
  //For color
  VimbaCamera<sRGB> camera;
  long long w, h;
  camera.getSensorWidthAndHeight(w,h);
  std::cout<<"width: "<<w<<" height: "<<h<<std::endl;
  //For BW (gets black and white image from color camera too!)
  //VimbaCamera<byte> camera;
  camera.setOffset(00,00);
  camera.setSize(1936,1456);
  if(argc==1)
    camera.takePicture("out2.ppm");
  else if(argc==2)
    camera.takePicture("out2.ppm", atoi(argv[1]));
  else if(argc==3)
    camera.takePicture(argv[1], atoi(argv[2]));
  else assert(0);
  
}
