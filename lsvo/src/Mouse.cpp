
/*
 * Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property and proprietary
 * rights in and to this software, related documentation and any modifications thereto.
 * Any use, reproduction, disclosure or distribution of this software and related
 * documentation without an express license agreement from NVIDIA Corporation is strictly
 * prohibited.
 *
 * TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
 * AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
 * INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
 * SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
 * BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
 * INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES
 */

#if 1
#  if defined(__APPLE__)
#    include <GLUT/glut.h>
#  else
#    include <GL/glut.h>
#  endif
#else
#  error "You in big trouble without the GLUT."
#endif
#include <Mouse.hpp>
#include <optixu/optixu_matrix_namespace.h>
#include <iostream>
#include "camera_utils.hpp"



#define GLUT_MOTION 2


//-----------------------------------------------------------------------------
// 
// Helpers
//
//-----------------------------------------------------------------------------

using namespace optix;


  
  
//-----------------------------------------------------------------------------
// 
// Mouse definition 
//
//-----------------------------------------------------------------------------

Mouse::Mouse(PinholeCamera* camera, int xres, int yres)
  : camera(camera)
  , xres(xres)
  , yres(yres)
  , fov_speed(2)
  , dolly_speed(5)
  , translate_speed(33)
{
}

void Mouse::handleMouseFunc(int button, int state, int x, int y, int modifier)
{
  switch(state) {
  case GLUT_DOWN:
    current_interaction = InteractionState(modifier, button, state);
    call_func(x,y);
    break;
  case GLUT_UP:
    break;
  }
}

void Mouse::handleMoveFunc(int x, int y)
{
  current_interaction.state = GLUT_MOTION;
  call_func(x,y);
}

void Mouse::handlePassiveMotionFunc(int x, int y)
{
}

void Mouse::handleResize(int new_xres, int new_yres)
{
  xres = new_xres;
  yres = new_yres;
  camera->setAspectRatio(static_cast<float>(xres)/yres);
}

void Mouse::call_func(int x, int y)
{
  int modifier = current_interaction.modifier;
  int button   = current_interaction.button;
  if (modifier == 0                 && button == GLUT_LEFT_BUTTON)
    rotate(x, y);
  if (modifier == 0                 && button == GLUT_MIDDLE_BUTTON)
    translate(x, y);
  if (modifier == GLUT_ACTIVE_SHIFT && button == GLUT_RIGHT_BUTTON)
    fov(x, y);
  if (modifier == 0                 && button == GLUT_RIGHT_BUTTON)
    dolly(x, y);
}

void Mouse::fov(int x, int y)
{
  if(current_interaction.state == GLUT_MOTION) {
    float xmotion = (current_interaction.last_x - x)/static_cast<float>(xres);
    float ymotion = (current_interaction.last_y - y)/static_cast<float>(yres);
    float scale;
    if(fabsf(xmotion) > fabsf(ymotion))
      scale = xmotion;
    else
      scale = ymotion;
    scale *= fov_speed;

    if (scale < 0.0f)
      scale = 1.0f/(1.0f-scale);
    else
      scale += 1.0f;

    // Manipulate Camera
    camera->scaleFOV(scale);
  }
  current_interaction.last_x = x;
  current_interaction.last_y = y;
}


void Mouse::translate(int x, int y)
{
  if(current_interaction.state == GLUT_MOTION) {
    float xmotion =  float(current_interaction.last_x - x)/xres;
    float ymotion = -float(current_interaction.last_y - y)/yres;
    float2 translation = make_float2(xmotion, ymotion) * translate_speed;

    camera->translate(translation);
  }
  current_interaction.last_x = x;
  current_interaction.last_y = y;
}


void Mouse::dolly(int x, int y)
{
  if(current_interaction.state == GLUT_MOTION) {
    float xmotion = -float(current_interaction.last_x - x)/xres;
    float ymotion = -float(current_interaction.last_y - y)/yres;

    float scale;
    if(fabsf(xmotion) > fabsf(ymotion))
      scale = xmotion;
    else
      scale = ymotion;
    scale *= dolly_speed;

    camera->dolly(scale);
  }
  current_interaction.last_x = x;
  current_interaction.last_y = y;
}

void Mouse::rotate(int x, int y)
{

  float xpos = 2.0f*static_cast<float>(x)/static_cast<float>(xres) - 1.0f;
  float ypos = 1.0f - 2.0f*static_cast<float>(y)/static_cast<float>(yres);

  if ( current_interaction.state == GLUT_DOWN ) {
    
    current_interaction.rotate_from = projectToSphere( xpos, ypos, 0.8f );

  } else if(current_interaction.state == GLUT_MOTION) {

    float3 to( projectToSphere( xpos, ypos, 0.8f ) );
    float3 from( current_interaction.rotate_from );
  
    Matrix4x4 m = rotationMatrix( to, from);
    current_interaction.rotate_from = to;
    camera->transform( m ); 
  }
  current_interaction.last_x = x;
  current_interaction.last_y = y;

}

void Mouse::track_and_pan(int x, int y)
{
}

void Mouse::track(int x, int y)
{
}

void Mouse::pan(int x, int y)
{
}

