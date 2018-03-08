
// Included files for OpenGL Rendering
#include "../paint/gl_includes.h"


#include "argparser.h"
#include "../paint/button.h"
#include "../paint/path.h"
#include "../../common/Image.h"
#include "../../multi_cursor/key_and_mouse_logger.h"

#include "../../calibration/planar_interpolation_calibration/colors.h"

#include "MySlider.h"

//#include "pong.h"
//#include "../paint/text.h"

class vcDraw {

public:
//public member variables

	ArgParser *_args;
	Colors _global_colors;
	std::vector<bool> is_clicked;
	//std::vector<Button> _buttons;
	std::vector<MySlider> _volume_sliders;
	std::vector<MySlider> _eq_sliders;
	std::vector<MySlider> _fx_sliders;
	std::vector<Button*> _buttons;
	PointTracker *_global_point_tracker;

public:
//public methods
	vcDraw(){ /*do nothing*/ }
	vcDraw(ArgParser *a, Colors &gc, PointTracker *gpt){
		_args = a;
		_global_colors = gc; 
		for(int i=0; i<6; i++){
			is_clicked.push_back(false);
		}
		_global_point_tracker = gpt;
	}
	void set_clicked(int mouse, bool tf){
		is_clicked[mouse] = tf;
	}
	void initial_draw();
	void initialize_sliders();
	void draw_cursors();
	//void draw_buttons();
	void draw_sliders();
	void draw_strokes(bool button_strokes);
	
	
	
};
