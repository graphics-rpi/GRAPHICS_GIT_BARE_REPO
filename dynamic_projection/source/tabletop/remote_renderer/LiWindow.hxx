#ifndef _LI_WINDOW_HXX_
#define _LI_WINDOW_HXX_

#ifdef CONTRAPTION

#include "LiPrerequisites.hxx"
#include "LiWindowAttribs.hxx"

#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <boost/shared_array.hpp>
#include <sstream>
#include <string>

namespace LI {
  struct XCxn {
    explicit XCxn(unsigned int server  = 0,
                  unsigned int screen  = 0,
                  const char* hostname = "");

    std::string mHostname;
    unsigned int mServer;
    unsigned int mScreen;
  };

  class Window {
  public:
    explicit Window(const char* title,
                    const XCxn& connection,
                    const WindowAttribs& attribs);
    virtual ~Window();

    //! The thread of execution that calls this command sets its context to this window.
    //! This should be called by the init callback.
    void bindContext() const;
    //! Blocks until GL execution is complete.
    void flushGL() const;
    //! Non-blocking call that pushes all GL commands through and updates the display.
    void flushDisplay() const;
    void raiseWindow() const;
    //! Processes XEvents from the XServer.
    //! Should be called by the run callback.
    bool processEvents() const;

    std::string getTitle() const;
    const XCxn& getConnection() const;
    const WindowAttribs& getAttributes() const;

  protected:
    std::string mTitle;
    XCxn mCxn;
    WindowAttribs mAttribs;

    bool mDirectContext;
    mutable bool mDestroyed;
    ::Window mWindow; //!< This is a GLX Window, not a LI::Window.
    Display* mDisplay;
    Atom wmDeleteWindow;
    GLXContext mContext;
    SizeID mOldVideoMode;

  private:
    bool makeFullscreen();
    void destroy() const;
  };

  // Inline definitions below.














































































  inline XCxn::XCxn(unsigned int server,
                    unsigned int screen,
                    const char* hostname)
    : mHostname(hostname),
      mServer(server),
      mScreen(screen) {}

  inline Window::Window(const char* title,
                        const XCxn& cxn,
                        const WindowAttribs& attribs)
    : mTitle(title),
      mCxn(cxn),
      mAttribs(attribs) {
    // Create display string
    std::stringstream ss;
    ss << cxn.mHostname <<  ":" << cxn.mServer << "." << cxn.mScreen;

    mDisplay = XOpenDisplay(ss.str().c_str());
    if(mDisplay == NULL) {
      throw FatalRuntimeError((std::string("Failed to open XDisplay: ") + ss.str()).c_str());
    }

    if(!glXQueryExtension(mDisplay, NULL, NULL)) {
      throw FatalRuntimeError("Could not query GLX version");
    }

    // Try to obtain the desired visual
    boost::shared_array<int> attrib_list(mAttribs.getAttribList());
    XVisualInfo* visualInfo = glXChooseVisual(mDisplay, mCxn.mScreen, attrib_list.get());
    if(visualInfo == NULL) {
      throw HardwareError("Could not obtain desired visual");
    }

    ::Window root = RootWindow(mDisplay, mCxn.mScreen);
    unsigned long mask = CWBorderPixel | CWColormap | CWEventMask;
    XSetWindowAttributes attr;
    attr.border_pixel = 0;
    attr.colormap = XCreateColormap(mDisplay, root, visualInfo->visual, AllocNone);
    attr.override_redirect = True;

    //// IMPORTANT
    // This determines what events get generated
    // We may want to include this somehow as a parameter
    attr.event_mask = StructureNotifyMask | ExposureMask;

    mWindow = XCreateWindow(mDisplay, root, 0, 0,
                            mAttribs.mWidth, mAttribs.mHeight,
                            0, visualInfo->depth, InputOutput, visualInfo->visual, mask, &attr);
    if(!mWindow) {
      throw FatalRuntimeError("Could not create XWindow");
    }

    if(mAttribs.mFullscreen) {
      bool success = makeFullscreen();
      if(!success) {
        throw FatalRuntimeError("Could not make fullscreen");
      }
    }

    // Hide the cursor by creating a transparent 1x1 pixmap
    Pixmap cursor_pixmap = XCreatePixmap(mDisplay, mWindow, 1, 1, 1);
    GC graphic_context = XCreateGC(mDisplay, cursor_pixmap, 0, NULL);
    XDrawPoint(mDisplay, cursor_pixmap, graphic_context, 0, 0);
    XFreeGC(mDisplay, graphic_context);

    // Create the cursor, using the pixmap as both the shape and the mask of the cursor
    XColor color;
    color.flags = DoRed | DoGreen | DoBlue;
    color.red = color.blue = color.green = 0;
    Cursor mHiddenCursor = XCreatePixmapCursor(mDisplay, cursor_pixmap, cursor_pixmap, &color, &color, 0, 0);

    // We don't need the pixmap any longer, free it
    XFreePixmap(mDisplay, cursor_pixmap);
    XDefineCursor(mDisplay, mWindow, mHiddenCursor);

    XSizeHints hints;
    hints.width = mAttribs.mWidth;
    hints.height = mAttribs.mHeight;
    hints.flags = USSize;
    XSetNormalHints(mDisplay, mWindow, &hints);
    XSetStandardProperties(mDisplay, mWindow, title, title, None, (char**)NULL, 0, &hints);
    wmDeleteWindow = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, mWindow, &wmDeleteWindow, 1);

    mContext = glXCreateContext(mDisplay, visualInfo, None, True);
    if(mContext == NULL) {
      throw FatalRuntimeError();
    }
    mDirectContext = glXIsDirect(mDisplay, mContext);

    XMapWindow(mDisplay, mWindow);
    XFree(visualInfo);
  }

  inline Window::~Window() {
#ifdef EBUG
    std::cerr << "Window dtor." << std::endl;
#endif
    destroy();
  }

  inline void Window::bindContext() const {
    if(!glXMakeContextCurrent(mDisplay, mWindow, mWindow, mContext)) {
      throw FatalRuntimeError("Failed to bind context");
    }
  }

  inline void Window::flushGL() const {
    glXWaitGL(); // or glFinish();
  }

  inline void Window::flushDisplay() const {
    if(mAttribs.mDoubleBuffered) {
      glXSwapBuffers(mDisplay, mWindow);
    } else {
      glFlush();
    }
  }

  inline void Window::raiseWindow() const {
    XRaiseWindow(mDisplay, mWindow);
  }

  inline bool Window::processEvents() const {
    bool keep_running = true;
    XEvent evt;

    while(XPending(mDisplay) > 0) {
      XNextEvent(mDisplay, &evt);
      switch(evt.type) {
//       case KeyPress: {
//         KeySym keycode = XLookupKeysym(&evt.xkey, 0);
//         switch(keycode) {
//         default:
//           break;
//         }
//         break;
//       }
//       case KeyRelease: {
//         KeySym keycode = XLookupKeysym(&evt.xkey, 0);
//         switch(keycode) {
//         default:
//           break;
//         }
//         break;
//       }
      case ClientMessage:
        if(static_cast<unsigned int>(evt.xclient.data.l[0]) == wmDeleteWindow) {
#ifdef EBUG
          std::cerr << "WM_DELETE_WINDOW message sent. Time to close shop." << std::endl;
#endif
          return false;
        }
        break;

      default:
        break;
      }
    }

    return keep_running;
  }

  inline std::string Window::getTitle() const {
    return std::string(mTitle);
  }

  inline const XCxn& Window::getConnection() const {
    return mCxn;
  }

  inline const WindowAttribs& Window::getAttributes() const {
    return mAttribs;
  }

  inline bool Window::makeFullscreen() {
    int version;
    ::Window root = RootWindow(mDisplay, mCxn.mScreen);
    if(XQueryExtension(mDisplay, "RANDR", &version, &version, &version)) {
      XRRScreenConfiguration* config = XRRGetScreenInfo(mDisplay, root);
      if(config) {
        Rotation current_rotation;
        mOldVideoMode = XRRConfigCurrentConfiguration(config, &current_rotation);
        int num_sizes;
        XRRScreenSize* sizes = XRRConfigSizes(config, &num_sizes);
        if(sizes && num_sizes > 0) {
          for(int i = 0; i < num_sizes; ++i) {
            if(sizes[i].width == static_cast<int>(mAttribs.mWidth) && sizes[i].height == static_cast<int>(mAttribs.mHeight)) {
              XRRSetScreenConfig(mDisplay, config, root, i, current_rotation, CurrentTime);
              XRRFreeScreenConfigInfo(config);
              return true;
            }
          }
        }
        XRRFreeScreenConfigInfo(config);
      }
    }
    return false;
  }

  inline void Window::destroy() const {
    if(mAttribs.mFullscreen) {
      XRRScreenConfiguration* config = XRRGetScreenInfo(mDisplay, RootWindow(mDisplay, mCxn.mScreen));
      if(config) {
        Rotation current_rotation;
        XRRConfigCurrentConfiguration(config, &current_rotation);
        XRRSetScreenConfig(mDisplay, config, RootWindow(mDisplay, mCxn.mScreen), mOldVideoMode, current_rotation, CurrentTime);
        XRRFreeScreenConfigInfo(config);
      }
    }
    glXMakeCurrent(mDisplay, None, NULL);
    glXDestroyContext(mDisplay, mContext);
    XDestroyWindow(mDisplay, mWindow);
    XCloseDisplay(mDisplay);
    mDestroyed = true;
  }
}

#endif

#endif
