#include <string>
#include <vector>
#include "../paint/button.h"

class MySlider {
public:

  MySlider(const std::string &n, double val, const Vec3f &c, 
  	const std::string &filename, double bw, double bh, double bx, double by,
  	double miny, double maxy) {
    name = n;
    value = val;
    color = c;
    image = filename;
    button_width = bw;
    button_height = bh;
    button_position = Pt(bx, by);
    button_max_height = maxy;
    button_min_height = miny;
  }
  void initialize_button(){
    button = new Button(button_position, button_width, button_height, 
    	color, image, image);
    button->addText(name);
    button->enable_texture(); 
  }
  
  //~MySlider(){ delete button; };
  
  std::string name;
  double value;
  Vec3f color;
  std::string image;
  double button_width;
  double button_height;
  Pt button_position;
  double button_max_height;
  double button_min_height;
  
  Button *button;
};
 
