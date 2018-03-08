#include <iostream>
#include "text.h"
#include <cassert>
#include <algorithm>
#include "../../calibration/planar_interpolation_calibration/planar_calibration.h"

#include "../../../../remesher/src/utils.h"

// the natural height of the character set
#define CHARACTER_HEIGHT_above 119.05
#define CHARACTER_HEIGHT_below 33.33
#define CHARACTER_HEIGHT (CHARACTER_HEIGHT_above + CHARACTER_HEIGHT_below)


// =================================================
// =================================================
// COMPUTE THE WIDTH OF TEXT STRING
// =================================================

static double characterwidth(char c) {
/* TODO FIXME HACK SHOULD BE UNCOMMENTED??? 
  if (c != '_') {
    return glutStrokeWidth(GLUT_STROKE_ROMAN, c);
  } else {
    return glutStrokeWidth(GLUT_STROKE_ROMAN, ' ');
  }
*/
}

double drawstring_relative_width(const char *text) {

  // calculate the natural width of the characters
  const char * p;
  double WIDTH = 0;
  for (p = text; *p; p++) {
    WIDTH += characterwidth(*p);
    /*if (*p != '_') {
      WIDTH += glutStrokeWidth(GLUT_STROKE_ROMAN, *p);
    } else {
      WIDTH += glutStrokeWidth(GLUT_STROKE_ROMAN, ' ');
    }
    */
  }

  double relative_width = WIDTH / CHARACTER_HEIGHT;
  return relative_width;
}


// =================================================
// =================================================
// =================================================
static void  drawstring_leftjustify_scale(GLfloat x, GLfloat y, /*double WIDTH, */const char *text, const Vec3f &color, 
    double scale, double transparency, bool bold=false) { //, bool flipy) {

   // scale the parameters
  double above = CHARACTER_HEIGHT_above * scale;
  double below = CHARACTER_HEIGHT_below  * scale;

  glEnable (GL_LINE_SMOOTH );
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glPointSize(1);
  // HACK! fix me later scaling problem with nodes
  double linewidth = std::min(2.0,std::max(2.0,0.5*scale));
  int lw = linewidth;
  if (bold) { lw *= 1.5; }
  glLineWidth(lw); 

  //glColor3f(color.r(),color.g(),color.b());
  glColor4f(color.r(),color.g(),color.b(),transparency);
  glPushMatrix();
  
  glTranslatef(x, y-(above-below)/2.0, 0);
  glScalef(scale,scale,scale);
  
/* TODO FIXME HACK SHOULD BE UNCOMMENTED??? 
  const char * p;
  for (p = text; *p; p++) {
    if (*p != '_') {
      glutStrokeCharacter(GLUT_STROKE_ROMAN, *p);
    } else {
      glutStrokeCharacter(GLUT_STROKE_ROMAN, ' ');
    }
  }
*/

  glDisable (GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  glPopMatrix();
}

// =================================================
// =================================================
// =================================================
void  drawstring_centroid_scale(GLfloat x, GLfloat y, double WIDTH, const char *text, const Vec3f &color, double scale, double transparency) { //, bool flipy) {
  WIDTH *= scale;
  drawstring_leftjustify_scale(x-WIDTH/2.0,y,text,color,scale,transparency);
}


// =================================================
// =================================================
// =================================================
void  drawstring(GLfloat x, GLfloat y, const char *text, const Vec3f &color, double max_width, double max_height) {
  assert (max_width > 0 && max_height > 0);

  double relative_width = drawstring_relative_width(text);
  if (!text[0]) { 
    return; }
  if (relative_width < 0.001) return;
  double WIDTH = CHARACTER_HEIGHT*relative_width;

  double scale_w = -1;
  double scale_h = -1;

  assert (max_width > 0 && max_height > 0);

  if (max_width > 0) {
    scale_w = max_width/WIDTH;
  }

  // if the max height is specified
  if (max_height > 0) {
    scale_h = max_height/CHARACTER_HEIGHT; 
  }

  assert (scale_w > 0 && scale_h > 0);
  double scale = std::min(scale_w,scale_h);
  assert (scale > 0);

  drawstring_centroid_scale(x,y,WIDTH,text,color,scale,1.0); 
}

void  drawstringwithtransparency(GLfloat x, GLfloat y, const char *text, const Vec3f &color, double max_width, double max_height, double transparency){
  assert (max_width > 0 && max_height > 0);

  double relative_width = drawstring_relative_width(text);
  if (!text[0]) { 
    return; }
  if (relative_width < 0.001) return;
  double WIDTH = CHARACTER_HEIGHT*relative_width;

  double scale_w = -1;
  double scale_h = -1;

  assert (max_width > 0 && max_height > 0);

  if (max_width > 0) {
    scale_w = max_width/WIDTH;
  }

  // if the max height is specified
  if (max_height > 0) {
    scale_h = max_height/CHARACTER_HEIGHT; 
  }

  assert (scale_w > 0 && scale_h > 0);
  double scale = std::min(scale_w,scale_h);
  assert (scale > 0);

  drawstring_centroid_scale(x,y,WIDTH,text,color,scale,transparency); 
}


// =================================================
// =================================================
// =================================================
void  drawstrings(GLfloat x, GLfloat y, const std::vector<std::string> &texts, const std::vector<Vec3f> &colors, double max_width, double max_height) {
  assert (max_width > 0 && max_height > 0);

  assert (texts.size() == colors.size());
  assert (texts.size() > 0);

  std::vector<double> relative_widths;
  double sum_relative_width = 0;
  for (unsigned int i = 0; i < texts.size(); i++) {
    relative_widths.push_back(drawstring_relative_width(texts[i].c_str()));          
    sum_relative_width += relative_widths[i];
  }
    
  if (sum_relative_width < 0.001) return;
  double WIDTH = CHARACTER_HEIGHT*sum_relative_width;

  
  double scale_w = -1;
  double scale_h = -1;
  
  assert (max_width > 0 && max_height > 0);

  if (max_width > 0) {
    scale_w = max_width/WIDTH;
  }

  // if the max height is specified
  if (max_height > 0) {
    scale_h = max_height/CHARACTER_HEIGHT; 
  }

  assert (scale_w > 0 && scale_h > 0);
  double scale = std::min(scale_w,scale_h);
  assert (scale > 0);

  int tmp = -(WIDTH*scale)/2.0;
  for (unsigned int i = 0; i < texts.size(); i++) {
    drawstring_leftjustify_scale(x+tmp,y,texts[i].c_str(),colors[i],scale, 1.0); 
    tmp += CHARACTER_HEIGHT*relative_widths[i]*scale;
  }  
}

// =================================================

static std::string makestringforpath(const std::string &text, int &which_letter, double len, double scale) {
  std::string answer;
  double currlen = 0;
  while (1) {
    double tmp = currlen + characterwidth(text[which_letter])*scale;
    if (tmp > len) break;
    answer += text[which_letter];
    currlen = tmp; 
    which_letter = (which_letter+1)%text.size();    
  }
  return answer;
}

// =================================================
// =================================================
// =================================================
static void drawstringonpath_already_flipped(const std::deque<Pt> &path, const std::string& text, const Vec3f &color, double desired_height, double scalefactor, bool bold) {
  std::cout << "drawstringonpath b" << std::endl;
  assert (path.size() >= 2);
  int which_letter = 0;
  double scale = (desired_height*scalefactor)/CHARACTER_HEIGHT;
  //double relative_width = drawstring_relative_width(text.c_str());
  for (unsigned int i = 0; i < path.size()-1; i++) {

    std::cout << "drawstringonpath c " << text << std::endl;

    Pt dir = path[i+1]-path[i];
    double len = dir.Length();
    dir.Normalize();    
    std::string string_to_print = makestringforpath(text,which_letter,len,scale);
    double rad_angle = SignedAngleBetweenNormalized(Vec3f(dir.x,dir.y,0), Vec3f(1,0,0), Vec3f(0,0,1));
    double deg_angle = 180*rad_angle/M_PI;
    Pt p = path[i];
    glPushMatrix();
    glTranslatef(p.x,p.y,0);
    glRotated(-deg_angle,0,0,1);
    glTranslatef(-p.x,-p.y,0);
    drawstring_leftjustify_scale(p.x,p.y,string_to_print.c_str(),color,scale, bold, 1.0);
    glPopMatrix();
  }
}



void drawstringonpath(const std::deque<Pt> &path, const std::string& text, const Vec3f &color, double desired_height, double scalefactor, bool bold) {
  std::cout << "drawstringonpath a" << std::endl;
  assert (path.size() >= 2);
  Pt dir = path.back()-path.front();
  dir.Normalize();    
  double rad_angle = SignedAngleBetweenNormalized(Vec3f(dir.x,dir.y,0), Vec3f(1,0,0), Vec3f(0,0,1));
  double deg_angle = 180*rad_angle/M_PI;
  if (deg_angle > 90 || deg_angle < -90) {
    //std::cout << "FLIP!" << deg_angle << std::endl;
    int s = path.size();
    std::deque<Pt> path2(s);
    for (int i = 0; i < s; i++) { path2[i] = path[s-1-i]; }
    std::string text2 = text;
    for (unsigned int i = 0; i < text2.size(); i++) {
      if (text2[i] == '>') text2[i] = '<';
      //if (text2[i] == ' ') text2[i] = '*';
    }
    drawstringonpath_already_flipped(path2,text2,color,desired_height,scalefactor, bold);
  }
  else {
    //std::cout << "no FLIP!" << deg_angle << std::endl;
    drawstringonpath_already_flipped(path,text,color,desired_height,scalefactor, bold);
  }
}



int loadFontTexture(std::string filename) {
  // ugh staticity
  static bool statFirst = true;

  //char file[1024*1024*4];
  FILE* filep;
  GLuint testure_id = 12189;

  //(*ARGS->output) << "opening texture file:" << filename << "\r" << std::endl;
  //if(filep=fopen(filename.c_str(),"r")) {
  filep=fopen(filename.c_str(),"r");
  if(filep) {
      
    fclose(filep);
	
    Image<sRGB> image(filename);
    assert(GL_NO_ERROR==glGetError());
	
    //bool first=false;
    assert(GL_NO_ERROR==glGetError());
   
    glEnable(GL_TEXTURE_2D);    

    assert(GL_NO_ERROR==glGetError());    


    glGenTextures(1, &testure_id);


    glBindTexture(GL_TEXTURE_2D, testure_id);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);    
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
  
    //(*ARGS->output) << image.getCols() << " " << image.getRows() << std::endl;


    if (statFirst) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
		   image.getCols(), image.getRows(), 0,
		   GL_RGB, GL_UNSIGNED_BYTE,
		   (void*)image.getData());
    } else {
      glTexSubImage2D(GL_TEXTURE_2D, 0,
		      0, 0, image.getCols(), image.getRows(),
		      GL_RGB, GL_UNSIGNED_BYTE,
		      (void*)image.getData());
    }

    //(*ARGS->output) << "ERROR: " << glGetError() << std::endl;


    assert(GL_NO_ERROR==glGetError());

  }

  // ensure it actually loaded
  assert(testure_id != 12189);
  return testure_id;

}



// =================================================
// new stuff added by yauneg 15 July 2013
void drawCircularString(std::string message) {
  
  // load the text
  int testure_id = loadFontTexture("/ramdisk/testure.ppm");
    
  // stuff to initialize from the file
  float centerX = 0;
  float centerY = 0;
  float centerZ = 0;
  float radius = 0.4;
  // temp variables to read in the view
  float viewX = 0;
  float viewY = 0;
  float viewZ = 0;
  
  
  // this is where we read in the out.view file
  std::ifstream sepplesfile("/ramdisk/out.view");
  // this holds the views
  std::vector<float> views;
  // parse the out.view file
  while(!sepplesfile.eof()){
    std::string line;
    getline(sepplesfile, line);
    const char* textline = line.c_str();
    if (!strcmp(textline, "END OUT_VIEW_FILE\n")) {  
      break;
    } else {
      std::string line(textline);
      std::istringstream ss(line);
      std::string token;
      ss >> token;
      if (token == "table_center") {
	// read in center
	ss >> centerX;
	ss >> centerY;
	ss >> centerZ;
      } else if (token == "") {
	continue;
      } else if (token == "table_radius") {
	// read in table radius
	ss >> radius;
	//std::cout << radius << std::endl;
      } else if (token == "view") {
	//std::cout << "view" << std::endl;
	// read in the view's coords
	ss >> viewX;
	ss >> viewY;
	ss >> viewZ;
	views.push_back(viewX);
	views.push_back(viewY);
	views.push_back(viewZ);
      }
    }
  }
  

  // fight the z-fighting
  centerY += 0.0001;
  // fight the edge
  radius -= 0.15;

  glEnable(GL_TEXTURE_2D);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  // indices into the font texture atlas
  // 8 for each letter
  // ALSO probably only want to do this once instead of every time we draw text
  // MAYBE make text a class?
  TexCoord texcoords[26];
  for (int i = 0; i < 26; ++i) {
    //X1, Y1
    texcoords[i].x1   = ((float)((i/8+1)%16) + 1)/16.0;
    texcoords[i].y1 = ((float)((i/8+1)/16 + 5.0))/16.0;
  
    //X2, Y2
    texcoords[i].x2 = ((float)((i/8+1)%16))/16.0;
    texcoords[i].y2 = ((float)((i/8+1)/16 + 5.0))/16.0;

    //X3, Y3
    texcoords[i].x3 = ((float)((i/8+1)%16))/16.0;
    texcoords[i].y3 = ((float)((i/8+1)/16 + 4.0))/16.0;

    //X4, Y4
    texcoords[i].x4 = ((float)((i/8+1)%16) + 1)/16.0;
    texcoords[i].y4 = ((float)((i/8+1)/16 + 4.0))/16.0;
    
  }
     

  float letterWidth = 0.05;
  float letterHeight = 0.05;



  float theta = -M_PI;

  // spacing between messages
  float setSpace = M_PI/8;
  float slice = (message.length() * letterWidth / radius) + setSpace;
  float numFits = floor(2 * M_PI / slice);
  float remaining = 2 * M_PI - numFits * slice;
  float spacing = remaining / numFits + setSpace;

  // white text
  glColor3f(1.0, 1.0, 1.0);
  glBindTexture (GL_TEXTURE_2D, testure_id);

  while (theta > -3 * M_PI) {

    glBegin (GL_QUADS);
    for (unsigned int i = 0; i < message.length(); i++) {
      // horizontal
      //float currentX = startX - i * letterWidth;

      float currentX = centerX - radius*cos(theta);
      float currentZ = centerZ - radius*sin(theta);
    
      Vector3<float> p1(currentX, 0, currentZ);

      Vector3<float> r(currentX, 0, currentZ);
      Vector3<float> r2(centerX - radius*cos(theta - letterWidth/radius), 0, centerZ - radius*sin(theta - letterWidth/radius));
      Vector3<float> r_mid(centerX - radius*cos(theta - letterWidth/radius/2), 0, centerZ - radius*sin(theta - letterWidth/radius/2));
      r.normalize();
      r_mid.normalize();

      int texIndex = (message[i] - 97);
      
      // for ascii -> textcoord conversion
      int letter = (int) message[i];
      // lowest allowable letter is spacebar
      letter -= 32;
      // make sure it's in the ascii range spacebar to tilde
      if (letter < 0 || letter > 194) {
	std::cout << "that letter's not allowed!" << std::endl;
	continue;
      }
      

      Vector3<float> p2(r2);    
      //p2 = p2 - n * letterWidth;
      Vector3<float> p3(p2);
      r2.normalize();
      p3 = p3 + r_mid * letterHeight;
      Vector3<float> p4(p1);
      p4 = p4 + r_mid * letterHeight;

      glTexCoord2f ((letter % 16 + 1) / 16.0, (letter / 16 + 1) / 16.0);
      glVertex3f (p3.x(), 0.0001, p3.z());
      glTexCoord2f ((letter % 16) / 16.0, (letter / 16 + 1) / 16.0);
      glVertex3f (p4.x(), 0.0001, p4.z());
      glTexCoord2f ((letter % 16) / 16.0, (letter / 16) / 16.0);
      glVertex3f (p1.x(), 0.0001, p1.z());
      glTexCoord2f ((letter % 16 + 1) / 16.0, (letter / 16) / 16.0);
      glVertex3f (p2.x(), 0.0001, p2.z());


      theta -= letterWidth/radius;
    }
    glEnd ();

    // increment theta
    theta -= spacing;

  }

  glDisable(GL_TEXTURE_2D);
  glColor3f(0,0,0);
  


}
