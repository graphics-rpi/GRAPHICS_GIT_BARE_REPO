using namespace std;

class Listener{

public:

	Listener();
	Listener(ALfloat Pos[], ALfloat Vel[], ALfloat Ori[]);
	~Listener();
	void SetListener(ALfloat Pos[], ALfloat Vel[], ALfloat Ori[]);
	void ALSetListener();
	void SetPos(ALfloat Pos[]);
	void SetVel(ALfloat Vel[]);
	void SetOri(ALfloat Ori[]);
	ALfloat *GetPos();
	ALfloat *GetVel();
	ALfloat *GetOri();

	friend ostream& operator<< (ostream &ostr, const Listener &L) {

		ostr << "\nVelocity:" << '\n';

		for(int i = 0; i < 3; ++i)
		{

			ostr << L.ListenerVel[i] << " ";

		}

		ostr << '\n' << "Position:" << '\n';

		for(int i = 0; i < 3; ++i)
		{

			ostr << L.ListenerPos[i] << " ";

		}

		ostr << '\n' << "Orientation:" << '\n';

		for(int i = 0; i < 3; ++i)
		{

			ostr << L.ListenerOri[i] << " ";

		}

		return ostr;
	}

private:

	// Orientation of the Listener. (first 3 elements are "at", second 3 are "up")
	// Also note that these should be units of '1'.
	ALfloat *ListenerPos;
	ALfloat *ListenerVel;
	ALfloat *ListenerOri;

};
