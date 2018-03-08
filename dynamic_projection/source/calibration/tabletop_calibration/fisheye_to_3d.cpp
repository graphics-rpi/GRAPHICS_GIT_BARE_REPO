//screen is 5.2578 meters from origin in Y
#include <FisheyeCamera.hpp>
#include <Vector3.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv){
  if (argc != 5){
    fprintf(stderr,
	    "usage: ./fisheye_to_3d input_file output_file xyorz_co-ordinate axis-to-fix");
    return -1;
  }
  double row,col, r,c;
  char axis_to_fix;
  axis_to_fix=(argv[4])[0];
  printf("axis to fix %c\n", axis_to_fix);
  FILE *fp_in = fopen(argv[1], "rt");
  if (NULL == fp_in){
    FATAL_ERROR("cannot open %s", argv[1]);
  }

  FILE *fp_out = fopen(argv[2], "wt");
  if (NULL == fp_out){
    FATAL_ERROR("cannot open %s", argv[2]);
  }
  FisheyeCamera camera("color_camera_calibration.dat");

  while(EOF != fscanf(fp_in, "%lf %lf %lf %lf", &r, &c, &row, &col)){
    v3d point;
    switch(axis_to_fix)
    {
      case 'x':
      case 'X':
        point= camera.PixelToWorldFromX(row, col, atof(argv[3]));
        break;
      case 'y':
      case 'Y':
        point = camera.PixelToWorldFromY(row, col, atof(argv[3]));
        break;
      case 'z':
      case 'Z':
        point = camera.PixelToWorldFromZ(row, col, atof(argv[3]));
        break;
    }
    fprintf(fp_out,"%f %f %f %f %f\n",
             point.x(), 
             point.y(), 
             point.z(),
	    c, r);
  }

  fclose(fp_in);
  fclose(fp_out);


}


