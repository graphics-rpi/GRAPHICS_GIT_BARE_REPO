#include "vcDraw.h"

void vcDraw::initial_draw(){

	static GLfloat amb[] =  {0.4, 0.4, 0.4, 0.0};
  static GLfloat dif[] =  {1.0, 1.0, 1.0, 0.0};
  //  double fdw = _args->tiled_display.full_display_width;

  float s = 0.0;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT1);
  glDisable(GL_LIGHT2);
  amb[3] = dif[3] = cos(s) / 2.0 + 0.5;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glClear(GL_DEPTH_BUFFER_BIT);
  glEnable(GL_LIGHT2);
  glDisable(GL_LIGHT1);
  amb[3] = dif[3] = 0.5 - cos(s * .95) / 2.0;
  glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);

  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  _args->tiled_display.ORTHO();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  
}

void vcDraw::draw_cursors(){
	Vec3f color;
  glPointSize(20);
  glBegin(GL_POINTS);
  color = _global_colors.GetColor(PRIMARY_MOUSE);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,_args->tiled_display.full_display_height-GLOBAL_PRIMARY_MOUSE_POS.y);
  //glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,GLOBAL_PRIMARY_MOUSE_POS.y);

  color = _global_colors.GetColor(MOUSE_2);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_2_POS.x,_args->tiled_display.full_display_height-GLOBAL_MOUSE_2_POS.y);
  color = _global_colors.GetColor(MOUSE_3);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_3_POS.x,_args->tiled_display.full_display_height-GLOBAL_MOUSE_3_POS.y);
  color = _global_colors.GetColor(MOUSE_4);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_4_POS.x,_args->tiled_display.full_display_height-GLOBAL_MOUSE_4_POS.y);
  color = _global_colors.GetColor(MOUSE_5);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_5_POS.x,_args->tiled_display.full_display_height-GLOBAL_MOUSE_5_POS.y);
  color = _global_colors.GetColor(MOUSE_6);
  glColor3f(color.x(),color.y(),color.z());
  glVertex2f(GLOBAL_MOUSE_6_POS.x,_args->tiled_display.full_display_height-GLOBAL_MOUSE_6_POS.y);
  glEnd();
  
  glPointSize(10);
  glBegin(GL_POINTS);
  if(is_clicked[0]){
		color = _global_colors.GetColor(PRIMARY_MOUSE);
		glColor3f(1.0-color.x(),1.0-color.y(),1.0-color.z());
		glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,_args->
		tiled_display.full_display_height-GLOBAL_PRIMARY_MOUSE_POS.y);
		//glVertex2f(GLOBAL_PRIMARY_MOUSE_POS.x,GLOBAL_PRIMARY_MOUSE_POS.y);
	}if(is_clicked[1]){
		color = _global_colors.GetColor(MOUSE_2);
		glColor3f(1.0-color.x(),1.0-color.y(),1.0-color.z());
		glVertex2f(GLOBAL_MOUSE_2_POS.x,_args->
		tiled_display.full_display_height-GLOBAL_MOUSE_2_POS.y);
	}if(is_clicked[2]){ 	
		color = _global_colors.GetColor(MOUSE_3);
		glColor3f(1.0-color.x(),1.0-color.y(),1.0-color.z());
		glVertex2f(GLOBAL_MOUSE_3_POS.x,_args->
		tiled_display.full_display_height-GLOBAL_MOUSE_3_POS.y);
	}if(is_clicked[3]){	
		color = _global_colors.GetColor(MOUSE_4);
		glColor3f(1.0-color.x(),1.0-color.y(),1.0-color.z());
		glVertex2f(GLOBAL_MOUSE_4_POS.x,_args->
		tiled_display.full_display_height-GLOBAL_MOUSE_4_POS.y);
	}if(is_clicked[4]){	
		color = _global_colors.GetColor(MOUSE_5);
		glColor3f(1.0-color.x(),1.0-color.y(),1.0-color.z());
		glVertex2f(GLOBAL_MOUSE_5_POS.x,_args->
		tiled_display.full_display_height-GLOBAL_MOUSE_5_POS.y);
	}if(is_clicked[5]){	
		color = _global_colors.GetColor(MOUSE_6);
		glColor3f(1.0-color.x(),1.0-color.y(),1.0-color.z());
		glVertex2f(GLOBAL_MOUSE_6_POS.x,_args->
		tiled_display.full_display_height-GLOBAL_MOUSE_6_POS.y);
	}
	glEnd();

}
void vcDraw::draw_sliders(){

	 //tyler's attempt at lines
  glBegin(GL_LINES);
  glColor3f(1.0,1.0,1.0);
  for(unsigned int i=0; i<_volume_sliders.size(); i++){
  	glVertex3f((_volume_sliders[i].button_position.x + 
  							(_volume_sliders[i].button_width * 0.5)),
  							_volume_sliders[i].button_min_height, 0.0);
		glVertex3f((_volume_sliders[i].button_position.x + 
  							(_volume_sliders[i].button_width * 0.5)),
  							_volume_sliders[i].button_max_height, 0.0);
  }
  for(unsigned int i=0; i<_eq_sliders.size(); i++){
  	glVertex3f((_eq_sliders[i].button_position.x + 
  							(_eq_sliders[i].button_width * 0.5)),
  							_eq_sliders[i].button_min_height, 0.0);
		glVertex3f((_eq_sliders[i].button_position.x + 
  							(_eq_sliders[i].button_width * 0.5)),
  							_eq_sliders[i].button_max_height, 0.0);
  }
  for(unsigned int i=0; i<_fx_sliders.size(); i++){
  	glVertex3f((_fx_sliders[i].button_position.x + 
  							(_fx_sliders[i].button_width * 0.5)),
  							_fx_sliders[i].button_min_height, 0.0);
		glVertex3f((_fx_sliders[i].button_position.x + 
  							(_fx_sliders[i].button_width * 0.5)),
  							_fx_sliders[i].button_max_height, 0.0);
  }
  glEnd( );

	for (unsigned int i = 0; i < _volume_sliders.size(); i++) {
    _volume_sliders[i].button->paint();
  }
  for (unsigned int i = 0; i < _eq_sliders.size(); i++) {
    _eq_sliders[i].button->paint();
  }
  for (unsigned int i = 0; i < _fx_sliders.size(); i++) {
    _fx_sliders[i].button->paint();
  }
  for (unsigned int i = 0; i < _buttons.size(); i++) {
    _buttons[i]->paint();
  }
}

void vcDraw::draw_strokes(bool button_strokes){

	if(!_args->include_lasers){
		assert (_global_point_tracker != NULL);

		for (unsigned int i = 0; i < _global_point_tracker->size(); i++) {
		  int id = (*_global_point_tracker)[i].getID();
		  bool pressing = false;
		  // check if this tracker is attached to a button
		  for (unsigned int j = 0; j < _volume_sliders.size(); j++) {
		    if (_volume_sliders[j].button->isPressed() && 
		    		_volume_sliders[j].button->PressedBy() == id) 
					pressing = true;
		  }
		  for (unsigned int j = 0; j < _eq_sliders.size(); j++) {
		    if (_eq_sliders[j].button->isPressed() &&
		    		_eq_sliders[j].button->PressedBy() == id) 
					pressing = true;
		  }
		  
		  if (!pressing && button_strokes) continue;
		  if (pressing && !button_strokes) continue;

		  if (!(*_global_point_tracker)[i].IsActive()) continue;

		  // SET COLOR
		  Vec3f color = _global_colors.GetColor(id);
		  glColor3f(color.x(),color.y(),color.z());

		  // DRAW TRAIL
		  glLineWidth(3);
		  Path::draw_smooth_stroke((*_global_point_tracker)[i].getPtTrail());
		}
  }
} 

void vcDraw::initialize_sliders(){
	
	//drawRoutine._buttons.clear();
	
	double fdw = _args->tiled_display.full_display_width;
  double fdh = _args->tiled_display.full_display_height;
  
  double b_width = fdw * 0.04;
  double b_height = b_width;
  
  _volume_sliders.clear();

  _volume_sliders.push_back(MySlider("drums" ,0.75,  Vec3f(1,0,0), 
  	"../source/applications/volume_control/drums.ppm",
  	b_width, b_height, (fdw*(0.05+0.07*1)), (fdh*0.75), (fdh*0.5), (fdh*0.8)));
  _volume_sliders.push_back(MySlider("keyboard",0.75,  Vec3f(1,1,0), 
    "../source/applications/volume_control/keyboard.ppm",
    b_width, b_height, (fdw*(0.05+0.07*2)), (fdh*0.75), (fdh*0.5), (fdh*0.8)));
  _volume_sliders.push_back(MySlider("flute",0.75,  Vec3f(0,1,0), 
  	"../source/applications/volume_control/flute.ppm",
  	b_width, b_height, (fdw*(0.05+0.07*3)), (fdh*0.75), (fdh*0.5), (fdh*0.8)));
  _volume_sliders.push_back(MySlider("bass" ,0.75,Vec3f(0,0.5,1), 
  	"../source/applications/volume_control/guitar.ppm",
  	b_width, b_height, (fdw*(0.05+0.07*4)), (fdh*0.75), (fdh*0.5), (fdh*0.8)));
  	
	for(unsigned int i=0; i<_volume_sliders.size(); i++){
		_volume_sliders[i].initialize_button();
	}
  	
  	
	b_width = fdw * 0.025;
  b_height = b_width;
  
  _eq_sliders.clear();

  _eq_sliders.push_back(MySlider("eq1" ,0.5,  Vec3f(0.8,0.8,0.8), "",
  	b_width, b_height, (fdw*(0.45+0.05*1)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  _eq_sliders.push_back(MySlider("eq2",0.5,  Vec3f(0.8,0.8,0.8), "",
    b_width, b_height, (fdw*(0.45+0.05*2)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  _eq_sliders.push_back(MySlider("eq3",0.5,  Vec3f(0.8,0.8,0.8), "",
  	b_width, b_height, (fdw*(0.45+0.05*3)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  _eq_sliders.push_back(MySlider("eq4" ,0.5,Vec3f(0.8,0.8,0.8), "",
  	b_width, b_height, (fdw*(0.45+0.05*4)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
	_eq_sliders.push_back(MySlider("eq5" ,0.5,  Vec3f(0.8,0.8,0.8), "",
  	b_width, b_height, (fdw*(0.45+0.05*5)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  _eq_sliders.push_back(MySlider("eq6",0.5,  Vec3f(0.8,0.8,0.8), "",
    b_width, b_height, (fdw*(0.45+0.05*6)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  _eq_sliders.push_back(MySlider("eq7",0.5,  Vec3f(0.8,0.8,0.8), "",
  	b_width, b_height, (fdw*(0.45+0.05*7)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  _eq_sliders.push_back(MySlider("eq8" ,0.5,Vec3f(0.8,0.8,0.8), "",
  	b_width, b_height, (fdw*(0.45+0.05*8)), (fdh*0.65-0.5*b_height), (fdh*0.5), (fdh*0.8)));
  	
  for(unsigned int i=0; i<_eq_sliders.size(); i++){
		_eq_sliders[i].initialize_button();
	}
  	
	b_width = fdw * 0.04;
  b_height = b_width;
  	
	_fx_sliders.clear();

	//overdrive sliders
  _fx_sliders.push_back(MySlider("od_drums" ,0.0,  Vec3f(1.0,0.4,0.0), "",
  	b_width, b_height, (fdw*(0.05+0.07*1)), (fdh*0.1-0.5*b_height), (fdh*0.1), (fdh*0.4)));
  	
	_fx_sliders.push_back(MySlider("od_keyboard" ,0.0,  Vec3f(1.0,0.4,0.0), "",
  	b_width, b_height, (fdw*(0.05+0.07*2)), (fdh*0.1-0.5*b_height), (fdh*0.1), (fdh*0.4)));
  	
	_fx_sliders.push_back(MySlider("od_flute" ,0.0,  Vec3f(1.0,0.4,0.0), "",
  	b_width, b_height, (fdw*(0.05+0.07*3)), (fdh*0.1-0.5*b_height), (fdh*0.1), (fdh*0.4)));
  	
	_fx_sliders.push_back(MySlider("od_bass" ,0.0,  Vec3f(1.0,0.4,0.0), "",
  	b_width, b_height, (fdw*(0.05+0.07*4)), (fdh*0.1-0.5*b_height), (fdh*0.1), (fdh*0.4)));
  	
  	
	//phase shifter slider
	_fx_sliders.push_back(MySlider("ps" ,0.33,  Vec3f(0.6,0.6,0.6), "",
  	b_width, b_height, (fdw*(0.68)), (fdh*0.2-0.5*b_height), (fdh*0.1), (fdh*0.4)));
  	
	for(unsigned int i=0; i<_fx_sliders.size(); i++){
		_fx_sliders[i].initialize_button();
	}
  	
  	
	b_width = fdw * 0.025;
  b_height = b_width;
  	
	_buttons.clear();
	
	Button *b = new Button(Pt((fdw*0.47-(b_width/2)),(fdh*0.9)), b_width, b_height, 
    Vec3f(0,1,0),"Play");
	_buttons.push_back(b);
  //_buttons[_buttons.size()-1]->addText("Play");
  _buttons[_buttons.size()-1]->enable_texture(); 
  
  b = new Button(Pt((fdw*0.53-(b_width/2)),(fdh*0.9)), b_width, b_height, 
    Vec3f(1,0,0),"");
  _buttons.push_back(b);
  _buttons[_buttons.size()-1]->addText("Stop");
  _buttons[_buttons.size()-1]->enable_texture(); 
  
  b = new Button(Pt((fdw*0.6),(fdh*0.3)), b_width*1.4, b_height*1.4,
  	Vec3f(0,0.6,0.6), "");
  _buttons.push_back(b);
  _buttons[_buttons.size()-1]->clearText();
  _buttons[_buttons.size()-1]->addText("Pitchshifter");
  _buttons[_buttons.size()-1]->addText("OFF");
  _buttons[_buttons.size()-1]->enable_texture();

}

