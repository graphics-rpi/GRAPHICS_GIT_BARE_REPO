#include <cstdlib>
#include <cstdio>


#ifdef __APPLE__
#else
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#endif

#include <string>
#include <vector>
#include "Source.h"
#include "Listener.h"


class SoundHandler {

public:

	SoundHandler(double SpeedOfSound = 20.0);
	int AddSource(string Filename, bool Looping = false);
	int AddSource(ALfloat Vel[], ALfloat Pos[], string Filename, bool Looping = false);
	void PlaySource(int Num);
	void PauseSource(int Num);
	void StopSource(int Num);
	void SetOrigin(ALfloat Pos[]);
	void SetVelSource(int Num, ALfloat Vel[]);
	void SetPosSource(int Num, ALfloat Pos[]);
	void SetSourceGain(int Num, float Val);
	void ToggleSourceMoving(int Num);
	void ToggleSourceLoop(int Num);
	void AdvanceStep();
	void AdvanceSteps();
	void SetStep(double _Step);
	Source* GetSource(int Num);
	Listener* GetListener();

private:

	Listener Ear;
	vector <Source> Sources;
	double Step;

};
