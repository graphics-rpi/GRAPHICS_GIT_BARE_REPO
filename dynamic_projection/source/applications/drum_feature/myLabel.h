#ifndef _MYLABEL_H_
#define _MYLABEL_H_

#include <string>
#include <vector>
#include <math.h>
#include "../paint/text.h"
#include "../../multi_cursor/interaction.h"

class myLabel{
public:
	
	//Constructor myLabel(Pt POSITION, Vec3f COLOR, char* TEXT, 
	//										int HEIGHT, int WIDTH, bool FADES)
	myLabel(Pt pos, Vec3f c, std::string t, int h, int w, bool f){
		position = pos;
		color = c;
		text = t;
		height = h;
		width = w;
		fades = f;
	}
	
	//Getters
	Vec3f getColor(){return color;}
	std::string getText(){return text;}
	bool isFading(){return fades;}
	
	float closestCursorDist(double fdh){
		float vx, vy;
		float min_dist, dist;
		Pt curs;
		
		min_dist = 9999999; //infinity
	
		//for(unsigned int i=0; i<Interaction::cursorVec.size(); i++){
		for(unsigned int i=0; i<Interaction::getNumCursors(); i++){
		  curs = Interaction::getCursor(i)->getPosition(); //fdh);
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
				color = Vec3f(0.0,0.0,0.0);

			//TOO SMALL	(black)
			}else if(dist <= 75.0){
				color = Vec3f(1.0,1.0,1.0);
				
			//IN RANGE (greyscale)
			}else{
				dist -= 74.0;
				assert(dist >= 0);
				float factor = dist/125.0;
				//assert(factor <= 1.0);
				factor = (1.0 - factor);
				//std::cout << "factor " << factor << std::endl;
				
				color = Vec3f(factor,factor,factor);
				
			}
		}
	}

	void render(double fdh){
		fadeColor(fdh);
		drawstring(position.x, position.y, text.c_str(), color, width, height);
	}




protected:
	Pt position;
	Vec3f color;
	std::string text;
	int height;
	int width;
	bool fades;


};


#endif
