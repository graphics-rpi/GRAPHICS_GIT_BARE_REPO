#include <string>
#include <iostream>
using namespace std;

class Source{

public:

	Source();
	Source(string Filename, bool Looping = false);
	Source(ALfloat Pos[], ALfloat Vel[], string Filename, bool Looping = false);
	Source(const Source &copyme);
	~Source();
	Source& operator= (const Source &Copyme);
	void SetSource(ALfloat Pos[], ALfloat Vel[], string Filename);
	bool LoadALData(bool Looping = false);
	bool LoadALData(string Filename, bool Looping = false);
	void ToggleMove();
	void ToggleLoop();
	bool IsMoving();
	void MoveStep(double Step = 1);
	ALfloat *GetVel();
	ALfloat *GetPos();
	void SetVel(ALfloat Vel[]);
	void SetPos(ALfloat Pos[]);
	void SetGain(ALfloat Gain);
	void Play();
	void Stop();
	void Pause();

	friend ostream& operator<< (ostream &ostr, const Source &S) {

		ostr << '\n' << S.SourceFile;

			ostr << "\nVelocity:" << '\n';

			for(int i = 0; i < 3; ++i)
			{

				ostr << S.SourceVel[i] << " ";

			}

			ostr << '\n' << "Position:" << '\n';

			for(int i = 0; i < 3; ++i)
			{

				ostr << S.SourcePos[i] << " ";

			}

			return ostr;
	}

private:

	string SourceFile;
	//The sources position
	ALfloat *SourcePos;
	//The sources velocity
	ALfloat *SourceVel;
	//Holds the sound data
	ALuint Buffer;
	//ALUT source number
	ALuint ALSource;
	//File type information
	ALenum Format;
	//File size information
	ALsizei Size;
	//File data
	ALvoid* Data;
	//The frequency of the file
	ALsizei Freq;
	//Looping bit
	ALboolean Loop;
	//Is the source moving?
	ALboolean Move;

};
