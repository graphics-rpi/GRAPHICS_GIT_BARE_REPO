#include <stdlib.h>
#include <stdio.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <iostream>
#include <string>
#include <unistd.h>
#include "SoundHandler.h"

using namespace std;

int main(int argc, char **argv){

	SoundHandler speaker = SoundHandler();
	ALfloat Vel[] = {-1.0,0.0,0.0};
	ALfloat Pos[] = {50.0,0.0,0.0};
	speaker.AddSource("wavdata/FancyPants.wav");
	speaker.AddSource(Pos, Vel, "wavdata/Creek.wav");

	char c = ' ';
	string file;
	int index;
	bool loop;
	ALfloat triplet[3];


	while(c != 'q')
	{
		c = getchar();

		switch(c)
		{

		// Pressing 'a' will add a sample.
		case 'a': case 'A':

			cout << "Enter filename: ";
			cin >> file;
			cout << "Loop?: ";
			cin >> loop;
			speaker.AddSource(file, loop);

			break;

			// Pressing 'p' will play a sample
		case 'p': case 'P':

			cout << "Source index: ";
			cin >> index;
			speaker.PlaySource(index);

			break;

			// Pressing 's' will stop the sample from playing.
		case 's': case 'S':

			cout << "Source index: ";
			cin >> index;
			speaker.StopSource(index);

			break;

			// Pressing 'h' will pause the sample.
		case 'h': case 'H':

			cout << "Source index: ";
			cin >> index;
			speaker.PauseSource(index);

			break;

		case 'v': case 'V':

			cout << "Velocity\n Source index: ";
			cin >> index;
			cout << "x: ";
			cin >> triplet[0];
			cout << "y: ";
			cin >> triplet[1];
			cout << "z: ";
			cin >> triplet[2];
			speaker.SetVelSource(index, triplet);

			break;

		case 'b': case 'B':

			cout << "Position\n Source index: ";
			cin >> index;
			cout << "x: ";
			cin >> triplet[0];
			cout << "y: ";
			cin >> triplet[1];
			cout << "z: ";
			cin >> triplet[2];
			speaker.SetPosSource(index, triplet);

			break;

		case 'm': case 'M':

			cout << "Toggle movement: ";
			cin >> index;
			speaker.ToggleSourceMoving(index);

			break;

		case 'o': case 'O':

			speaker.AdvanceSteps();

			break;

		}
	}

}
