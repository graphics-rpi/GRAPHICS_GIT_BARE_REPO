#ifndef drawn_object_h_
#define drawn_object_h_

class drawn_object{
	public:
		virtual void render() {
			std::cout<<"WARNING: USING DEFAULT RENDER, THERE BE DRAGONS HERE\n";
			return;
		}

		/*virtual void render()=0;*/

		/*drawn_object(){}

		~drawn_object(){}*/
};

#endif
