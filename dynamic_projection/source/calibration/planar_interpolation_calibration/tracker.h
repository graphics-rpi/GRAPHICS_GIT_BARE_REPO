#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <deque>
#include <list>
#include <sys/time.h>
#include "planar_calibration.h"
#include "../../ir_tracking/ir_data_point.h"

//#define LASER_TAIL_LENGTH 100
//#define TRACKING_PREDICTION 0.5

long timevaldiff(const struct timeval &starttime, const struct timeval &finishtime);


struct TrackerData {
  TrackerData(const Pt &pt_, const IntTri &intTri_, 
	      const std::vector<unsigned long> &ips_,
	      const std::vector<std::vector<IR_Interpolable_Data> > &interpolable_data_);

  
  Pt pt;
  IntTri intTri;
  std::vector<unsigned long> ips;
  std::vector<std::vector<IR_Interpolable_Data> > interpolable_data; 
};

// ================================================================================
// A consistent labeling of a tracked point

class Cursor;

class TrackedPoint {

 public:
  TrackedPoint(const TrackerData &td, timeval tv) {
    current_triangle = td.intTri;
    id = next_id;
    next_id++;
    last_active_tv = tv;

    intensity_trail.push_back(IntensityPt(td.pt,td.ips[0],td.interpolable_data[0][0])); 

    pt_trail.push_back(td.pt);
    laser_tail_length = default_laser_tail_length;
    last_delta_pos = td.pt;
    which_laser = -1;

    cursor = NULL;
    FORCED_LASER = -1;
  }

  ~TrackedPoint() {
    assert (cursor == NULL);
    assert (FORCED_LASER == -1);
  }

  Cursor* getCursor() const { return cursor; }
  int getForcedLaser() const { return FORCED_LASER; }
  void setCursor(Cursor *c, int F) { cursor = c; FORCED_LASER = F; }


  void update(const TrackerData &td, timeval tv) {
    current_triangle = td.intTri;

    intensity_trail.push_back(IntensityPt(td.pt,td.ips[0],td.interpolable_data[0][0])); 

    pt_trail.push_back(td.pt);
    if (laser_tail_length > 0) {
      while ((int)intensity_trail.size() > laser_tail_length) {
	intensity_trail.pop_front();
      }
      while ((int)pt_trail.size() > laser_tail_length) {
	pt_trail.pop_front();
      }
    } 
    last_active_tv = tv;
  }
 
  Pt getCurrentPosition() const { 
    assert (IsActive()); 

    if (pt_trail.size() == 1) {
      return pt_trail.back();
    } 

    std::deque<Pt>::const_reverse_iterator itr = pt_trail.rbegin();
    const Pt &pos = (*itr);
    itr++;
    const Pt &pos2 = (*itr);
    Pt diff = pos-pos2;
    Pt answer = pos + diff * tracking_prediction;
    return answer;
  }

  //void getIntensity(double &brightness, double &radius, unsigned long &ip) const;
  void getIntensity(IR_Interpolable_Data &interpolable_data, unsigned long &ip) const;

  Pt getDelta() {

    //Unsigned always returns true
	//assert (pt_trail.size() >= 0);
    if (pt_trail.size() == 1) {
      return Pt(0,0);
    }     
    Pt answer = last_delta_pos-pt_trail.back();
    last_delta_pos = pt_trail.back();
    return answer;
  }

  const IntTri& getCurrentTriangle() const { 
    assert (IsActive());
    return current_triangle; }
  const std::deque<Pt>& getPtTrail() const { 
    return pt_trail; }

  const std::list<IntensityPt>& getIntensityTrail() const { 
    //assert (IsActive());
    return intensity_trail; }

  int getWhichLaser() const { return which_laser; }
  void setWhichLaser(int laser) { 
    if (which_laser != -1) {
      //      
      if (which_laser != laser) {
	std::cout << "not...  settingWhichLaser " << which_laser << "->" << laser << std::endl;
	std::cout << "WANTED A CHANGE! " << which_laser << "->" << laser << std::endl;
      }
    } else {
      std::cout << "setWhichLaser " << which_laser << "->" << laser << std::endl;
      which_laser = laser; 
    }
  }


  int getID() const { return id; }
  bool IsActive() const;
  void setLaserTailLength(int l) {
    laser_tail_length = l;
  }

  void clear_history() {
    Pt a = pt_trail.back();
    IntensityPt b = intensity_trail.back();
    pt_trail.clear();
    pt_trail.push_back(a);
    intensity_trail.clear();
    intensity_trail.push_back(b);
  }

 private:
  // representation;
  IntTri current_triangle;

  std::list<IntensityPt> intensity_trail;

  std::deque<Pt> pt_trail;
  int id;
  timeval last_active_tv;

  Pt last_delta_pos;

  int which_laser;

  static int next_id;


  Cursor* cursor;
  int FORCED_LASER;

 public:
  int laser_tail_length;
  static int default_laser_tail_length;
  static double tracking_prediction;



};

// ================================================================================
// A structure that holds all the tracked points

class PointTracker {

 public:
  PointTracker(PlanarCalibration *data, 
	       //void (*add)(const TrackedPoint &pt), 
	       //void (*remove)(const TrackedPoint &pt),
	       void (*add)(TrackedPoint *pt), 
	       void (*remove)(TrackedPoint *pt),
	       int default_laser_tail_length_,
	       double tracking_prediction_,
	       bool differentiate_lasers_=true) { 
    calibration_data = data;
    add_func = add;
    remove_func = remove;
    TrackedPoint::default_laser_tail_length = default_laser_tail_length_;
    TrackedPoint::tracking_prediction = tracking_prediction_;
    differentiate_lasers = differentiate_lasers_;
  }

  void ProcessPointData(const std::vector<IR_Data_Point> &raw_points,
			const std::vector<Pt> &mouse_points = std::vector<Pt>());

  // ACCESSOR
  const TrackedPoint* operator[](int i) const { return tracked_point_ptrs[i]; }
  TrackedPoint* operator[](int i) { return tracked_point_ptrs[i]; }
  unsigned int size() const { return tracked_point_ptrs.size(); }
  static const timeval& getTimeval() { return last_tv; }
  static bool ReadIRPointData(std::ifstream &istr, std::vector<IR_Data_Point> &raw_points);
 
  int getFrame() const { return last_frame; }

 private:
  // PRIVATE HELPER FUNCTION
  void DetermineCorrespondences(const std::vector<TrackerData> &current);
  void NormalizeIntensityForVelocity();
  void assign_laser_ids();

  // REPRESENTATION
  PlanarCalibration *calibration_data;
  std::vector<TrackedPoint*> tracked_point_ptrs;
  static timeval last_tv;
  static int last_frame;

  void (*add_func)(TrackedPoint *pt);
  void (*remove_func)(TrackedPoint *pt);

  bool differentiate_lasers;

};


// ================================================================================


enum action_classification { EARLY_TRACKING, 
			     STATIONARY, 
			     HORIZONTAL_Z_STRIKE, VERTICAL_Z_STRIKE, Z_STRIKE, 
			     CLOCKWISE_CIRCLE, COUNTERCLOCKWISE_CIRCLE, 
			     OTHER_CLASSIFICATION };


action_classification Classify(const std::deque<Pt> &trail_list, double motion_tolerance,
			       double &answer_radius, Pt &answer_center);




#endif
