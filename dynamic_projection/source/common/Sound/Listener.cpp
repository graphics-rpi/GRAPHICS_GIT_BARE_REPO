#include <cstdlib>
#include <cstdio>
#include <iostream>

#ifdef __APPLE__
#include <AL/alut.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif



#include "Listener.h"
using namespace std;


//Default Constructor sets velocity to 0, position to the origin and a default orientation
Listener::Listener(){

	ListenerPos = new ALfloat[3];
	ListenerVel = new ALfloat[3];
	ListenerOri = new ALfloat[6];

	for(int i = 0; i < 3; ++i)
	{

		ListenerPos[i] = 0.0;
		ListenerVel[i] = 0.0;
		ListenerOri[i] = 0.0;
		ListenerOri[i+3] = 0.0;

	}

	ListenerOri[2] = -1.0;
	ListenerOri[4] = 1.0;
	ALSetListener();

}

//Constructor that initializes the position, velocity and orientation to user specified ones.
Listener::Listener(ALfloat Pos[], ALfloat Vel[], ALfloat Ori[]){

	ListenerPos = new ALfloat[3];
	ListenerVel = new ALfloat[3];
	ListenerOri = new ALfloat[6];

	for(int i = 0; i < 3; ++i)
	{

		ListenerPos[i] = Pos[i];
		ListenerVel[i] = Vel[i];
		ListenerOri[i] = Ori[i];
		ListenerOri[i+3] = Ori[i+3];

	}

	ALSetListener();

}

//Deconstructor cleans up the listener class
Listener::~Listener(){

  delete [] ListenerPos;
  delete [] ListenerVel;
  delete [] ListenerOri;

}

//This function is to set the listener to a new set of values
void Listener::SetListener(ALfloat Pos[], ALfloat Vel[], ALfloat Ori[]){

	for(int i = 0; i < 3; ++i)
	{

		ListenerPos[i] = Pos[i];
		ListenerVel[i] = Vel[i];
		ListenerOri[i] = Ori[i];
		ListenerOri[i+3] = Ori[i+3];

	}

	ALSetListener();

}

//This function must be called upon any change of listener parameters
//It makes calls to openAL to set up the object in the AL code.
void Listener::ALSetListener(){

	alListenerfv(AL_POSITION,    ListenerPos);
	alListenerfv(AL_VELOCITY,    ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);

}

//Sets position, sends it to AL
void Listener::SetPos(ALfloat Pos[]){

	for(int i = 0; i < 3; ++i)
	{

		ListenerPos[i] = Pos[i];

	}

	alListenerfv(AL_POSITION, ListenerPos);

}

//Sets velocity, sends it to AL
void Listener::SetVel(ALfloat Vel[]){

	for(int i = 0; i < 3; ++i)
	{

		ListenerVel[i] = Vel[i];

	}

	alListenerfv(AL_VELOCITY, ListenerVel);

}

//Sets orientation, sends it to AL
void Listener::SetOri(ALfloat Ori[]){

	for(int i = 0; i < 3; ++i)
	{

		ListenerOri[i] = Ori[i];

	}

	alListenerfv(AL_ORIENTATION, ListenerOri);

}


//Get the position
ALfloat *Listener::GetPos(){

	return ListenerPos;

}

//Get the velocity
ALfloat *Listener::GetVel(){

	return ListenerVel;

}

//Get the origin
ALfloat *Listener::GetOri(){

	return ListenerOri;

}

