#ifndef _TEXT_H_
#define _TEXT_H_

// Included files for OpenGL Rendering
#include "../paint/gl_includes.h"


#include <deque>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>

#include "../../../../remesher/src/vectors.h"

//void  drawstring(GLfloat x, GLfloat y, const char *text, double r, double g, double b, double max_width, double max_height, double linewidth_extra = 0.0, bool flipy = false);

class TexCoord
{
  public:
    float x1,x2,y1,y2,x3,y3,x4,y4;
};

class Pt;

double drawstring_relative_width(const char *text);



void  drawstring_centroid_scale(GLfloat x, GLfloat y, double WIDTH, const char *text, const Vec3f &color, double scale, double transparency); //, bool flipy);



void  drawstring(GLfloat x, GLfloat y, const char *text, const Vec3f &color, double max_width, double max_height);

void  drawstringwithtransparency(GLfloat x, GLfloat y, const char *text, const Vec3f &color, double max_width, double max_height, double transparency);

void  drawstringonpath(const std::deque<Pt> &path, const std::string &text, const Vec3f &color, double max_height, double scale,bool bold);


void  drawstrings(GLfloat x, GLfloat y, const std::vector<std::string> &texts, const std::vector<Vec3f> &colors, double max_width, double max_height);

// the new textured text functions
int loadFontTexture(std::string filename);
void drawCircularString(std::string message);
void drawViewString(std::string message, int texture_idx);


//, double linewidth_extra); //, bool flipy=false);
//, double linewidth_extra = 0.0); 
//, bool flipy = false);



#endif
