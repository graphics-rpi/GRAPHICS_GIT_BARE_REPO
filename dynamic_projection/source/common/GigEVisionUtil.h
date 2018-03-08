#include <cstddef>

struct GigEVisionCameraTypes {
  typedef enum {CAMERA_ANY, CAMERA_GC1290M, CAMERA_GC1290C} CameraModel;
  static const char *display_names[];
};


struct GigEVisionPixelTypes {
  typedef enum {SAMPLE_MONO8, SAMPLE_MONO16, SAMPLE_BAYER8, 
		SAMPLE_BAYER16, SAMPLE_RGB24, SAMPLE_RGB48} SampleFormat;
  static const char *pixel_formats[];
};


// traits class for camera image pixel types
template<GigEVisionPixelTypes::SampleFormat format>
struct GigEVisionPixelTraits: public GigEVisionPixelTypes
{
  typedef GigEVisionPixelTypes::SampleFormat SampleFormat;
  const char *FormatString(){
    return NULL;
  }
  SampleFormat Format(){
    return format;
  }  
};
