#include <stdio.h>
#include <stdlib.h>
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
#include <unistd.h>
#include <utility>

/* use -lpthread or -pthread on the gcc line */
#include <pthread.h>

/* for open sound control */
#include "osc/OscOutboundPacketStream.h"
#include "ip/UdpSocket.h"
using namespace std;

//localhost
//#define ADDRESS "127.0.0.1"
//Tyler's Laptop Empac
#define ADDRESS "128.213.17.125"
#define PORT 7000

#define OUTPUT_BUFFER_SIZE 1024

//Globals=========================
/* Device name */
const char* devM2 = "/dev/m2";
const char* devM3 = "/dev/m3";
const char* devM4 = "/dev/m4";
//tyler's logitech mouse
//const char* devM5 = "/dev/m5";
vector<const char *> mice;
vector<input_event> events;
vector< pair<int, int> > mpos;
vector< int > wpos;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//================================

/* Decode the type of mouse movement (X or Y) */
const char* strCode(short code)
{
	switch (code) {
		case 0: return "X";
		case 1: return "Y";
		case 8: return "WHEEL";
		case 272: return "LEFT";
		case 273: return "RIGHT";
		default: return "ERR: not a mouse movement!";
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
//	//Read MultiMouse ==================
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
		fprintf(stderr, "Can't open file, errno:%d (%s)\n", errno, strerror(errno));
		exit(1);
	}
	
	UdpTransmitSocket transmitSocket( IpEndpointName( ADDRESS, PORT ) );
    char buffer[OUTPUT_BUFFER_SIZE];
    
	pthread_mutex_unlock( &mutex );    /* release the lock */
	
	
	
	while(fread(&events[mNum], sizeof(events[mNum]), 1, fIn) == 1){
		//cout << "I'm here" << mNum + 2 << endl;	
		//cout << events[mNum].code << endl;
		//cout << "herherherh" << endl;
		
		//for mouse or wheel motion
		if (events[mNum].type == EV_REL){
			//printf("M%d %s %d\n ", mNum+2, 
			//	strCode(events[mNum].code), events[mNum].value);
			if(strCode(events[mNum].code) == "WHEEL"){
//				osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//				p << osc::BeginBundleImmediate << osc::BeginMessage( "wheel" ) 
//				  << mNum+2 << events[mNum].value << osc::EndMessage;
//				transmitSocket.Send( p.Data(), p.Size() );
				int mult = 8;
				if(  (wpos[mNum] + (events[mNum].value*mult)) >= 0 
				  && (wpos[mNum] + (events[mNum].value*mult)) < 128){
					wpos[mNum] += (events[mNum].value * mult);
				}
			}
			else if(strCode(events[mNum].code) == "X"){
//				osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//				p << osc::BeginBundleImmediate << osc::BeginMessage( "wheel" ) 
//				  << mNum+2 << events[mNum].value << osc::EndMessage;
//				transmitSocket.Send( p.Data(), p.Size() );
				if(  (mpos[mNum].first + events[mNum].value) >= 0 
				  && (mpos[mNum].first + events[mNum].value) < 1000){
					mpos[mNum].first += events[mNum].value;
				}
			}
			else if(strCode(events[mNum].code) == "Y"){
//				osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
//				p << osc::BeginBundleImmediate << osc::BeginMessage( "wheel" ) 
//				  << mNum+2 << events[mNum].value << osc::EndMessage;
//				transmitSocket.Send( p.Data(), p.Size() );
				if(  (mpos[mNum].second - events[mNum].value) >= 0 
				  && (mpos[mNum].second - events[mNum].value) < 1000){
					mpos[mNum].second -= events[mNum].value;
				}
			}
			cout << "M" << mNum+2 << " (x,y): (" << mpos[mNum].first << "," 
				<< mpos[mNum].second << ")" << endl;
			cout << "M" << mNum+2 << " wheel: " << wpos[mNum] << endl;
			
			osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
			p << osc::BeginBundleImmediate << osc::BeginMessage( "wheel" ) 
			  << mNum+2 << wpos[mNum] << osc::EndMessage;
			transmitSocket.Send( p.Data(), p.Size() );		
		}
		//for key press down
		else if(events[mNum].type == EV_KEY && events[mNum].value == 1){
			printf("M%d %s-Down\n ", mNum+2, strCode(events[mNum].code));
			
		}
		//for key press up
		else if(events[mNum].type == EV_KEY && events[mNum].value == 0){
			printf("M%d %s-Up\n ", mNum+2, strCode(events[mNum].code));
			if(strCode(events[mNum].code) == "LEFT"){
				osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
				p << osc::BeginBundleImmediate << osc::BeginMessage( "left" ) 
				  << mNum+2 << osc::EndMessage;
				transmitSocket.Send( p.Data(), p.Size() );
			}else if(strCode(events[mNum].code) == "RIGHT"){
				osc::OutboundPacketStream p( buffer, OUTPUT_BUFFER_SIZE );
				p << osc::BeginBundleImmediate << osc::BeginMessage( "right" ) 
				  << mNum+2 << osc::EndMessage;
				transmitSocket.Send( p.Data(), p.Size() );
			}
		}
	}
	free(arg);
	fclose(fIn);
	return NULL;
}

int main(int argc, char* argv[]){
	
	int numMice = 3;
	//tyler's Logitech
	//int numMice = 4;
	pthread_t tid[ numMice ];   /* keep track of the thread IDs */
	
	mice.push_back(devM2);
	mice.push_back(devM3);
	mice.push_back(devM4);
	//Tyler's Logitech
	//mice.push_back(devM5);
	
	//initialize some global vectors
	for(int i=0; i<mice.size(); i++){
		input_event holder;
		events.push_back(holder);
		mpos.push_back(make_pair(0,0));
		wpos.push_back(0);
	}	
	
//	input_event ev2;
//	input_event ev3;
//	input_event ev4;
	/* Open device file */
//	FILE* fIn2 = fopen(devM2, "r");
//	FILE* fIn3 = fopen(devM3, "r");
//	FILE* fIn4 = fopen(devM4, "r");

//	if (!(fIn2 || fIn3 || fIn4)) {
//		fprintf(stderr, "Can't open file, errno:%d (%s)\n", errno, strerror(errno));
//		exit(1);
//	}

	/* Read from device file */
	int rc, i;
	int * mNum;
	for(i=0; i<numMice; i++){
		mNum = (int *)malloc( sizeof( int ) );
    	*mNum = i;
		//mNum = i;
		rc = pthread_create( &tid[i], NULL, childThread, mNum );
		if ( rc != 0 ){
			perror( "MAIN: Could not create child thread" );
		}	
	}
	for(int i=0; i<numMice; i++){
		pthread_join( tid[i], NULL );
	}
	
//	while (1){
//		//check each mouse for new input
//		mouse_handler(fIn2, ev2, 2, 0, 50);
//		mouse_handler(fIn3, ev3, 3, 0, 50);
//		mouse_handler(fIn4, ev4, 4, 4, 0);
//	}

//	fclose(fIn2);
//	fclose(fIn3);
//	fclose(fIn4);
	return 0;
}


