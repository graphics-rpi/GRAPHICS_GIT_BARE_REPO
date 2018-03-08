#ifndef _IR_DATA_POINT_
#define _IR_DATA_POINT_

#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>

#include "../common/Image.h"


// Chris Stuetzle
#define NUM_BRIGHTNESS_BINS 20

extern int brightness_threshold_seed;

class IR_Data_Point;


class IR_Interpolable_Data {
 public:
  IR_Interpolable_Data() {
    for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
      brightness_histogram_[i] = -1;
    }
    valid_=false;
  }
  double getBrightness() const {
    double answer = 0;
    assert (valid());
    for (int i = 0; i < NUM_BRIGHTNESS_BINS; i++) {
      answer += brightness_histogram_[i];
    }
    return answer;
  }

  friend std::ostream& operator<<(std::ostream &ostr, const IR_Interpolable_Data &pt);
  friend std::istream& operator>>(std::istream &istr, IR_Interpolable_Data &pt);


  double CalculateDifference(const IR_Interpolable_Data &IRD) const;
  int numBins() const { return NUM_BRIGHTNESS_BINS; }
  double getHistogramIntensity(int b) const {
    assert (b >= 0 && b < NUM_BRIGHTNESS_BINS);
    assert (valid());
    return brightness_histogram_[b];
  }

  void ScaleWithAverage(const IR_Interpolable_Data &avg);


  bool valid() const { return valid_; }
  void set_valid(bool v) { valid_ = v; }

  // RERESENTATION
  double brightness_histogram_[NUM_BRIGHTNESS_BINS];
 private:
  bool valid_;
};


class Weighted_Average_IR_Interpolable_Data {
 public:
  Weighted_Average_IR_Interpolable_Data() {}
  void addSample(const IR_Interpolable_Data &sample, double weight) {
    info.push_back(std::make_pair(sample,weight));
  }
  IR_Interpolable_Data getAverage() const {
    IR_Interpolable_Data answer;
    if (info.size() == 0) return answer;
    assert (info.size() > 0);
    for (unsigned int bin = 0; bin < NUM_BRIGHTNESS_BINS; bin++) {
      answer.brightness_histogram_[bin] = 0;
    }
    double weight_sum = 0;

    for (unsigned int j = 0; j < info.size(); j++) {
      double weight = info[j].second;
      for (unsigned int bin = 0; bin < NUM_BRIGHTNESS_BINS; bin++) {
	answer.brightness_histogram_[bin] += weight * info[j].first.brightness_histogram_[bin];
      }
      weight_sum += weight;
    }
  
    assert (weight_sum > 0);
    
    for (unsigned int bin = 0; bin < NUM_BRIGHTNESS_BINS; bin++) {
      answer.brightness_histogram_[bin] /= weight_sum;
    }

    answer.set_valid(true);

    return answer;
  }

 private:

  std::vector<std::pair<IR_Interpolable_Data,double> > info;

};


class IR_Data_Point {
 public:

  IR_Data_Point();
  IR_Data_Point(unsigned long ip_, double x_, double y_,const Image<byte> &micro_image);

  void print(std::ostream &ostr);
  void write_led_image(const char* s) { led_image_.write(s);  }
  void write_false_color_led_image(const char* s);

  double x() const { return x_; }
  double y() const { return y_; }
  double ip() const { return ip_; }
  double brightness() const { return interpolable_data_.getBrightness(); }
  const IR_Interpolable_Data interpolable_data() const { return interpolable_data_; }

  friend std::ostream& operator<<(std::ostream &ostr, const IR_Data_Point &pt);
  friend std::istream& operator>>(std::istream &istr, IR_Data_Point &pt);

 private:

  // representation
  unsigned long ip_;
  double x_;
  double y_;

  IR_Interpolable_Data interpolable_data_;
  Image<byte> led_image_;

  // helper functions
  void calculateBrightness();
  void calculateHistogram();
};

inline bool operator<(const IR_Data_Point &a, const IR_Data_Point &b) {
  if (a.brightness() > b.brightness()) return true;
  return false;
}

#endif
