#ifndef HDR_IMAGE_INCLUDED_
#define HDR_IMAGE_INCLUDED_

#include <cassert>
#include <Image.h>
#include <cstdio>
#include <Matrix3.h>

class HDRImage {
public:
  HDRImage(){
    initialized = false;
  }

  HDRImage(const char *filename){
    image.read(filename);
    weight = Image<double>(image.getRows(), image.getCols());
    for (int row=0; row<weight.getRows(); row++){
      for (int col=0; col<weight.getCols(); col++){
	weight(row, col) = 1.;
      }
    }
    initialized = true;
  }

  // add an LDR image
  void addImage(Image<sRGB> & in, double exposure){
    if (! initialized){
      image = Image<v3d>(in.getRows(), in.getCols());
      weight = Image<double>(in.getRows(), in.getCols());
      for (int row=0; row<in.getRows(); row++){
	for (int col=0; col<in.getCols(); col++){
	  double w = PixelWeight(in(row, col), exposure);
	  v3d lin = convertVector3(in(row, col), 0.)/255.;
	  lin.apply(sRGBdegamma);
          lin /= exposure;
	  image(row, col) = w * lin;
	  weight(row, col) = w;
	}
      }
      initialized = true;
    } else {
      assert(image.getRows() == in.getRows());
      assert(image.getCols() == in.getCols());
      for (int row=0; row<in.getRows(); row++){
	for (int col=0; col<in.getCols(); col++){
	  double w = PixelWeight(in(row, col), exposure);
	  v3d lin = convertVector3(in(row, col), 0.)/255.;
	  lin.apply(sRGBdegamma);
          lin /= exposure;
	  image(row, col) += w * lin;
	  weight(row, col) += w;
	}
      }
    }
  }

  void write(const char *filename){
    assert(initialized);
    Image<v3d> out(image.getRows(), image.getCols());
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	if (weight(row,col) > 0){
	  out(row,col) = image(row,col)/weight(row, col);
	} else {
	  out(row, col) = v3d(0., 0., 0.);
	}
      }
    }
    out.write(filename);
  }

  Image<sRGB> getLDR(double exposure){
    m3d M = m3d::identity();
    return getLDR(exposure, M);
  }

  Image<sRGB> getLDR(double exposure, const m3d &M){
    assert(initialized);
    Image<sRGB> ldr(image.getRows(), image.getCols());
    for (int row=0; row<image.getRows(); row++){
      for (int col=0; col<image.getCols(); col++){
	v3d val = v3d(0., 0., 0.);
	if (weight(row,col) > 0){
	  val = image(row,col)/weight(row, col);
	}
	val = exposure * (M * val);
	val.apply(sRGBclamp).apply(sRGBgamma).apply(sRGB255);
	ldr(row, col) = convertVector3(val, byte(0));
      }
    }
    return ldr;
  }

private:
  bool initialized;
  Image<v3d> image;
  Image<double> weight;

  double PixelWeight(const sRGB &val, double exposure){
    //return (val.min()/255. * (1. - val.max()/255.))/exposure;
    //    return (val.max() < 255 ? 1. : 0.) * val.max()/255.;

    // assumes additive Gaussian white noise
    int min_val = 10;
    int max_val = 10;
    return (val.max() > min_val ? 1.: double(val.max())/min_val) *
      (val.max() < (255-max_val) ? 1. : (255.-val.max())/max_val);
  }
};

#endif // #ifndef HDR_IMAGE_INCLUDED_
