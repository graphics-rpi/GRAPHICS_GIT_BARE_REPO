#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
#include <AL/alut.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <cassert>
#include "SoundHandler.h"

//Initiates the SoundHandler, SpeedOfSound is an
//AL metric that is unitless. It is an optional argument,
//20 is the default
SoundHandler::SoundHandler(double SpeedOfSound) {

	alutInit(NULL, 0);
	alGetError();
	alDopplerFactor(1.0);
	alDopplerVelocity(SpeedOfSound);
	Step = 1;

}


//Adds another source with default velocity and position given a filename
//looping is optional
int SoundHandler::AddSource(string Filename, bool Looping){

	Sources.push_back(Source(Filename, Looping));
	return Sources.size() - 1;

}

//Adds another source with the given parameters
//looping is optional
int SoundHandler::AddSource(ALfloat Vel[], ALfloat Pos[], string Filename, bool Looping){

	Sources.push_back(Source(Vel, Pos, Filename, Looping));
	return Sources.size() - 1;

}

//Plays a source given an index
void SoundHandler::PlaySource(int Num){

	assert((unsigned int)Num < Sources.size());
	Sources[Num].Play();
}

//Stops a source given an index
void SoundHandler::StopSource(int Num){

	assert((unsigned int)Num < Sources.size());
	Sources[Num].Stop();

}

void SoundHandler::SetOrigin(ALfloat Pos[]){

	for(unsigned int i = 0; i < Sources.size(); ++i)
	{

		if(i == 0)
			Ear.SetPos(Pos);

		Sources[i].SetPos(Pos);

	}

}


//Pauses a source given an index.
void SoundHandler::PauseSource(int Num){

	assert((unsigned int)Num < Sources.size());
	Sources[Num].Pause();
}

//Sets the velocity of the Num source.
void SoundHandler::SetVelSource(int Num, ALfloat Vel[]){

	assert((unsigned int)Num < Sources.size());
	Sources[Num].SetVel(Vel);

}

//Sets the position of the Num source.
void SoundHandler::SetPosSource(int Num, ALfloat Pos[]){

	assert((unsigned int)Num < Sources.size());
	Sources[Num].SetPos(Pos);

}

//Changes the gain value for the Num source.
void SoundHandler::SetSourceGain(int Num, float Val){

	Sources[Num].SetGain(Val);

}

//Flips the current toggle value, this will cause
//step advancing to move the given index's source
void SoundHandler::ToggleSourceMoving(int Num){

	assert((unsigned int)Num < Sources.size());
	Sources[Num].ToggleMove();

}

//Flips the loop value for a source Num
void SoundHandler::ToggleSourceLoop(int Num){

	Sources[Num].ToggleLoop();

}

//Moves the source one time step
void SoundHandler::AdvanceStep(){

	for(unsigned int i = 0; i < Sources.size(); ++i)
	{

		if(Sources[i].IsMoving())
			Sources[i].MoveStep(Step);

	}

}

//Moves all sources 100 steps
void SoundHandler::AdvanceSteps(){

	for(unsigned int j = 0; j < 100; ++j)
	{

// CHRIS STUETZLE:
// THIS WAS NOT COMPILING...PUT IT BACK IN AT SOME POINT!!!
//		usleep(70000);

		for(unsigned int i = 0; i < Sources.size(); ++i)
		{

			if(Sources[i].IsMoving())
				Sources[i].MoveStep(Step);

		}
	}

}

//Sets the step for the movement functions
void SoundHandler::SetStep(double _Step){

	Step = _Step;

}

//Returns a source object given an index
Source* SoundHandler::GetSource(int Num){

	return &Sources[Num];

}

//Returns the listener object.
Listener* SoundHandler::GetListener(){

	return &Ear;

}

