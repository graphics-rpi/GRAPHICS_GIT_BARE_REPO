#include "camera.h"
#include "canvas.h"
#include "argparser.h"

#include "../../common/glm/glm/gtc/matrix_transform.hpp"
#include "../../common/glm/glm/gtx/string_cast.hpp"

Camera::Camera(ArgParser* args){
    aspect_ratio_ = (float)args->width_ / (float)args->height_;
    fov_ = 45.0f;
    near_plane_ = 0.1f;
    far_plane_ = 100.0f;
    position_ = glm::vec3(0.0f,0.0f,3.0f);
    target_ = position_ + glm::vec3( 0.0f, 0.0f, -3.0f );
    up_ = glm::vec3(0.0f, 1.0f, 0.0f);

    rotation_ = glm::mat4(1.0f);
    view_ = glm::lookAt(position_, target_, up_);
    projection_ = glm::perspective(fov_, aspect_ratio_, near_plane_, far_plane_);

    args_ = args; 
}

void Camera::Move(const glm::vec3 & input){
    // Modify the position and target
    position_ += input;
    target_ += input;

    // update view matrix
    view_ = glm::lookAt(position_, target_, up_);
    return;
}

glm::mat4 Camera::ComputeMVPMatrix( const glm::mat4 & model ){
    glm::mat4 model_view_projection = projection_ * view_ * model;
    return model_view_projection;
}

glm::vec3 Camera::ComputeScreenToWorld( int mouse_x, int mouse_y ){
    GLfloat mouse_z;

    glReadPixels(mouse_x, mouse_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mouse_z);
    Canvas::HandleGLError("glReadPixels");

    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, args_->width_, args_->height_);
    glm::vec3 screen_pos = glm::vec3((float)mouse_x , (float)(args_->height_ - mouse_y), mouse_z);


    glm::vec3 world_pos = glm::unProject(screen_pos, view_, projection_, viewport);
    std::cout << "SCREEN_POS: " << glm::to_string(screen_pos) << std::endl;
    std::cout << "WORLD_POS: " << glm::to_string(world_pos) << std::endl;

    return world_pos;
}
