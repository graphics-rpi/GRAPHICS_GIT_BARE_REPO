#include <stdio.h>
#include <stdlib.h>
#include <CalibratedCamera.h>
#include <Matrix3.h>

int main(int argc, char **argv){
  if (argc != 5){
    fprintf(stderr, 
            "usage: %s <camera calibration> <near>"
            " <far> <opengl camera output>\n", 
            argv[0]);
    exit(-1);
  }

  CalibratedCamera cam;
  //cam.loadAdjustedCalibration(argv[1]);
  cam.loadCalibration(argv[1]);
  double near = atof(argv[2]);
  double far = atof(argv[3]);
  cam.writeOpenGLCamera(argv[4], near, far);
  return 0;
}
