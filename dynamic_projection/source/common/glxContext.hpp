#ifndef GLXCONTEXT_INCLUDED_
#define GLXCONTEXT_INCLUDED_

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <cassert>

//
// create a context for opengl calls without a display window
//  - creates a context on the default screen of the root window of the
//    specified display; defaults to display specified by $DISPLAY environment var
//
// usage: declare one of these to use off-screen rendering class
//        context will be closed on object destruction
//
// to-do: make this a singleton?
class glxContext {
 public:

  glxContext(const char *display_name = NULL){
    display = XOpenDisplay(display_name);
    assert(NULL != display);
    int attribList[] = {GLX_USE_GL, GLX_RGBA, None};
    XVisualInfo *visual_info = glXChooseVisual(display, DefaultScreen(display), attribList);
    assert(NULL != visual_info);
    context = glXCreateContext(display, visual_info, NULL, true);
    assert(NULL != context);
    glXMakeCurrent(display, RootWindow(display, DefaultScreen(display)), context);
  }

  ~glxContext(){
    glXDestroyContext(display, context);
    XCloseDisplay(display);
  }

 private:
  glxContext(const glxContext &);
  glxContext &operator=(const glxContext &);

  Display *display;
  GLXContext context;
};

#endif // #ifndef GLXCONTEXT_INCLUDED_
