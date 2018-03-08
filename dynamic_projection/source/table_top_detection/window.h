#ifndef _WINDOW_H_
#define _WINDOW_H_

// ======================================================================
// WINDOW
// ======================================================================

//class Object;

class Window {
public:
  Window (Rectangle &rect, std::string window_keyword){
    rectangle = rect;
    keyword = window_keyword;
  }
  enum WindowMarker {CYAN, MAGENTA, YELLOW};
  void draw(Image<sRGB> &image){
    rectangle.draw(image);
  }
  void project(CalibratedCamera &camera, double height){
    rectangle.project(camera, height);
  }
                                            
  void write(FILE *fp){
    fprintf(fp, "window ");
    rectangle.write(fp);
    fprintf(fp, " %s\n", keyword.c_str());
  }
private:
  WindowMarker marker_color;
  Rectangle rectangle;
  std::string keyword;
};

#endif
