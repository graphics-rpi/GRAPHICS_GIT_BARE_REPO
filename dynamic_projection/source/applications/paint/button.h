#ifndef __BUTTON_H__
#define __BUTTON_H__

#include <glm/glm.hpp>

#include <string>
#include <sys/time.h>
#include <list>

#include "ClickableObject.h"
#include "../../calibration/planar_interpolation_calibration/tracker.h"
#include "../../../../remesher/src/vectors.h"

// because the QTElement/QTEdge/QTNode makes a multiple inheritance diamond, this has to be virtual 
//class Button : public ClickableObject {
class Button : virtual public ClickableObject {

public:
  // ==========================
  // CONSTRUCTORS & DESTRUCTORS
  Button(const Pt &p, double w, double h, const Vec3f &c, const std::string &t="");
  Button(const Pt &p, double w, double h, const Vec3f &c,
         const std::string &small_texture_filename, const std::string &large_texture_filename,
         double u0=0, double v0=0, double u1=1, double v1=1);

  void specify_texture(const std::string &small_texture_filename, const std::string &large_texture_filename,
         double u0=0, double v0=0, double u1=1, double v1=1);

  virtual ~Button();

  // ==========================
  // DIMENSIONS & POSITION
  bool PointInside(const Pt &p) const;
  virtual double DistanceFrom(const Pt &p) const;

  glm::vec3 getUpperLeftText(){ 
      Pt p = getMagnifiedLowerLeftCorner(); 
      return glm::vec3(p.x, p.y + getHeight(), 0.0f); 
  }
  glm::vec3 getLowerRightText(){ 
      Pt p = getMagnifiedLowerLeftCorner(); 
      return glm::vec3(p.x + getWidth(), p.y, 0.0f); 
  }

  // ==========================
  // TEXT
  int numTextStrings() const { return text_strings.size(); }
  const std::string& getText(int i) const {
    assert (i >= 0 && i < numTextStrings());
    return text_strings[i]; }
  void clearText() { text_strings.clear(); }
  void addText(const std::string& t);
  
  std::vector<std::string>& getTextStrings() { return text_strings; }
  void SetRenderText(bool val) { render_text = val; }
  bool GetRenderText() const { return render_text; }

  glm::vec3 getTextColor();

  // ==========================
  // TEXTURE
  
  void specify_small_texture(const std::string &small_filename);
  void specify_large_texture(const std::string &large_filename);
  void specify_uv_coordinates(double u0, double v0, double u1, double v1);

  
  void enable_texture() { texture_enabled = true; }
  void disable_texture() { texture_enabled = false; }
  bool is_texture_enabled() const { return texture_enabled; }

  // ==========================
  // RENDERING
  virtual void paint(const Vec3f &background_color=Vec3f(0,0,0)) const;

  void setCircle(bool val) { is_circle = val; }
  bool isCircle() const { return is_circle; }

  void setBoldText(bool val) { bold_text = val; }
  bool isBoldText() const { return bold_text; }

  Image<sRGB>* getImageSmall() const { assert (SmallImageSpecifiedAndLoaded()); return sm_image; }
  Image<sRGB>* getImageLarge() const { assert (LargeImageSpecifiedAndLoaded()); return lg_image; }

  virtual bool isVisible() const;
  virtual double  getZOrder() const;

  // GLM functions for moving data around
  void populateCircleBorder(std::vector<glm::vec3> & positions,
          std::vector<glm::vec4> & colors,
          std::vector<GLfloat> & radii);
  void populateCircleButton(std::vector<glm::vec3> & positions,
          std::vector<glm::vec4> & colors,
          std::vector<GLfloat> & radii);
  void populateCircleTexturedButton(std::vector<glm::vec3> & positions,
          std::vector<glm::vec2> & uvs,
          std::vector<GLfloat> & radii);

  void populateRectangleBorder(std::vector<glm::vec3> & positions, std::vector<glm::vec4> & colors);
  void populateRectangleTexturedButton(std::vector<glm::vec3> & positions, std::vector<glm::vec2> & uvs);
  void populateRectangleButton(std::vector<glm::vec3> & positions, std::vector<glm::vec4> & colors);

  // These two functions load the image if not already loaded
  // use un-consting hack to load
  bool SmallImageSpecifiedAndLoaded() const;
  bool LargeImageSpecifiedAndLoaded() const;

  int getSmallTexName() const { assert (SmallImageSpecifiedAndLoaded()); return sm_texName; }
  int getLargeTexName() const { assert (LargeImageSpecifiedAndLoaded()); return lg_texName; }

 protected:
  // helper functions
  void initialize();
  Image<sRGB>* loadImage(const std::string &f,GLuint *tn);

  // ==========================
  // REPRESENTATION

  // appearance
  bool render_text;
  std::vector<std::string> text_strings;
  bool is_circle;
  bool bold_text;

  // texture
  bool texture_enabled;

  double u0,v0,u1,v1;


private:

  void load_small_texture();
  void load_large_texture();

  std::string sm_image_filename;
  std::string lg_image_filename;
  GLuint sm_texName;
  GLuint lg_texName;
  Image<sRGB>* sm_image;
  Image<sRGB>* lg_image;

  int m_order;
  bool m_visible;

};

void draw_circle(const Pt &pt, double radius, int num);
void draw_circle_with_texture(const Pt &pt, double radius, int num);

#endif
