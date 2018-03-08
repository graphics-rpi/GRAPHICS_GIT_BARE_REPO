#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "../../common/glm/glm/glm.hpp"

class ArgParser;

class Camera{
public:
  // Constructor
  Camera(ArgParser* args);

  // Accessors
  glm::mat4 rotation() const {return rotation_;}
  glm::mat4 view() const {return view_;}
  glm::mat4 projection() const {return projection_;}

  void set_rotation( const glm::mat4 & rotation ){ rotation_ = rotation; };
  void set_view( const glm::mat4 & view ){ view_ = view; };
  void set_projection( const glm::mat4 & projection ){ projection_ = projection; };

  void Move(const glm::vec3 & input);

  // Helper functions
  glm::mat4 ComputeMVPMatrix( const glm::mat4 & model );
  glm::vec3 ComputeScreenToWorld( int mouse_x, int mouse_y );

private:
  float aspect_ratio_;
  float fov_;
  float near_plane_;
  float far_plane_;
  glm::vec3 position_;
  glm::vec3 target_;
  glm::vec3 up_;

  glm::mat4 rotation_;
  glm::mat4 view_;
  glm::mat4 projection_;

  ArgParser* args_;
};


#endif
