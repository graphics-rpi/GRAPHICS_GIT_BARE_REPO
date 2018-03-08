//
// Camera.hpp : abstract base class defining generic camera API
// created: 3/21/09/tcy
//
#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED
#include "Image.h"

template <typename Pixel>
class CameraAPI
{
public:
  CameraAPI(){};
  virtual ~CameraAPI(){};
  // Flush() discards any existing images in the camera pipeline
  virtual void Flush() = 0;
  // GetNextFrame() returns the next image in the camera pipeline
  virtual Image<Pixel> GetNextFrame() = 0;
  // GetLatestFrame() returns an image guaranteed to have been taken after
  //  GetLatestFrame() was called; some frames make be discarded 
  virtual Image<Pixel> GetLatestFrame() = 0;
};
#endif //#ifndef CAMERA_HPP_INCLUDED
