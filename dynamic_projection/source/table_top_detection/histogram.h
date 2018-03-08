#ifndef _HISTOGRAM_H_
#define _HISTOGRAM_H_

#include <vector>
#include <cassert>
#include <iostream>

#include "color.h"

// ======================================================================
// HISTOGRAM
// ======================================================================

class Histogram {
public:

  Histogram() : hist(NUM_COLORS,0) {}
  int &operator[](int idx){
    assert(idx >= 0);
    assert(idx < (int)hist.size());
    return hist[idx];
  }
  int peak_idx(){
    int best_idx = 0;
    int max_count = hist[0];
    //std::cout << "PEAK";
    for (unsigned i=0; i<hist.size(); i++){
      //std::cout << "  " << hist[i];
      if (hist[i] > max_count){
        max_count = hist[i];
        best_idx = i;
      }
    }
    //std::cout << std::endl;
    return best_idx;
  }

  int total() const {
    int answer = 0;
    for (unsigned int i = 0; i < hist.size(); i++) { answer += hist[i]; }
    assert (answer > 0);
    return answer;
  }

  std::vector<std::pair<int,double> > calculate_percents() {

    std::vector<std::pair<int,double> > answer;

    int tot = total();

    for (unsigned int i = 0; i < hist.size(); i++) { 
      answer.push_back(std::make_pair(i,hist[i]/double(tot)));
    }
    return answer;
  }

private:

  std::vector<int> hist;
		       

};


inline Histogram dominant_color_idx(std::vector<Point> &points, Image<byte> *selected_components,
                                    const Image<sRGB> *colors) {
  assert (selected_components != NULL);
  assert (colors != NULL);
  Histogram hist;//(NUM_COLORS);
  for (unsigned i=0; i<points.size(); i++){
    int bin = (*selected_components)(int(points[i].row), int(points[i].col));
    hist[bin]++;
  }
  return hist;
}


#endif
