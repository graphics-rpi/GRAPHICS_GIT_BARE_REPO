#ifndef _LI_PREREQUISITES_HXX_
#define _LI_PREREQUISITES_HXX_

#ifdef CONTRAPTION

#include "LiException.hxx"

#include <cassert>
#include <iostream>

namespace LI {
  typedef double Real;

  // Forward declaration of classes
  class FatalRuntimeError;
  class OpenGLCamera;
  class Projector;
  class ProjectorManager;
  class ThreadDispatcher;
  class Vector3;
  class Window;

  struct XCxn;
  struct WindowAttribs;
}

#endif

#endif
