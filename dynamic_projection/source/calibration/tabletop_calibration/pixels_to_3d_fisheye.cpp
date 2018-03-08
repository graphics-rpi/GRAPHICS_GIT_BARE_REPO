#include <stdio.h>
#include <stdlib.h>
#include <CalibratedCamera.h>

int main(int argc, char **argv){
  CalibratedCamera camera;
  camera.loadCalibration("camera_images/camera_calibration.dat");
  v3d center = camera.getCenter();

  FILE *fp = fopen(argv[1], "rt");
  if (NULL == fp){
    FATAL_ERROR("cannot open %s", argv[1]);
  }

  double height = atof(argv[2]);
  double x_offset = atof(argv[3]);
  double y_offset = atof(argv[4]);
  double z_offset = atof(argv[5]);

  double row, col;
  double r, c;
  while(EOF != fscanf(fp, "%lf %lf %lf %lf", &r, &c, &row, &col)){
    v3d point = camera.PointFromPixel(row, col, height);
    printf("%f %f %f %f %f\n",
           point.x()+x_offset, 
           point.y()+y_offset, 
           point.z()+z_offset,
	   c, r);

    // test reprojection
    //v3d pixel = camera.PixelFromPoint(point);
    //double dy = pixel.y() - row;
    //double dx = pixel.x() - col;
    //double err = sqrt(dx*dx+dy*dy);
    // fprintf(stderr, "error = %f\n", err);
  }

  fclose(fp);
  return 0;
}
