#include "PuzzlePiece.h"
#include "puzzle_argparser.h"
#define BUTTON_BORDER 10
#define DAMP_MOVEMENT 0.3

#include "score.h"
using namespace std;

PuzzlePiece::PuzzlePiece(const Pt &p, double w, double h, const std::string &f,
			 double u0_, double v0_, double u1_, double v1_) :
  ClickableObject (p,w,h,Vec3f(1,1,1) ),
  Button(p, w, h, Vec3f(1,1,1), "", f, u0_, v0_, u1_, v1_) 
{

  Init();

  assert (border_info.size() == 0);

}

void PuzzlePiece::Init(){

  for(int i = 0; i < 9; ++i){
    borders[i] = false;
  }
  
  borders[0] = true;
  Pt pt = getLowerLeftCorner();
  
  x0 = pt.x;
  x1 = pt.x + getWidth();
  y0 = pt.y;
  y1 = pt.y + getHeight();
  
  x0p =  pt.x + BUTTON_BORDER;
  y0p =  pt.y - BUTTON_BORDER;
  x1p =  pt.x + getWidth() - BUTTON_BORDER;
  y1p =  pt.y + getHeight() + BUTTON_BORDER;
  
  double x_percent = (BUTTON_BORDER)/(x1-x0);
  double y_percent = (BUTTON_BORDER)/fabs((y0-y1));
  
  u0p = u0 + (u1-u0) * x_percent;
  u1p = u1 - (u1-u0) * x_percent;
  v0p = v0 - (v0-v1) * y_percent;
  v1p = v1 + (v0-v1) * y_percent;
  
  
  correct = false;
  correct_pt = Pt(0,0);
}








void PuzzlePiece::paint(const Vec3f &) const{

  if (border_info.size() != 0 &&
      border_info.size() != 1) return;
  
  //  glDisable(GL_BLEND);
  
  Pt pt = getLowerLeftCorner();




  //  assert (0) ;
    // NEED TO FIX THIS! CONST PROBLEM BELOW

  //  #if 0

  PuzzlePiece *this_hack = (PuzzlePiece*)this;

  this_hack->x0 = pt.x;
  this_hack->x1 = pt.x + getWidth();
  this_hack->y1 = pt.y;
  this_hack->y0 = pt.y + getHeight();
  
  this_hack->x0p =  x0 + BUTTON_BORDER;
  this_hack->y0p =  y0 - BUTTON_BORDER;
  this_hack->x1p =  x1 - BUTTON_BORDER;
  this_hack->y1p =  y1 + BUTTON_BORDER;
  //#endif



  //if (isPressed()) {
  if (border_info.size() == 0) {//= 1) {
  //  std::cout << "border_info_size is bad " << border_info.size() << std::endl;	    
  } else {
    //std::cout << "border info size " << border_info.size() << std::endl;
    assert (border_info.size() == 1);
    Vec3f border_color = border_info[0].color;

    std::cout << "border color red? " << border_color << std::endl;
    
    glColor3f(border_color.r(),border_color.g(),border_color.b());
    glBegin(GL_QUADS);
    glVertex2f(pt.x-BUTTON_BORDER           ,pt.y-BUTTON_BORDER            );
    glVertex2f(pt.x+getWidth()+BUTTON_BORDER,pt.y-BUTTON_BORDER            );
    glVertex2f(pt.x+getWidth()+BUTTON_BORDER,pt.y+getHeight()+BUTTON_BORDER);
    glVertex2f(pt.x-BUTTON_BORDER           ,pt.y+getHeight()+BUTTON_BORDER);
    glEnd();
  }
  
  glColor3f(color.r(),color.g(),color.b());

  assert (LargeImageSpecifiedAndLoaded());
  
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, getLargeTexName());
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glBegin(GL_QUADS);
  
  //Center tile
  Fade(center);
  glTexCoord2f(u0p, v0p);     glVertex2f(x0p, y0p);
  glTexCoord2f(u0p, v1p);     glVertex2f(x0p, y1p);
  glTexCoord2f(u1p, v1p);     glVertex2f(x1p, y1p);
  glTexCoord2f(u1p, v0p);     glVertex2f(x1p, y0p);
  
  //Top left corner
  Fade(topleft);
  glTexCoord2f(u0, v0);       glVertex2f(x0, y0);
  glTexCoord2f(u0p, v0);      glVertex2f(x0p, y0);
  glTexCoord2f(u0p, v0p);     glVertex2f(x0p, y0p);
  glTexCoord2f(u0, v0p);      glVertex2f(x0, y0p);
  
  //Left edge
  Fade(left);
  glTexCoord2f(u0, v0p);      glVertex2f(x0, y0p);
  glTexCoord2f(u0, v1p);      glVertex2f(x0, y1p);
  glTexCoord2f(u0p, v1p);     glVertex2f(x0p, y1p);
  glTexCoord2f(u0p, v0p);     glVertex2f(x0p, y0p);
  
  //Left bottom corner
  Fade(botleft);
  glTexCoord2f(u0, v1p);      glVertex2f(x0, y1p);
  glTexCoord2f(u0, v1);       glVertex2f(x0, y1);
  glTexCoord2f(u0p, v1);      glVertex2f(x0p, y1);
  glTexCoord2f(u0p, v1p);     glVertex2f(x0p, y1p);
  
  //Bottom edge
  Fade(bot);
  glTexCoord2f(u0p, v1p);     glVertex2f(x0p, y1p);
  glTexCoord2f(u0p, v1);     glVertex2f(x0p, y1);
  glTexCoord2f(u1p, v1);     glVertex2f(x1p, y1);
  glTexCoord2f(u1p, v1p);     glVertex2f(x1p, y1p);
  
  //Bottom right corner
  Fade(botright);
  glTexCoord2f(u1p, v1p);     glVertex2f(x1p, y1p);
  glTexCoord2f(u1p, v1);      glVertex2f(x1p, y1);
  glTexCoord2f(u1, v1);       glVertex2f(x1, y1);
  glTexCoord2f(u1, v1p);      glVertex2f(x1, y1p);
  
  //Right edge
  Fade(right);
  glTexCoord2f(u1p, v0p);     glVertex2f(x1p, y0p);
  glTexCoord2f(u1p, v1p);     glVertex2f(x1p, y1p);
  glTexCoord2f(u1, v1p);      glVertex2f(x1, y1p);
  glTexCoord2f(u1, v0p);      glVertex2f(x1, y0p);
  
  //Top edge
  Fade(top);
  glTexCoord2f(u0p, v0);      glVertex2f(x0p, y0);
  glTexCoord2f(u0p, v0p);     glVertex2f(x0p, y0p);
  glTexCoord2f(u1p, v0p);     glVertex2f(x1p, y0p);
  glTexCoord2f(u1p, v0);      glVertex2f(x1p, y0);
  
  //Top right corner
  Fade(topright);
  glTexCoord2f(u1p, v0);      glVertex2f(x1p, y0);
  glTexCoord2f(u1p, v0p);     glVertex2f(x1p, y0p);
  glTexCoord2f(u1, v0p);      glVertex2f(x1, y0p);
  glTexCoord2f(u1, v0);       glVertex2f(x1, y0);
  
  glEnd();
  
  //    HandleGLError("BUTTON PAINT C6");
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  
  //  HandleGLError("AFTER BUTTON PAINT");
  
}


bool PuzzlePiece::GetBorderVal(int index){

	return borders[index];

}

void PuzzlePiece::SetBorderVal(int index, bool newval){

	borders[index] = newval;

}

void PuzzlePiece::ResetBorder(){

	for(int i = 1; i < 9; ++i)
		borders[i] = false;

}

void PuzzlePiece::FullBorder(){

	for(int i = 1; i < 9; ++i)
		borders[i] = true;

}

double PuzzlePiece::CentroidDist(const PuzzlePiece &a){

	  Pt p = Button::getCentroid();
	  Pt pt = a.getCentroid();

	  double x_dist,y_dist;
	  if (p.x < pt.x) {
	    x_dist = pt.x-p.x;
	  } else if (p.x > pt.x+getWidth()) {
	    x_dist = p.x - pt.x+getWidth();
	  } else {
	    x_dist = 0;
	  }
	  if (p.y < pt.y) {
	    y_dist = pt.y-p.y;
	  } else if (p.y > pt.y+getHeight()) {
	    y_dist = p.y - pt.y+getHeight();
	  } else {
	    y_dist = 0;
	  }
	  double answer = sqrt(x_dist*x_dist+y_dist*y_dist);
	  return answer;

}


void PuzzlePiece::Fade(int index) const {

	if(borders[index])
	{
		glColor4f(1,1,1,1);
	}
	else
	{
		glColor4f(.4, .4, .4, .4);
	}

}


void PuzzlePiece::OnPress() {
  //std::cout << "press" << std::endl;
  //std::cout << "CP" << getCorrectPoint() << std::endl;
  //std::cout << "LL" << getLowerLeftCorner() << std::endl;
  atclick = getLowerLeftCorner();
}

Pt SnappedToGrid(const Pt &current);

void PuzzlePiece::OnRelease(Score &score) {
  //std::cout << "release" << std::endl;


  Pt tmp = getLowerLeftCorner();
  Pt snapped = SnappedToGrid(tmp);
  Pt correct = getCorrectPoint();

  //  std::cout << "\natclick " << atclick << std::endl;
  //std::cout << "tmp     " << tmp << std::endl;
  //std::cout << "snapped " << snapped << std::endl;
  //std::cout << "correct " << correct << std::endl;  


  // distances in pixels!
  double orig_dist = DistanceBetweenTwoPoints(atclick,correct);
  double now_dist = DistanceBetweenTwoPoints(snapped,correct);

  //std::cout << "ORIG : " << orig_dist << "   NOW : " << now_dist << std::endl;

  if (now_dist < orig_dist-1) {
    // moved closer to correct spot
    score.incrGood();
  } else if (orig_dist < 1 && now_dist > 1) {
    // took out of correct spot
    score.incrBad();
  } else if (now_dist > orig_dist*2 - 1) {
    // took out of correct spot
    score.incrBad();
  } else {
    score.incrNeutral();
  }
}

