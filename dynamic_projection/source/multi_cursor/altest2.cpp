#include <iostream>
 
//#include <conio.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <cstdio>
#include <cstring>
#include <string>
using namespace std;
 
// Buffers to hold sound data.
ALuint Buffer;
 
// Sources are points of emitting sound.
ALuint Source;
 
/*
 * These are 3D cartesian vector coordinates. A structure or class would be
 * a more flexible of handling these, but for the sake of simplicity we will
 * just leave it as is.
 */
 
// Position of the source sound.
ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };
 
// Velocity of the source sound.
ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };
 
// Position of the Listener.
ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
 
// Velocity of the Listener.
ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
 
// Orientation of the Listener. (first 3 elements are "at", second 3 are "up")
// Also note that these should be units of '1'.
ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
 
/*
 * ALboolean LoadALData()
 *
 *         This function will load our sample data from the disk using the Alut
 *         utility and send the data into OpenAL as a buffer. A source is then
 *         also created to play that buffer.
 */
ALboolean LoadALData()
{
	// Variables to load into.
	ALenum format;
	ALsizei size;
	ALvoid* data;
	ALsizei freq;
	ALboolean loop;

	// Load wav data into a buffer.
	alGenBuffers(1, &Buffer);

	if(alGetError() != AL_NO_ERROR)
		return AL_FALSE;
		
	//char* soundFileName;
	//strcpy(soundFileName, "./multi_mouse/dumbwavs/dumbtest.wav");

	// Load any of your favourite wav song here
	alutLoadWAVFile((signed char *)"./multi_mouse/dumbwavs/dumbtest.wav", &format, &data, &size, &freq, &loop);
	alBufferData(Buffer, format, data, size, freq);
	alutUnloadWAV(format, data, size, freq);

	// Bind the buffer with the source.
	alGenSources(1, &Source);

	if(alGetError() != AL_NO_ERROR)
		return AL_FALSE;

	alSourcei (Source, AL_BUFFER,   Buffer   );
	alSourcef (Source, AL_PITCH,    1.0      );
	alSourcef (Source, AL_GAIN,     1.0      );
	alSourcefv(Source, AL_POSITION, SourcePos);
	alSourcefv(Source, AL_VELOCITY, SourceVel);
	alSourcei (Source, AL_LOOPING,  loop     );

	// Do another error check and return.
	if(alGetError() == AL_NO_ERROR)
	return AL_TRUE;

	return AL_FALSE;
}
 
/*
 * void SetListenerValues()
 *
 *         We already defined certain values for the Listener, but we need
 *         to tell OpenAL to use that data. This function does just that.
 */
void SetListenerValues()
{
	alListenerfv(AL_POSITION,    ListenerPos);
	alListenerfv(AL_VELOCITY,    ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);
}
 
/*
 * void KillALData()
 *
 *         We have allocated memory for our buffers and sources which needs
 *         to be returned to the system. This function frees that memory.
 */
void KillALData()
{
	alDeleteBuffers(1, &Buffer);
	alDeleteSources(1, &Source);
	alutExit();
}
 
int main(int argc, char *argv[])
{
	cout << "Play OpenAL waveform audio file" << endl;
	cout << "Type a character controls:" << endl;
	cout << "p) To Play" << endl;
	cout << "s) To Stop playing" << endl;
	cout << "h) To Hold / Pause playing" << endl;
	cout << "q) To Quit playing" << endl << endl;
	
	ALenum eBufferFormat = 0; 
	eBufferFormat = alGetEnumValue("AL_FORMAT_71CHN16"); 
	if (!eBufferFormat) { 
		printf("No support for 7.1 playback!\n"); 
		//return 0; 
	}else{
		cout << "wooooo" << endl;
	}

	// Initialize OpenAL and clear the error bit. 
	alutInit(NULL, 0);
	alGetError();

	// Load the wav data. 
	if(LoadALData() == AL_FALSE) //method 1: LoadALData()
	{
	printf("Error loading data.");
		return 0;
	}

	SetListenerValues(); //method 2: void SetListenerValues()

	// Setup an exit procedure. 
	//atexit(KillALData); //method 3: void KillALData()

	// Loop. 
	char c = ' ';
	while(c != 'q')
	{
		//c = getche();
		cin >> c;
		switch(c)
		{
			// Pressing 'p' will begin playing the sample. 
			case 'p': alSourcePlay(Source); break;

			// Pressing 's' will stop the sample from playing. 
			case 's': alSourceStop(Source); break;

			// Pressing 'h' will pause the sample. 
			case 'h': alSourcePause(Source); break;
		};
	}
	KillALData();
	return 0;
} 






