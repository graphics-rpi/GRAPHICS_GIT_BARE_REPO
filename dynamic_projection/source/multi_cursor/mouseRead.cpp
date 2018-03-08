#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <linux/input.h>
/* According to POSIX.1-2001 */
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "key_and_mouse_logger.h"
#include <unistd.h>

/* use -lpthread or -pthread on the gcc line */
#include <pthread.h>
using namespace std;

//Globals============================================
/* Device name */
const char* devM2 = "/dev/m2";
const char* devM3 = "/dev/m3";
const char* devM4 = "/dev/m4";
const char* devM5 = "/dev/m5";
const char* devM6 = "/dev/m6";
const char* devBlank = "/dev/mBlank";
vector<const char *> mice;
vector<input_event> events;
vector< pair<int, int> > mpos;
vector< int > wpos;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//===================================================

#define MK_STATE_DIRECTORY                   "../state/mouse_and_keyboard"
#define MK_ACTION_FILENAME_TEMPLATE          "../state/mouse_and_keyboard/actions_XXX.txt"
DirLock global_mk_dirlock(MK_STATE_DIRECTORY);

//=======================================================
//stuff for key and mouse logging

void my_keyfunc (int which_keyboard, int key, int scancode, int action, int glfw_modifiers){/*do nothing*/}
//void my_specialkeyfunc (int which_keyboard, int specialkey, int x, int y, int glut_modifiers){/*do nothing*/}
void my_mousefunc (int which_mouse, int button, int action, int glfw_modifiers){/*do nothing*/}
void my_motionfunc (int which_mouse, double x, double y, int glfw_modifiers) {/*do nothing*/}
void my_scrollfunc (int which_mouse, double x, double y, int glfw_modifiers) { }

void clamp_to_display(Pt &pt) { /* do nothing */ }
					
//=======================================================

/* Decode the type of mouse movement (X or Y) */
const char* strCode(short code)
{
	switch (code) {
		case 0: return "X";
		case 1: return "Y";
		case 8: return "WHEEL";
		case 272: return "LEFT";
		case 273: return "RIGHT";
        case 274: return "MIDDLE";
		default: return "ERR: not a mouse movement!";
	}
}


int whichMouse(int m) {
  switch (m) {
  case 0: return MOUSE_2;
  case 1: return MOUSE_3;
  case 2: return MOUSE_4;
  case 3: return MOUSE_5;
  case 4: return MOUSE_6;
//  case 0: return M2.getId();
//  case 1: return M3.getId();
//  case 2: return M4.getId();
//  case 3: return M5.getId();
//  case 4: return M6.getId();
  default: assert(0); exit(0);
  }
}

//void mouse_handler(FILE* fIn, input_event ev, int mouseNum, int time, int utime){
//	/* Read from device file */
//	struct timeval tv;
//	int retval;
//	/* Wait up to five seconds. */
//	tv.tv_sec = time;
//	tv.tv_usec = utime;
//	
//	//Read MultiMouse2 (m2)==================
//	fd_set fdset;
//	FD_ZERO(&fdset);
//	FD_SET(fileno(fIn), &fdset);
//	retval = select(fileno(fIn)+1, &fdset, NULL, NULL, &tv);
//	if(retval == 1){
//		if(fread(&ev, sizeof(ev), 1, fIn) == 1){
//			if (ev.type == EV_REL){
//				printf("M%d %s %d\n ", mouseNum, strCode(ev.code), ev.value);
//			}
//			else if(ev.type == EV_KEY && ev.value == 0){
//				printf("M%d %s-Down\n ", mouseNum, strCode(ev.code));
//			}
//			else if(ev.type == EV_KEY && ev.value == 1){
//				printf("M%d %s-Up\n ", mouseNum, strCode(ev.code));
//			}
//		}
//	}
//}

void * childThread(void * arg){
	int mNum = *(int *)arg;
	
	pthread_mutex_lock( &mutex );    /* get the lock */
	cout << "arg num " << mNum << endl;
	FILE * fIn = fopen(mice[mNum], "r");
	if (!fIn) {
		//fprintf(stderr, "Can't open file, errno:%d (%s)\n", errno, strerror(errno));
		//exit(1);
		std::cout << "couldn't open m" << mNum+2 << std::endl;
	}
	pthread_mutex_unlock( &mutex );    /* release the lock */
	if(fIn){
	  //struct stat buf;
	  
	   // while(1){
		//std::cout << "gets here" << std::endl;
		//if(stat(mice[mNum], &buf) != -1){ 
		  //std::cout << "gets here" << std::endl;
		  //usleep(1000);
	  	
			while(fread(&events[mNum], sizeof(events[mNum]), 1, fIn) == 1){
			//if(fread(&events[mNum], sizeof(events[mNum]), 1, fIn) == 1){
			  //mouse movement action (pos or wheel)
			  if (events[mNum].type == EV_REL){
			    //printf("M%d %s %d\n ", mNum+2, strCode(events[mNum].code), events[mNum].value);
			    if(strCode(events[mNum].code) == std::string("WHEEL")){
			      //int mult = 8;
			      //if(  (wpos[mNum] + (events[mNum].value*mult)) >= 0 
					  // && (wpos[mNum] + (events[mNum].value*mult)) < 128){
						//	wpos[mNum] += (events[mNum].value * mult);
			      //}
			      
			      //TO DO
			      //only works with state set to GLFW_RELEASE or GLFW_PRESS
			      //needs to work with any state
			      
			      //for wheel up
			      if(events[mNum].value > 0){
                        printf("M%d SCROLL UP\n ", mNum+2);
                        pthread_mutex_lock( &mutex );    // get the lock 
					    log_mousescroll(whichMouse(mNum), 0, 1);
					    pthread_mutex_unlock( &mutex );    // release the lock 
			      //for wheel down
			      }else if(events[mNum].value < 0){
                        printf("M%d SCROLL DOWN\n ", mNum+2);
					    pthread_mutex_lock( &mutex );    // get the lock 
					    log_mousescroll(whichMouse(mNum), 0, -1);
					    pthread_mutex_unlock( &mutex );    // release the lock 
			      
			      }
			      //log
			    }
			    else if(strCode(events[mNum].code) == std::string("X")){
			      //if(  (mpos[mNum].first + events[mNum].value) >= 0 
				 	  //&& (mpos[mNum].first + events[mNum].value) < 1000){
						//	mpos[mNum].first += events[mNum].value;
			      //}
                    printf("M%d X:%d\n ", mNum+2, events[mNum].value);
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mousemotion_relative(whichMouse(mNum),events[mNum].value,0.0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			      //log
			    }
			    else if(strCode(events[mNum].code) == std::string("Y")){
			      //if(  (mpos[mNum].second - events[mNum].value) >= 0 
				    //	&& (mpos[mNum].second - events[mNum].value) < 1000){
			  	//	mpos[mNum].second -= events[mNum].value;
			      //}
			    printf("M%d Y:%d\n ", mNum+2, events[mNum].value);
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mousemotion_relative(whichMouse(mNum),0.0,events[mNum].value);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			      //log
			    } else {
			      //std::cout << "some other code " << strCode(events[mNum].code << std::endl;
			    }
			  }
			  //mouse button down action
			  else if(events[mNum].type == EV_KEY && events[mNum].value == 1){
			    printf("M%d %s-Down\n ", mNum+2, strCode(events[mNum].code));
			    if(strCode(events[mNum].code) == std::string("LEFT")){
			      pthread_mutex_lock( &mutex );    // get the lock 
                  //TODO: SHOULDNT PASS 0 AS GLFW MODIFIERS TO LOG_MOUSEACTION AND OTHER FUNCS BELOW
			      log_mouseaction(whichMouse(mNum),GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS, 0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			    }else if(strCode(events[mNum].code) == std::string("RIGHT")){
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mouseaction(whichMouse(mNum), GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS, 0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			    }else if(strCode(events[mNum].code) == std::string("MIDDLE")){
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mouseaction(whichMouse(mNum), GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS, 0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			    }
			  }
			  //mouse button up action
			  else if(events[mNum].type == EV_KEY && events[mNum].value == 0){
			    printf("M%d %s-Up\n ", mNum+2, strCode(events[mNum].code));
			    if(strCode(events[mNum].code) == std::string("LEFT")){
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mouseaction(whichMouse(mNum),GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE, 0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			      //log
			    }else if(strCode(events[mNum].code) == std::string("RIGHT")){
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mouseaction(whichMouse(mNum),GLFW_MOUSE_BUTTON_RIGHT,GLFW_RELEASE, 0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			      //log
			    }else if(strCode(events[mNum].code) == std::string("MIDDLE")){
			      pthread_mutex_lock( &mutex );    // get the lock 
			      log_mouseaction(whichMouse(mNum), GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE, 0);
			      pthread_mutex_unlock( &mutex );    // release the lock 
			    }
			  }
			}
		fclose(fIn);
		//}
	    //}
	}
	  
	free(arg);
	
	return NULL;
}

int main(int argc, char* argv[]){
	
  //int numMice = 3;
	pthread_t tid[ NUM_NON_PRIMARY_MICE ];   /* keep track of the thread IDs */
	
	mice.push_back(devM2);
	mice.push_back(devM3);
	mice.push_back(devM4);
	mice.push_back(devM5);
	mice.push_back(devM6);

	// clean out mouse & keyboard state directory before we start
	int success = !system ("rm -f ../state/mouse_and_keyboard/*txt");
	assert (success);

	// all files created should be delete-able by normal users
	umask(0);
	
	for(unsigned int i=0; i<mice.size(); i++){
		input_event holder;
		events.push_back(holder);
		mpos.push_back(make_pair(0,0));
		wpos.push_back(0);
	}	

	/* Read from device file */
	int rc, i;
	int * mNum;
	for(i=0; i<NUM_NON_PRIMARY_MICE; i++){
		mNum = (int *)malloc( sizeof( int ) );
    	*mNum = i;
		rc = pthread_create( &tid[i], NULL, childThread, mNum );
		if ( rc != 0 ){
			perror( "MAIN: Could not create child thread" );
		}	
	}


	// parent thread
	while (1) {
	  //pthread_mutex_lock( &mutex );    /* get the lock */
	  //load and save
	  load_and_save_key_and_mouse_data(global_mk_dirlock,
              MK_ACTION_FILENAME_TEMPLATE,
              my_keyfunc,
              my_mousefunc,
              my_motionfunc,
              my_scrollfunc);
	  //usleep(1000);
	  usleep(1000);
	  //pthread_mutex_unlock( &mutex );    /* release the lock */
	}
	
	return 0;
}


