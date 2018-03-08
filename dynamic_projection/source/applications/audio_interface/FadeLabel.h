#ifndef _FADELABEL_H_
#define _FADELABEL_H_

#include <string>
#include <vector>
#include <math.h>
#include "../paint/text.h"
#include "../../multi_cursor/interaction.h"

class FadeLabel{
public:
	
	//Constructor FadeLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
	//										int HEIGHT, int WIDTH, bool FADES)
	FadeLabel(Pt pos, Vec3f c, std::string t, int h, int w, bool f){
		position = pos;
		color = c;
		text = t;
		height = h;
		width = w;
		fades = f;
		transparency = 1.0;
	}
	
	//Getters
	Vec3f getColor(){return color;}
	std::string getText(){return text;}
	void setText(std::string &str){ text = str; }
	bool isFading(){return fades;}
	void Move(Pt pos){position = pos;}
	
	float closestCursorDist(double fdh){
		float vx, vy;
		float min_dist, dist;
		Pt curs;
		
		min_dist = 9999999; //infinity
	
		for(unsigned int i=0; i<Interaction::getNumCursors(); i++){
		  curs = Interaction::getCursor(i)->getScreenPosition(); //fdh);
			vx = curs.x - position.x;
			vy = curs.y - position.y;
			dist = sqrt(vx*vx + vy*vy);
			if(dist < min_dist){
				min_dist = dist;
			}
		}
		
		return min_dist;
	}
	
	void fadeColor(double fdh){
		if(fades){
			float dist = closestCursorDist(fdh);
			
			//TOO BIG (white)
			if(dist > 200.0){
				//color = Vec3f(0.0,0.0,0.0);
				transparency = 0.0;

			//TOO SMALL	(black)
			}else if(dist <= 75.0){
				//color = Vec3f(1.0,1.0,1.0);
				transparency = 1.0;
				
			//IN RANGE (greyscale)
			}else{
				dist -= 74.0;
				assert(dist >= 0);
				float factor = dist/125.0;
				//assert(factor <= 1.0);
				factor = (1.0 - factor);
				//std::cout << "factor " << factor << std::endl;
				
				//color = Vec3f(factor,factor,factor);
				transparency = factor;
				
			}
		}
	}

	void render(double fdh){
		fadeColor(fdh);
		Pt bottomLeft;
		bottomLeft.x = position.x - (width/2) - (width/20.0);
		bottomLeft.y = position.y - (height/2);

		glEnable(GL_BLEND);
  		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glBegin(GL_TRIANGLES);
			glColor4f(0.0,0.0,0.0,transparency);
			glVertex2f(bottomLeft.x, bottomLeft.y);
			glVertex2f(bottomLeft.x, (bottomLeft.y + height));
			glVertex2f((bottomLeft.x + width + width/40), bottomLeft.y);
		glEnd();
		glBegin(GL_TRIANGLES);
			glColor4f(0.0,0.0,0.0,transparency);
			glVertex2f((bottomLeft.x + width + width/40), (bottomLeft.y + height));
			glVertex2f(bottomLeft.x, (bottomLeft.y + height));
			glVertex2f((bottomLeft.x + width + width/40), bottomLeft.y);
		glEnd();
		glDisable(GL_BLEND);


		drawstringwithtransparency(position.x, position.y, text.c_str(), color, width, height, transparency);
	}

	void renderNoBackground(double fdh){
		fadeColor(fdh);
		drawstringwithtransparency(position.x, position.y, text.c_str(), color, width, height, transparency);
	}




protected:
	Pt position;
	Vec3f color;
	double transparency;
	std::string text;
	int height;
	int width;
	bool fades;


};


#endif
