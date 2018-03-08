#include <map>
#include "../common/MersenneTwister.h"

inline void CreateLabelsImage(const Image<int> &labels, const std::string &filename,
			      MTRand &mtrand ) {
  std::map<int,sRGB> label_colors;
  Image<sRGB> answer(labels.getRows(), labels.getCols());
  for (int row=0; row<labels.getRows(); row++){
    for (int col=0; col<labels.getCols(); col++){
      sRGB color(0,0,0);
      int which_label = labels(row,col);
      if (which_label > 0) {
	std::map<int,sRGB>::iterator itr = label_colors.find(which_label);
	if (itr == label_colors.end()) {
	  color = sRGB(255*mtrand.rand(),255*mtrand.rand(),255*mtrand.rand());
	  label_colors[which_label] = color;
	} else {
	  color = itr->second;
	}
      }
      answer(row, col) = color; 
    }
  }  
  //if (args->write_debug_images){
  answer.write(filename); //args->debug_path+"06_debug_labels.png");
  //}
}
