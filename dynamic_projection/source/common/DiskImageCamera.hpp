//
// DiskImageCamera.hpp : Camera class reading images from disk files
//
#ifndef DISKIMAGECAMERA_HPP_INCLUDED_
#define DISKIMAGECAMERA_HPP_INCLUDED_

#include "CameraAPI.hpp"
#include <string.h>
#include <stdlib.h>

template<typename Pixel>
class DiskImageCamera: public CameraAPI<Pixel>
{
public:
  DiskImageCamera(const char *filename_template, int first_idx, int last_idx){
    int length = strlen(filename_template)+1;
    this->filename_template = new char[length];
    strncpy(this->filename_template, filename_template, length);
    this->first_idx = first_idx;
    this->last_idx = last_idx;
    current_idx = first_idx;
  }

  ~DiskImageCamera(){
    delete [] filename_template;
  }

  void Flush(){
    // meaningless for this camera
  }

  Image<Pixel> GetNextFrame(){
    const int MAX_FILENAME_LENGTH = 1024;
    char filename[MAX_FILENAME_LENGTH];
    snprintf(filename, MAX_FILENAME_LENGTH, filename_template, current_idx++);
    return Image<Pixel>(filename);
  }

  Image<Pixel> GetLatestFrame(){
    // same as above for this camera
    return GetNextFrame(); 
  }

private:
  char *filename_template;
  int first_idx;
  int last_idx;
  int current_idx;
};

#endif //#ifndef DISKIMAGECAMERA_HPP_INCLUDED_
