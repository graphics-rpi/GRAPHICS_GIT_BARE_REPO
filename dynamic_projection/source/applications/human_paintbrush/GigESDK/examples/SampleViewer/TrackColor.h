//#include "FinderFrame.h"
//#include "ChildFrame.h"
//#include <stack>
//#include "stdafx.h"
//#include "SampleViewer.h"
//#include "FinderFrame.h"
//#include "HistFrame.h"
//#include "ImageView.h"
//
//
//#include <ImageLib.h>
//#include <math.h>
//#include <iostream>
//#include <fstream>

//#include "ListFrame.h"
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;

class trackColor{
public:
	trackColor(tPvFrame * f){
		t_width = (unsigned long)f->Width;
		t_height = (unsigned long)f->Height;
		t_frame = f;
		t_bluepixels = 0;

		std::ofstream t_debug("C:/users/grfx/Desktop/debug_hp.txt");
		t_debug << "debug stuff:" << std::endl;
		t_debug.close();
	}
	void sendStr(std::string s){
		t_debug.open("C:/users/grfx/Desktop/debug_hp.txt", std::fstream::app);
		t_debug << s << " ";
		t_debug.close();
	}
	void sendInt(int s){
		t_debug.open("C:/users/grfx/Desktop/debug_hp.txt", std::fstream::app);
		t_debug << s << " ";// << std::endl;
		t_debug.close();
	}
	void sendDouble(double s){
		t_debug.open("C:/users/grfx/Desktop/debug_hp.txt", std::fstream::app);
		t_debug << s << " ";
		t_debug.close();
	}
	void sendChar(char s){
		t_debug.open("C:/users/grfx/Desktop/debug_hp.txt", std::fstream::app);
		t_debug << s << " ";
		t_debug.close();
	}
	void sendNL(){
		t_debug.open("C:/users/grfx/Desktop/debug_hp.txt", std::fstream::app);
		t_debug << std::endl;
		t_debug.close();
	}
	void sendOut(int posx, int posy, int linesize){
		std::ofstream t_output("../../../paint/PositionToMax.txt");
		t_output << ((double)posx*(128.0/(double)t_width)) << endl << (128.0-((double)posy*(128.0/(double)t_height))) << endl << linesize << endl;
		t_output.close();
		std::ofstream t_output2("../../../paint/PositionToPaint.txt");
		t_output2 << ((double)posx*(600.0/(double)t_width)) << endl << (600.0-((double)posy*(600.0/(double)t_height))) << endl << linesize << endl;
		t_output2.close();
	}
	void analyzeFrame(){
		sendInt(t_frame->Height);
		sendStr(" ");
		sendInt(t_frame->Width);
		sendNL();sendNL();

		unsigned long buffer_size = t_frame->Height * t_frame->Width;
		unsigned int* bufferRed = new unsigned int[buffer_size];
		unsigned int* bufferGreen = new unsigned int[buffer_size];
		unsigned int* bufferBlue = new unsigned int[buffer_size];

		PvUtilityColorInterpolate(t_frame, bufferRed, bufferGreen,
									bufferBlue, 0, 0);	
		
		int red = 0, blue = 0, green = 0;
		
		for(int i=0; i<buffer_size; i++){
			if(bufferBlue[i] > bufferGreen[i] && bufferBlue[i] > bufferRed[i]){
				blue++;
			}else if(bufferGreen[i] > bufferRed[i]){
				green++;
			}else{
				red++;
			}
		}
		sendStr("R=");
		sendDouble((double)red/(double)buffer_size * 100.0);
		sendStr("G=");
		sendDouble((double)green/(double)buffer_size * 100.0);
		sendStr("B=");
		sendDouble((double)blue/(double)buffer_size * 100.0);
		sendNL();
		sendNL();

		for(int i=0; i<t_frame->Height; i += 40){
			for(int j=0; j<t_frame->Width; j += 40){
					if(    bufferBlue[i*t_frame->Height + j] > bufferGreen[i*t_frame->Height + j] 
						&& bufferBlue[i*t_frame->Height + j] > bufferRed[i*t_frame->Height + j]){
						sendStr("B");
					}else{
						sendStr("-");
					}
			}
			sendNL();
		}
	

		delete [] bufferRed;
		delete [] bufferGreen;
		delete [] bufferBlue;
	}
	void normalize(BYTE* m_Buffer){
		double length;
		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){

				length = sqrt(((double)m_Buffer[indBuf(i,j,2)]*(double)m_Buffer[indBuf(i,j,2)])
							+ ((double)m_Buffer[indBuf(i,j,1)]*(double)m_Buffer[indBuf(i,j,1)]));
				length = sqrt((double)m_Buffer[indBuf(i,j,0)]*(double)m_Buffer[indBuf(i,j,0)] + length*length);
				
				m_Buffer[indBuf(i,j,2)] = (int)((m_Buffer[indBuf(i,j,2)] / length) * 255);
				m_Buffer[indBuf(i,j,1)] = (int)((m_Buffer[indBuf(i,j,1)] / length) * 255);
				m_Buffer[indBuf(i,j,0)]   = (int)((m_Buffer[indBuf(i,j,0)] / length) * 255);
			}
		}
	}

	int indBuf(int i, int j, int bgr){
		return (i*t_width*3 + (j*3) + bgr);
	}
	double getGrayScale(double r, double g, double b){
		//return ((0.2989 * r) + ( 0.5870 * g) + (0.1140 * b));
		//return ((0.2989 * r) + ( 0.5870 * g) + (0.1140 * b));
		return ((0.1 * r) + ( 0.3 * g) + (0.2 * b));
	}


	void extractBlue(BYTE* m_Buffer){
		t_thresh.push_back(60); //lowest thresh - blue
		t_thresh.push_back(70); //magenta
		t_thresh.push_back(80); //green
		t_thresh.push_back(90); //yellow
		t_thresh.push_back(100); //highest thresh - red
		int i_crop = 90*0;
		int j_crop = 170*0;
		t_avgi = t_avgj = 0;
		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){
				//border exclusion
				if(i < i_crop || i > (t_height-i_crop) || j < j_crop || j > (t_width - j_crop) ){
					m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 255;
				//highest threshold
				}else if(   (m_Buffer[indBuf(i,j,0)] - t_thresh[4]) > m_Buffer[indBuf(i,j,1)] &&
				      (m_Buffer[indBuf(i,j,0)] - t_thresh[4]) > m_Buffer[indBuf(i,j,2)]){
					//m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 255;
					t_bluepixels++;
					t_avgi += i;
					t_avgj += j;
				}else if(   (m_Buffer[indBuf(i,j,0)] - t_thresh[3]) > m_Buffer[indBuf(i,j,1)] &&
							(m_Buffer[indBuf(i,j,0)] - t_thresh[3]) > m_Buffer[indBuf(i,j,2)]){
					//m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,1)] = 255;
					m_Buffer[indBuf(i,j,2)] = 255;
					t_bluepixels++;
					t_avgi += i;
					t_avgj += j;
				}else if(   (m_Buffer[indBuf(i,j,0)] - t_thresh[2]) > m_Buffer[indBuf(i,j,1)] &&
							(m_Buffer[indBuf(i,j,0)] - t_thresh[2]) > m_Buffer[indBuf(i,j,2)]){
					//m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,1)] = 255;
					m_Buffer[indBuf(i,j,2)] = 0;
					t_bluepixels++;
					t_avgi += i;
					t_avgj += j;
				}else if(   (m_Buffer[indBuf(i,j,0)] - t_thresh[1]) > m_Buffer[indBuf(i,j,1)] &&
							(m_Buffer[indBuf(i,j,0)] - t_thresh[1]) > m_Buffer[indBuf(i,j,2)]){
					//m_Buffer[indBuf(i,j,1)] = 255;
					m_Buffer[indBuf(i,j,0)] = 255;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 255;
					t_bluepixels++;
					t_avgi += i;
					t_avgj += j;
				//lowest threshold
				}else if(   (m_Buffer[indBuf(i,j,0)] - t_thresh[0]) > m_Buffer[indBuf(i,j,1)] &&
							(m_Buffer[indBuf(i,j,0)] - t_thresh[0]) > m_Buffer[indBuf(i,j,2)]){
					m_Buffer[indBuf(i,j,0)] = 255;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 0;
					//m_Buffer[indBuf(i,j,0)] = 0;
					t_bluepixels++;
					t_avgi += i;
					t_avgj += j;
				//doesn't pass any threshold
				//convert to grayscale image
				}else{
					//double gray = getGrayScale((double)m_Buffer[indBuf(i,j,2)], (double)m_Buffer[indBuf(i,j,1)], (double)m_Buffer[indBuf(i,j,0)]);
					//m_Buffer[indBuf(i,j,0)] = (int)gray;
					//m_Buffer[indBuf(i,j,1)] = (int)gray;
					//m_Buffer[indBuf(i,j,2)] = (int)gray;
					m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 0;
					/*t_bluepixels++;
					t_avgi += i;
					t_avgj += j;*/
				}
				//m_Buffer[indBuf(i,j,1)] = 0;
				//m_Buffer[indBuf(i,j,2)] = 0;
			}
		}


		t_avgi /= t_bluepixels;
		t_avgj /= t_bluepixels;

		/*sendNL();
		sendStr("blue pixels:");
		sendInt(t_bluepixels);
		sendNL();*/

		int pix_low = 2500;
		int pix_high = 20000;

		t_bluepixels = (t_bluepixels - pix_low);
		if(t_bluepixels <= 0) return; 
		t_bluepixels /= (pix_high - pix_low)/70;

		sendOut(t_avgj, t_avgi, t_bluepixels);
	}

	//EXTENSIONS==============================================================================
	//========================================================================================
	void rgNormalize(BYTE* m_Buffer){

		//will equal (r + g + b) for each pixel
		double intensity = 0.0;
		double r, g, b;
		int rInt, gInt, bInt;

		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){
				intensity = m_Buffer[indBuf(i,j,0)] + m_Buffer[indBuf(i,j,1)] + m_Buffer[indBuf(i,j,2)];
				if(intensity > 0.0){
					r = (double)m_Buffer[indBuf(i,j,2)] / intensity;
					g = (double)m_Buffer[indBuf(i,j,1)] / intensity;
					b = (double)m_Buffer[indBuf(i,j,0)] / intensity;
				}

				rInt = r*255;
				gInt = g*255;
				bInt = b*255;

				m_Buffer[indBuf(i,j,2)] = rInt;
				m_Buffer[indBuf(i,j,1)] = gInt;
				m_Buffer[indBuf(i,j,0)] = bInt;
			}
		}
	}

	void greyWorld(BYTE* m_Buffer){

		double rAvg, gAvg, bAvg;
		rAvg = gAvg = bAvg = 0;
		int rInt, gInt, bInt;
		double rScale, bScale;
		int r, g, b;

		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){
				rAvg += m_Buffer[indBuf(i,j,2)];
				gAvg += m_Buffer[indBuf(i,j,1)];
				bAvg += m_Buffer[indBuf(i,j,0)];
			}
		}
		double numPixels = t_height * t_width;
		assert(numPixels > 0);
		rAvg /= numPixels;
		gAvg /= numPixels;
		bAvg /= numPixels;

		rScale = gAvg / rAvg;
		bScale = gAvg / bAvg;

		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){

				rInt = (m_Buffer[indBuf(i,j,2)] * rScale);
				//gInt = ((m_Buffer[indBuf(i,j,1)]/2) / gAvg)*255;
				bInt = (m_Buffer[indBuf(i,j,0)] * bScale);

				if(rInt > 255) rInt = 255;
				if(gInt > 255) gInt = 255;
				if(bInt > 255) bInt = 255;

				m_Buffer[indBuf(i,j,2)] = rInt;
				//m_Buffer[indBuf(i,j,1)] = gInt;
				m_Buffer[indBuf(i,j,0)] = bInt;
			}
		}
	}
	void rgbMax(BYTE* m_Buffer){

		double rMax, gMax, bMax;
		rMax = gMax = bMax = 0;
		int rInt, gInt, bInt;
		int r, g, b;
		double rScale, bScale;

		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){
				if(m_Buffer[indBuf(i,j,2)] > rMax && m_Buffer[indBuf(i,j,2)] < 240) rMax = m_Buffer[indBuf(i,j,2)];
				if(m_Buffer[indBuf(i,j,1)] > gMax && m_Buffer[indBuf(i,j,1)] < 240) gMax = m_Buffer[indBuf(i,j,1)];
				if(m_Buffer[indBuf(i,j,0)] > bMax && m_Buffer[indBuf(i,j,0)] < 240) bMax = m_Buffer[indBuf(i,j,0)];
			}
		}

		rScale = gMax / rMax;
		bScale = gMax / bMax;
		//double totalMax = (rMax/255) * (gMax/255) * (bMax/255);
		sendDouble(rScale);
		sendDouble(bScale);
		sendNL();
		sendDouble(rMax);
		sendDouble(gMax);
		sendDouble(bMax);
		sendNL();

		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){

				rInt = (m_Buffer[indBuf(i,j,2)] * rScale);
				//gInt = ((m_Buffer[indBuf(i,j,1)]/2) / gAvg)*255;
				bInt = (m_Buffer[indBuf(i,j,0)] * bScale);

				if(rInt > 255) rInt = 255;
				//if(gInt > 255) gInt = 255;
				if(bInt > 255) bInt = 255;

				m_Buffer[indBuf(i,j,2)] = rInt;
				//m_Buffer[indBuf(i,j,1)] = gInt;
				m_Buffer[indBuf(i,j,0)] = bInt;
			}
		}
	}

	void findBlue(BYTE* m_Buffer){
		t_thresh.push_back(90);
		for(int i=0; i<t_height; i+=1){
			for(int j=0; j<t_width; j+=1){

				if((m_Buffer[indBuf(i,j,0)] - t_thresh[0]) > m_Buffer[indBuf(i,j,1)] &&
				   (m_Buffer[indBuf(i,j,0)] - t_thresh[0]) > m_Buffer[indBuf(i,j,2)]){

					m_Buffer[indBuf(i,j,0)] = 255;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 0;

				}else{

					m_Buffer[indBuf(i,j,0)] = 0;
					m_Buffer[indBuf(i,j,1)] = 0;
					m_Buffer[indBuf(i,j,2)] = 0;
				}

			}
		}

	}

	void boundingBox(BYTE* m_Buffer){
		bool left, right, up, down;
		left = right = up = down = true;
		bool l_isBlue, r_isBlue, u_isBlue, d_isBlue;
		l_isBlue = r_isBlue = u_isBlue = d_isBlue = false;
		int lpos, rpos, upos, dpos;
		lpos = rpos = t_avgj;
		upos = dpos = t_avgi;
		if((lpos - 1) >= 0){ lpos--;}
		if((rpos + 1) < t_width){ rpos++; }
		if((dpos - 1) >= 0){ dpos--; }
		if((upos + 1) < t_height){ upos++; }

		while(left || right || up || down){
			for(int a=dpos; a<upos; a++){
				if(!(    m_Buffer[indBuf(a,lpos,2)] == 0
					  && m_Buffer[indBuf(a,lpos,1)] == 0
					  && m_Buffer[indBuf(a,lpos,0)] == 0)) l_isBlue = true;
				if(!(    m_Buffer[indBuf(a,rpos,2)] == 0
					  && m_Buffer[indBuf(a,rpos,1)] == 0
					  && m_Buffer[indBuf(a,rpos,0)] == 0)) r_isBlue = true;
			}
			if(!l_isBlue) left = false;
			if(!r_isBlue) right = false;
			l_isBlue = r_isBlue = false;
			for(int b=lpos; b<rpos; b++){
				if(!(    m_Buffer[indBuf(upos,b,2)] == 0
					  && m_Buffer[indBuf(upos,b,1)] == 0
					  && m_Buffer[indBuf(upos,b,0)] == 0)) u_isBlue = true;
				if(!(    m_Buffer[indBuf(dpos,b,2)] == 0
					  && m_Buffer[indBuf(dpos,b,1)] == 0
					  && m_Buffer[indBuf(dpos,b,0)] == 0)) d_isBlue = true;
			}
			if(!d_isBlue) down = false;
			if(!u_isBlue) up = false;
			d_isBlue = u_isBlue = false;

			if((lpos - 1) >= 0 && left){ lpos--;}
			else{ left = false; }
			if((rpos + 1) < t_width && right){ rpos++; }
			else{ right = false; }
			if((dpos - 1) >= 0 && down){ dpos--; }
			else{ down = false; }
			if((upos + 1) < t_height && up){ upos++; }
			else{ up = false; }
		}
		for(int a=dpos; a<upos; a++){
			m_Buffer[indBuf(a,lpos,2)] = 255;
			m_Buffer[indBuf(a,lpos,1)] = 255;
			m_Buffer[indBuf(a,lpos,0)] = 255;
			m_Buffer[indBuf(a,rpos,2)] = 255;
			m_Buffer[indBuf(a,rpos,1)] = 255;
			m_Buffer[indBuf(a,rpos,0)] = 255;
		}
		for(int b=lpos; b<rpos; b++){
			m_Buffer[indBuf(upos,b,2)] = 255;
			m_Buffer[indBuf(upos,b,1)] = 255;
			m_Buffer[indBuf(upos,b,0)] = 255;
			m_Buffer[indBuf(dpos,b,2)] = 255;
			m_Buffer[indBuf(dpos,b,1)] = 255;
			m_Buffer[indBuf(dpos,b,0)] = 255;
		}
		/*for(int c = t_avgi-8; c < t_avgi+8; c++){
			for(int d = t_avgj-8; d < t_avgj+8; d++){
				if(c >= 0 && c < t_height && d >= 0 && d < t_width){
					m_Buffer[indBuf(c,d,2)] = 255;
					m_Buffer[indBuf(c,d,1)] = 255;
					m_Buffer[indBuf(c,d,0)] = 255;
				}
			}
		}*/
	}

	void TrackSetup(BYTE* m_Buffer){
		//rgbMax(m_Buffer);
		greyWorld(m_Buffer);
		rgNormalize(m_Buffer);
		//greyWorld(m_Buffer);
		//rgbMax(m_Buffer);
		//findBlue(m_Buffer);
		extractBlue(m_Buffer);
		boundingBox(m_Buffer);
	}

public:
	std::ofstream t_debug;
	//std::ofstream t_output;

private:
	tPvFrame * t_frame;
	int t_bluepixels;
	vector<int> t_coord;
	vector<int> t_thresh;
	unsigned long t_height;
	unsigned long t_width;
	int t_avgi, t_avgj;
	


};