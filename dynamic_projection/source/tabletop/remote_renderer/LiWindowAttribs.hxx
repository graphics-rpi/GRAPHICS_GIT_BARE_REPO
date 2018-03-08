#ifndef _LI_WINDOW_ATTRIBS_HXX_
#define _LI_WINDOW_ATTRIBS_HXX_

#ifdef CONTRAPTION

#include "LiPrerequisites.hxx"

#include <boost/shared_array.hpp>
#include <GL/glx.h>

namespace LI {
  struct WindowAttribs {
    explicit WindowAttribs(unsigned int width,
                           unsigned int height,
                           bool fullscreen      = false,
                           bool double_buffered = true,
                           unsigned int rgb     = 8,
                           unsigned int depth   = 24,
                           unsigned int stencil = 0,
                           unsigned int accum   = 0);

    explicit WindowAttribs(unsigned int width,
                           unsigned int height,
                           bool fullscreen,
                           bool double_buffered,
                           unsigned int r,
                           unsigned int g,
                           unsigned int b,
                           unsigned int depth   = 24,
                           unsigned int stencil = 0,
                           unsigned int accum   = 0);

    explicit WindowAttribs(unsigned int width,
                           unsigned int height,
                           bool fullscreen,
                           bool double_buffered,
                           unsigned int r,
                           unsigned int g,
                           unsigned int b,
                           unsigned int a,
                           unsigned int depth   = 24,
                           unsigned int stencil = 0,
                           unsigned int accum   = 0);

    WindowAttribs& setWidth(unsigned int pixels);
    WindowAttribs& setHeight(unsigned int pixels);
    WindowAttribs& setFullscreen(bool fullscreen);
    WindowAttribs& useDoubleBuffer();
    WindowAttribs& useSingleBuffer();
    WindowAttribs& setRGB(unsigned int bits);
    WindowAttribs& setRGBA(unsigned int bits);
    WindowAttribs& setR(unsigned int bits);
    WindowAttribs& setG(unsigned int bits);
    WindowAttribs& setB(unsigned int bits);
    WindowAttribs& setA(unsigned int bits);
    WindowAttribs& setStencil(unsigned int bits);
    WindowAttribs& setAccum(unsigned int bits);

    boost::shared_array<int> getAttribList() const;

    bool mFullscreen;
    bool mDoubleBuffered;
    unsigned int mWidth;
    unsigned int mHeight;
    unsigned int mRGBA[4];
    unsigned int mDepth;
    unsigned int mStencil;
    unsigned int mAccum;

  private:
    WindowAttribs();
  };

  // Inline definitions below.














































































  inline WindowAttribs::WindowAttribs(unsigned int width,
                                      unsigned int height,
                                      bool fullscreen,
                                      bool double_buffered,
                                      unsigned int rgb,
                                      unsigned int depth,
                                      unsigned int stencil,
                                      unsigned int accum)
    : mFullscreen(fullscreen),
      mDoubleBuffered(double_buffered),
      mWidth(width),
      mHeight(height),
      mDepth(depth),
      mStencil(stencil),
      mAccum(accum) {
    mRGBA[0] = mRGBA[1] = mRGBA[2] = rgb;
    mRGBA[3] = 0;
  }

  inline WindowAttribs::WindowAttribs(unsigned int width,
                                      unsigned int height,
                                      bool fullscreen,
                                      bool double_buffered,
                                      unsigned int r,
                                      unsigned int g,
                                      unsigned int b,
                                      unsigned int depth,
                                      unsigned int stencil,
                                      unsigned int accum)
    : mFullscreen(fullscreen),
      mDoubleBuffered(double_buffered),
      mWidth(width),
      mHeight(height),
      mDepth(depth),
      mStencil(stencil),
      mAccum(accum) {
    mRGBA[0] = r;
    mRGBA[1] = g;
    mRGBA[2] = b;
    mRGBA[3] = 0;
  }

  inline WindowAttribs::WindowAttribs(unsigned int width,
                                      unsigned int height,
                                      bool fullscreen,
                                      bool double_buffered,
                                      unsigned int r,
                                      unsigned int g,
                                      unsigned int b,
                                      unsigned int a,
                                      unsigned int depth,
                                      unsigned int stencil,
                                      unsigned int accum)
    : mFullscreen(fullscreen),
      mDoubleBuffered(double_buffered),
      mWidth(width),
      mHeight(height),
      mDepth(depth),
      mStencil(stencil),
      mAccum(accum) {
    mRGBA[0] = r;
    mRGBA[1] = g;
    mRGBA[2] = b;
    mRGBA[3] = a;
  }

  inline WindowAttribs& WindowAttribs::setWidth(unsigned int pixels) {
    mWidth = pixels;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setHeight(unsigned int pixels) {
    mHeight = pixels;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setFullscreen(bool fullscreen) {
    mFullscreen = fullscreen;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::useDoubleBuffer() {
    mDoubleBuffered = true;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::useSingleBuffer() {
    mDoubleBuffered = false;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setRGB(unsigned int bits) {
    mRGBA[0] = mRGBA[1] = mRGBA[2] = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setRGBA(unsigned int bits) {
    mRGBA[0] = mRGBA[1] = mRGBA[2] = mRGBA[3] = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setR(unsigned int bits) {
    mRGBA[0] = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setG(unsigned int bits) {
    mRGBA[1] = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setB(unsigned int bits) {
    mRGBA[2] = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setA(unsigned int bits) {
    mRGBA[3] = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setStencil(unsigned int bits) {
    mStencil = bits;
    return *this;
  }

  inline WindowAttribs& WindowAttribs::setAccum(unsigned int bits) {
    mAccum = bits;
    return *this;
  }

  inline boost::shared_array<int> WindowAttribs::getAttribList() const {
    boost::shared_array<int> attrib_list(new int[23]);

    attrib_list[0]  = GLX_RGBA;
    attrib_list[1]  = GLX_RED_SIZE;
    attrib_list[3]  = GLX_GREEN_SIZE;
    attrib_list[5]  = GLX_BLUE_SIZE;
    attrib_list[7]  = GLX_ALPHA_SIZE;
    attrib_list[9]  = GLX_DEPTH_SIZE;
    attrib_list[11] = GLX_STENCIL_SIZE;
    attrib_list[13] = GLX_ACCUM_RED_SIZE;
    attrib_list[15] = GLX_ACCUM_GREEN_SIZE;
    attrib_list[17] = GLX_ACCUM_BLUE_SIZE;
    attrib_list[19] = GLX_ACCUM_ALPHA_SIZE;

    if(mDoubleBuffered) {
      attrib_list[21] = GLX_DOUBLEBUFFER;
      attrib_list[22] = None;
    } else {
      attrib_list[21] = None;
    }

    attrib_list[2]  = mRGBA[0];
    attrib_list[4]  = mRGBA[1];
    attrib_list[6]  = mRGBA[2];
    attrib_list[8]  = mRGBA[3];
    attrib_list[10] = mDepth;
    attrib_list[12] = mStencil;
    attrib_list[14] = mAccum;
    attrib_list[16] = mAccum;
    attrib_list[18] = mAccum;
    attrib_list[20] = mAccum;

    return attrib_list;
  }
}

#endif

#endif
