#include <cstdlib>
#include <cstdio>

#ifdef __APPLE__
#include <AL/alut.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#endif


#include <string>
#include <iostream>
#include "Source.h"
using namespace std;

//Default constructor initializes the source to do nothing
//Not recommended for use
Source::Source(){

	Move = false;
	SourcePos = new ALfloat[3];
	SourceVel = new ALfloat[3];

	for(unsigned int i = 0; i < 3; ++i)
	{

		SourcePos[i] = 0.0;	
		SourceVel[i] = 0.0;

	}

}

//Constructor that takes a file name and creates
//a source at the origin with default pitch and gain
//that is stationary. Looping is a default parameter
//if set to true that causes the playing file to loop
Source::Source(string Filename, bool Looping){

	SourceFile = Filename;
	Loop = Looping;
	Move = false;
	SourcePos = new ALfloat[3];
	SourceVel = new ALfloat[3];

	for(int i = 0; i < 3; ++i)
	{

		SourcePos[i] = 0.0;	
		SourceVel[i] = 0.0;

	}

	LoadALData(Looping);

}

//Constructor that takes a file name and creates
//a source at the given position with default pitch and gain
//with the given velocity. Looping is a default parameter
//if set to true that causes the playing file to loop
Source::Source(ALfloat Pos[], ALfloat Vel[], string Filename, bool Looping){

	SourceFile = Filename;
	Loop = Looping;
	Move = false;
	SourcePos = new ALfloat[3];
	SourceVel = new ALfloat[3];

	for(int i = 0; i < 3; ++i)
	{

		SourcePos[i] = Pos[i];
		SourceVel[i] = Vel[i];

	}

	LoadALData(Looping);

}

//Copy constructor
Source::Source(const Source &Copyme){

	SourceFile = Copyme.SourceFile;
	Loop = Copyme.Loop;
	Move = Copyme.Move;

	SourcePos = new ALfloat[3];
	SourceVel = new ALfloat[3];

	for(int i = 0; i < 3; ++i)
	{

		SourcePos[i] = Copyme.SourcePos[i];
		SourceVel[i] = Copyme.SourceVel[i];

	}

	LoadALData(Loop);

}

//Assignment operator
Source& Source::operator= (const Source &Copyme){

	SourceFile = Copyme.SourceFile;
	Loop = Copyme.Loop;
	Move = Copyme.Move;

	for(unsigned int i = 0; i < 3; ++i)
	{

		SourcePos[i] = Copyme.SourcePos[i];
		SourceVel[i] = Copyme.SourceVel[i];

	}	

	LoadALData();

	return *this;
}

//Destructor, also handles deleting AL resources
Source::~Source(){

  delete [] SourcePos;
  delete [] SourceVel;
	alDeleteBuffers(1, &Buffer);
	alDeleteSources(1, &ALSource);

}

//Sets a source all at once given position, velocity and a file name
void Source::SetSource(ALfloat Pos[], ALfloat Vel[], string Filename){

	SourceFile = Filename;

	for(int i = 0; i < 3; ++i)
	{

		SourcePos[i] = Pos[i];
		SourceVel[i] = Vel[i];

	}

	LoadALData();

}

//Loads the Data from the saved filename
bool Source::LoadALData(bool Looping){

	// Load wav Data into a buffer.
	alGenBuffers(1, &Buffer);

	if(alGetError() != AL_NO_ERROR){
		return AL_FALSE;
	}

	// Load the resources
	alutLoadWAVFile((ALbyte*)SourceFile.c_str(), &Format, &Data, &Size, &Freq, &Loop);
	// Load the Data into the buffer
	alBufferData(Buffer, Format, Data, Size, Freq);
	// Unload the resources
	alutUnloadWAV(Format, Data, Size, Freq);

	// Bind the buffer with the source.

	alGenSources(1, &ALSource);

	if(alGetError() != AL_NO_ERROR)
		return AL_FALSE;

	//Set the source object
	alSourcei (ALSource, AL_BUFFER,   Buffer   );
	alSourcef (ALSource, AL_PITCH,    1.0      );
	alSourcef (ALSource, AL_GAIN,     1.0      );

	alSourcefv(ALSource, AL_POSITION, SourcePos);
	alSourcefv(ALSource, AL_VELOCITY, SourceVel);

	if(Looping){
		alSourcei (ALSource, AL_LOOPING,  AL_TRUE   );
	}
	else{
		alSourcei (ALSource, AL_LOOPING,  AL_FALSE   );
	}

	// Do another error check and return.

	if(alGetError() == AL_NO_ERROR){
		return AL_TRUE;
	}

	return AL_FALSE;

}

//This takes a filename and loads all resources for file playing goodness.
bool Source::LoadALData(string Filename, bool Looping){

	alDeleteBuffers(1, &Buffer);
	alDeleteSources(1, &ALSource);

	SourceFile = Filename;

	ALenum Format;
	ALsizei size;
	ALvoid* Data;
	ALsizei freq;
	ALboolean loop;

	// Load wav Data into a buffer.
	alGenBuffers(1, &Buffer);

	if(alGetError() != AL_NO_ERROR)
		return AL_FALSE;

	// Load the resources
	alutLoadWAVFile((ALbyte*)Filename.c_str(), &Format, &Data, &Size, &freq, &loop);
	// Load the Data into the buffer
	alBufferData(Buffer, Format, Data, Size, freq);
	// Unload the resources
	alutUnloadWAV(Format, Data, Size, freq);

	// Bind the buffer with the source.

	alGenSources(1, &ALSource);

	if(alGetError() != AL_NO_ERROR)
		return AL_FALSE;

	//Set the source object
	alSourcei (ALSource, AL_BUFFER,   Buffer   );
	alSourcef (ALSource, AL_PITCH,    1.0      );
	alSourcef (ALSource, AL_GAIN,     1.0      );
	alSourcefv(ALSource, AL_POSITION, SourcePos);
	alSourcefv(ALSource, AL_VELOCITY, SourceVel);

	if(Looping)
		alSourcei (ALSource, AL_LOOPING,  AL_TRUE   );
	else
		alSourcei (ALSource, AL_LOOPING,  AL_FALSE   );

	// Do another error check and return.

	if(alGetError() == AL_NO_ERROR)
		return AL_TRUE;

	return AL_FALSE;

}

//Toggles movement
void Source::ToggleMove(){

	Move = !Move;

}

//Flips and sets the loop in open AL
void Source::ToggleLoop(){

	Loop = !Loop;
	alSourcei (ALSource, AL_LOOPING,  Loop   );

}

//Returns a bool as to if the sound is moving or not
bool Source::IsMoving(){

	return Move;

}

//Moves the source one step on it's velocity
void Source::MoveStep(double Step){

	for(unsigned int i = 0; i < 3; ++i)
	{

		SourcePos[i] = SourcePos[i] + SourceVel[i] * Step;

	}

	alSourcefv(ALSource, AL_POSITION, SourcePos);

}

//Gets the velocity
ALfloat *Source::GetVel(){

	return SourceVel;

}

//Gets the position
ALfloat *Source::GetPos(){

	return SourcePos;

}

//Sets the velocity for the source given a velocity array
void Source::SetVel(ALfloat Vel[]){

	for(int i = 0; i < 3; ++i)
	{

		SourceVel[i] = Vel[i];

	}

	alSourcefv(ALSource, AL_VELOCITY, SourceVel);

}

//Sets the position and send it to AL
void Source::SetPos(ALfloat Pos[]){

	for(int i = 0; i < 3; ++i)
	{

		SourcePos[i] = Pos[i];

	}

	alSourcefv(ALSource, AL_POSITION, SourcePos);

}

void Source::SetGain(ALfloat Gain){

	alSourcef (ALSource, AL_GAIN, Gain);

}

//Plays the current source file
void Source::Play(){

	alSourcePlay(ALSource);

}

//Stops the playback
void Source::Stop(){

	alSourceStop(ALSource);

}

//Pauses the playback
void Source::Pause(){

	alSourcePause(ALSource);

}

