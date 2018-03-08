#include <Image.h>
#include <CalibratedCamera.h>

template <typename pixel_t>
Image<pixel_t> CorrectRadialDistortion(Image<pixel_t> &in, double u0, double v0,
				       double k1, double k2, double a0){
  Image <pixel_t> out(in.getRows(), in.getCols());
  double denom = a0;
  for (int row=0; row<out.getRows(); row++){
    double y = (row - v0) / denom;
    for (int col=0; col<out.getCols(); col++){
      double x = (col - u0) / denom;
      double r2 = (x*x + y*y);
      double r4 = r2 * r2;
      double u = col + (col - u0)*(k1 * r2 + k2 * r4);
      double v = row + (row - v0)*(k1 * r2 + k2 * r4);
      out(row, col) = in.bilinear(v, u);
    }
  }
  return(out);      
}

int main(int argc, char **argv){
  if (argc != 4){
    fprintf(stderr, 
	    "usage:\n undistort_image <input image>"
	    " <calibration.dat> <output image>\n");
    return -1;
  }


  CalibratedCamera camera;
  camera.loadCalibration(argv[2]);
  double a0 = camera.getA0();
  double u0 = camera.getCx();
  double v0 = camera.getCy();
  double k1 = camera.getK1();
  double k2 = camera.getK2();

  Image<sRGB> image(argv[1]);
  Image<sRGB> out = CorrectRadialDistortion(image, u0, v0, k1, k2, a0);
  out.write(argv[3]);
  
  return 0;
}
