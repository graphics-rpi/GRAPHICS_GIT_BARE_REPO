#include <iostream>
#include <fstream>
#include <iomanip>

#include "tracker.h"
#include "munkres_matrix.h"
#include "munkres.h"

#include "../../ir_tracking/ir_data_point.h"

#include "../../../../remesher/src/vectors.h"
#include "../../../../remesher/src/utils.h"


timeval PointTracker::last_tv;
int TrackedPoint::next_id = 1;
int PointTracker::last_frame;
int TrackedPoint::default_laser_tail_length;
double TrackedPoint::tracking_prediction;


std::ofstream *tracking_logfile = NULL;


// ================================================================================

long timevaldiff(const struct timeval &starttime, const struct timeval &finishtime) {
  long msec;

  msec=(finishtime.tv_sec-starttime.tv_sec)*1000;
  msec+=(finishtime.tv_usec-starttime.tv_usec)/1000;

  return msec;
}

// ================================================================================





TrackerData::TrackerData(const Pt &pt_, const IntTri &intTri_, 
			 const std::vector<unsigned long> &ips_,
			 const std::vector<std::vector<IR_Interpolable_Data> > &interpolable_data_) {
  pt = pt_;
  intTri = intTri_;
  //assert (ips_.size() >= 0);
  assert (interpolable_data_.size() == ips_.size());
  for (unsigned int i = 0; i < ips_.size(); i++) {
    assert (interpolable_data_[i].size() == MAX_LASERS);
  }
  ips = ips_;
  interpolable_data = interpolable_data_;
}


bool TrackedPoint::IsActive() const { 
  long diff_msec = timevaldiff(last_active_tv,PointTracker::getTimeval());
  // if an IR is "off" for more a fifth of a second, it is deemed inactive
  if (diff_msec < 100) {
    return true;
  }
  return false;
}  


void TrackedPoint::getIntensity(IR_Interpolable_Data &interpolable_data, unsigned long &ip) const {
  ip = 0;

  Weighted_Average_IR_Interpolable_Data waid;

  ip = intensity_trail.back().ip;

  int count = 0;
  for (std::list<IntensityPt>::const_reverse_iterator itr = intensity_trail.rbegin();
       itr != intensity_trail.rend() && count < 10;
       itr++) {
    const IntensityPt &p = *itr;
    if (ip != p.ip) continue;
    waid.addSample(p.interpolable_data,1.0);
    count++;
  }
  interpolable_data = waid.getAverage();
}



// ================================================================================
// READ THE RAW IR CAMERA PIXEL COORDINATES OF EACH POINT

bool PointTracker::ReadIRPointData(std::ifstream &istr, std::vector<IR_Data_Point> &raw_points) {
  //double x,y,brightness;
  std::string token;
  unsigned int sec, usec;
  unsigned int point_count;
  unsigned int this_frame;
  
  static bool printed_error_message = false;


  timeval this_tv;
  
  // read the file header
  istr >> token >> this_frame;
  if (token == "") {
    if (!printed_error_message) {
      printed_error_message = true;
      std::cout << "ERROR READING IR FILE" << std::endl;
    }
    return false;
  }
  assert (token == "FRAME");
  istr >> token >> sec >> usec;
  assert (token == "TIME");
  this_tv.tv_sec = sec;
  this_tv.tv_usec = usec;
  if (this_tv.tv_sec == last_tv.tv_sec && this_tv.tv_usec == last_tv.tv_usec) {
    // we've already read this file
    return false;
  }
  //if (this_frame - last_frame != 1) {
  if (this_frame - last_frame > 2) {
    std::cout << "WARNING: FRAME SKIP " << last_frame << "->" << this_frame << " " << this_frame-last_frame << std::endl;
  }

  last_frame = this_frame;
  last_tv = this_tv;

  istr >> token >> point_count;
  assert (token == "NUM_POINTS");
  
  // read the raw point data (still in camera space)
  raw_points.clear();

  IR_Data_Point pt;
  for (unsigned int i = 0; i < point_count; i++) { 
    istr >> pt;
    raw_points.push_back(pt); //IR_Data_Pointstd::make_pair(Pt(x,y),brightness));
  } 
  assert (raw_points.size() == point_count);
  return true;
}

// ================================================================================
// MAP EACH CAMERA PIXEL COORDINATE TO PROJECTOR PIXEL COORDINATES

std::vector<unsigned long> dummy_ips; //double all_neg[MAX_LASERS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
std::vector<std::vector<IR_Interpolable_Data> > dummy_vals; //double all_neg[MAX_LASERS] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

void PointTracker::ProcessPointData(const std::vector<IR_Data_Point> &raw_points, const std::vector<Pt> &mouse_points) {

  std::vector<TrackerData> keep_td;
  std::vector<TrackerData> reject_td;

  for (unsigned int i = 0; i < raw_points.size(); i++) {
    IntTri answer_triangle;
    Pt answer_point;
    std::vector<unsigned long> ips_out;
    std::vector<std::vector<IR_Interpolable_Data> > interpolable_data_out;
    calibration_data->BarycentricInterpolation(raw_points[i].x(),
					       raw_points[i].y(),
					       raw_points[i].ip(),
					       raw_points[i].interpolable_data(),
					       answer_triangle,answer_point,
					       ips_out,
					       interpolable_data_out
					       );

    bool cross = calibration_data->OnScreen(answer_point,raw_points[i].ip());

    TrackerData my_td(answer_point,answer_triangle,ips_out,interpolable_data_out);

    if (cross) { 
      //std::cout << "keep  " << answer_point << std::endl; 
      keep_td.push_back(my_td); 
    }
    else {
      //std::cout << "REJECT " << answer_point<< std::endl; 
      reject_td.push_back(my_td);
    }
  } 

  std::vector<TrackerData> current = keep_td;

  for (unsigned int i = 0; i < reject_td.size(); i++) {
    bool keep = true;
    for (unsigned int j = 0; j < keep_td.size(); j++) {
      double dist = DistanceBetweenTwoPoints(reject_td[i].pt,keep_td[j].pt);
      std::cout << "rejecting " << dist << std::endl;
      keep = false;
    }
    if (keep) {
      current.push_back(reject_td[i]);
    }
  }

  for (unsigned int i = 0; i < mouse_points.size(); i++) {
    IntTri answer_triangle;
    current.push_back(TrackerData(mouse_points[i],answer_triangle,dummy_ips,dummy_vals)); //,dummy_vals)); //all_neg,all_neg));
  } 

  DetermineCorrespondences(current);

  //NormalizeIntensityForVelocity();

  assign_laser_ids();

}

// ================================================================================
// CORRESPOND EACH IR POINT TO A ELEMENT TRACKED OVER TIME

void PointTracker::DetermineCorrespondences(const std::vector<TrackerData> &current) {
  int num_current = current.size();
  int num_prev = tracked_point_ptrs.size();

  // Prepare the matrix for the Hungarian algorithm
  MRMatrix<double> mat(num_current,num_prev);
  //MRMatrix<double> before_mat(num_current,num_prev);
  for (int i = 0; i < num_current; i++) {
    Pt cur = current[i].pt;
    for (int j = 0; j < num_prev; j++) {
      //std::cout << "i " << i << "   j " << j << std::endl;
      //std::cout << "Cur" << cur.x << " " << cur.y << std::endl;
      if (tracked_point_ptrs[j]->IsActive()) {	
	Pt prev = tracked_point_ptrs[j]->getCurrentPosition();
	//std::cout << "prev" << prev.x << " " << prev.y << std::endl;
	double dist = DistanceBetweenTwoPoints(cur,prev);
	mat(i,j) = dist;
	//before_mat(i,j) = dist;
	//std::cout << "dist " << i << " " << j << " " << dist << std::endl;
      } else {
	mat(i,j) = 1000000;  // infinity
	//before_mat(i,j) = 1000000;  // infinity
	//std::cout << "dist " << i << " " << j << " (infinity)" << std::endl;
	//exit(0);
      }
    }
  }
  MRMatrix<double> before_mat(mat);

  // solve the matrix
  Munkres mr;
  mr.solve(mat);

  // use the results to assign points and update the elements
  for (int i = 0; i < num_current; i++) {
    bool assigned = false;
    for (int j = 0; j < num_prev; j++) {
      if (mat(i,j) == 0) {
	//if (before_mat(i,j) < 150) {
	if (before_mat(i,j) < 1000) {   /// BARB: made much bigger for 4K projector
	  assert (assigned == false);
	  assigned = true;
	  tracked_point_ptrs[j]->update(current[i],last_tv); //.pt,current[i].intTri,last_tv, 
	  //);
	  //tracked_points[j].setWhichLaser(current[i].which_laser);
	  //tracked_points[j].setWhichLaser(-1); //current[i].which_laser);
	} else {
	  //std::cout << "GAP TOO BIG" << before_mat(i,j) << std::endl;
	}
      }
    }

    // add new elements when necessary
    if (assigned == false) {      
      //tracked_points.push_back(TrackedPoint(current[i].pt,current[i].intTri,last_tv));
      tracked_point_ptrs.push_back(new TrackedPoint(current[i],last_tv));
      TrackedPoint *pt = tracked_point_ptrs.back();
      //pt.setWhichLaser(-1); //current[i].which_laser);
      add_func(pt);
    }
  }
  
  // deactivate elements that are no longer needed
  std::vector<TrackedPoint*>::iterator itr = tracked_point_ptrs.begin();
  while (itr != tracked_point_ptrs.end()) {
    if ((*itr)->IsActive()) {
      itr++;
    } else {
      TrackedPoint *pt = *itr;
      remove_func(pt);
      delete pt;
      itr = tracked_point_ptrs.erase(itr);
    }
  }

  //assign_laser_ids();

}


static bool sort_by_second2(const std::pair<int,double> &a,const std::pair<int,double> &b) {
  return a.second < b.second;
}


//#define FORCE_UNIQUE_LASERS 1
#define FORCE_UNIQUE_LASERS 0

void PointTracker::assign_laser_ids() {
  int num_points = tracked_point_ptrs.size();

#if FORCE_UNIQUE_LASERS

  // USE THE HUNGARIAN METHOD
  // (only one tracked point per laser!)

  if (num_points > MAX_LASERS) return;

  // Prepare the matrix for the Hungarian algorithm
  MRMatrix<double> mat(num_points,MAX_LASERS);

  std::vector<std::vector<double> > intensity_differences(1,std::vector<double>(MAX_LASERS,-1));
  for (int i = 0; i < num_points; i++) {
    const TrackedPoint &pt = tracked_point_ptrs[i];    
    
    IR_Interpolable_Data interpolable_data;
    unsigned long ip;
    pt.getIntensity(interpolable_data,ip); 

    calibration_data->MatchToLaserProfiles(interpolable_data,ip,intensity_differences,false);

    for (int j = 0; j < MAX_LASERS; j++) {
      if (intensity_differences[0][j] > 0) {
	mat(i,j) = intensity_differences[0][j];
      } else {
	mat(i,j) = 1000000; // something big
      }
    }
  }

#if 0
  std::cout << "-------------------" << std::endl;
  for (int i = 0; i < num_points; i++) {
    for (int j = 0; j < MAX_LASERS; j++) {
      std::cout << mat(i,j) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
#endif

  MRMatrix<double> before_mat(mat);
  Munkres mr;
  mr.solve(mat);

#if 0
  for (int i = 0; i < num_points; i++) {
    for (int j = 0; j < MAX_LASERS; j++) {
      std::cout << mat(i,j) << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
#endif
  for (int i = 0; i < num_points; i++) {
    bool assigned = false;
    if (!differentiate_lasers) {
      assigned=true;
      tracked_point_ptrs[i]->setWhichLaser(0);
    } else {
      for (int j = 0; j < MAX_LASERS; j++) {
	if (mat(i,j) == 0) {
	  if (before_mat(i,j) < 1000) {
	    assert (assigned == false);
	    assigned = true;
	    //std::cout << "SETTING" << j << std::endl;
	    tracked_point_ptrs[i]->setWhichLaser(j);
	  }
	}
      }
    }
    if (assigned == false) {
      //std::cout << "UNASSIGNED LASER" << std::endl;
      tracked_point_ptrs[i]->setWhichLaser(-1);
    }
  }


  if (tracking_logfile != NULL) {
    for (int i = 0; i < num_points; i++) {
      const TrackedPoint *pt = tracked_point_ptrs[i];    
      (*tracking_logfile) << "Frame " << std::setw(10) << last_frame 
			  << "    pt_id " << std::setw(5) << pt->getID();
      (*tracking_logfile) << "       " << std::setw(3) << pt->getWhichLaser()+1 << "  " 
			  <<              std::fixed << std::setprecision(5) << std::setw(10) << 1;
      for (unsigned int i = 1; i < 6; i++) {  // HACK FOR CHRIS FOR NOW
	(*tracking_logfile) << "       " << std::setw(3) << -1 << "  " 
			    <<              std::fixed << std::setprecision(5) << std::setw(10) << -1;
      }
      (*tracking_logfile) << std::endl;
    }
  }


#else

  // JUST BE GREEDY, MATCH TO BEST LASER, DUPLICATES OK

  std::vector<std::vector<double> > intensity_differences(1,std::vector<double>(MAX_LASERS,-1));
  for (int i = 0; i < num_points; i++) {
    const TrackedPoint *pt = tracked_point_ptrs[i];    
    //double b,r;
    unsigned long ip;
    IR_Interpolable_Data interpolable_data;
    pt->getIntensity(interpolable_data,ip);
    calibration_data->MatchToLaserProfiles(interpolable_data,ip,intensity_differences,false);
    
    std::vector<std::pair<int,double> > stats;

    double best = -1;

    if (!differentiate_lasers) {
      //      assigned=true;
      tracked_point_ptrs[i]->setWhichLaser(0);
    } else {

      for (int j = 0; j < MAX_LASERS; j++) {
	
	if (intensity_differences[0][j] > 0) {
	  stats.push_back(std::make_pair(j,intensity_differences[0][j]));
	}
	
	if (intensity_differences[0][j] > 0 && 
	    (best < 0 || intensity_differences[0][j] < best)) {
	  best = intensity_differences[0][j];
	  tracked_point_ptrs[i]->setWhichLaser(j);
	}
	
      }
    }
    
    std::sort(stats.begin(),stats.end(),sort_by_second2);
    
    //  int PointTracker::last_frame;
    if (tracking_logfile != NULL) {
      const TrackedPoint *pt = tracked_point_ptrs[i];    
      (*tracking_logfile) << "Frame " << std::setw(10) << last_frame 
			  << "    pt_id " << std::setw(5) << pt->getID();
      for (unsigned int i = 0; i < stats.size(); i++) {
	(*tracking_logfile) << "       " << std::setw(3) << stats[i].first+1 << "  " 
			    <<              std::fixed << std::setprecision(5) << std::setw(10) << stats[i].second;
      }
      (*tracking_logfile) << std::endl;
      
    }
    
 
  }

  
#endif
  
  /*
  //  int PointTracker::last_frame;
  if (tracking_logfile != NULL) {
    for (int i = 0; i < num_points; i++) {
      const TrackedPoint &pt = tracked_points[i];    
      (*tracking_logfile) << "frame " << std::setw(10) << last_frame 
			  << "    pt_id " << std::setw(5) << pt.getID() 
			  << "    laser " << std::setw(3) << pt.getWhichLaser() 
			  << std::endl;
    }
    
  }
  */
}

// ===========================================================
// ===========================================================
// ===========================================================

static double compute_sum(const std::vector<double> &v) {
  assert (v.size() > 0);
  double answer = 0;
  for (unsigned int i = 0; i < v.size(); i++) {
    answer += v[i];
  }
  return answer;
}

static double compute_average(const std::vector<double> &v) {
  assert (v.size() > 0);
  double answer = 0;
  for (unsigned int i = 0; i < v.size(); i++) {
    answer += v[i];
  }
  return answer / double(v.size());
}

static double compute_stddev(const std::vector<double> &v, double avg) {
  assert (v.size() > 1);
  double answer = 0;
  for (unsigned int i = 0; i < v.size(); i++) {
    answer += (avg-v[i])*(avg-v[i]);
  }
  answer /= double(v.size()-1);
  answer = sqrt(answer);
  return answer;
}

static Pt compute_average(const std::vector<Pt> &v) {
  assert (v.size() > 0);
  Pt answer(0,0);
  for (unsigned int i = 0; i < v.size(); i++) {
    answer += v[i];
  }
  return answer / double(v.size());
}

/*
static Pt compute_stddev(const std::vector<Pt> &v, const Pt &avg) {
  assert (v.size() > 0);
  Pt answer(0,0);
  for (unsigned int i = 0; i < v.size(); i++) {
    Pt diff = avg-v[i];
    answer += Pt(diff.x*diff.x,diff.y*diff.y);
  }
  answer /= double(v.size()-1);
  return  Pt(sqrt(answer.x),sqrt(answer.y));
}
*/

// ===========================================================
// ===========================================================
// ===========================================================

static void best_circle(const Pt &in_a, const Pt &in_b, const Pt &in_c, double &answer_radius, Pt &answer_center, double &angle) {
  Vec3f a(in_a.x,in_a.y,0);
  Vec3f b(in_b.x,in_b.y,0);
  Vec3f c(in_c.x,in_c.y,0);
  Vec3f mid_ab = (a+b) * 0.5;
  Vec3f mid_bc = (b+c) * 0.5;
  Vec3f perp = Vec3f(0,0,1);
  Vec3f vab = b-a; vab.Normalize();
  Vec3f vbc = c-b; vbc.Normalize();
  Vec3f dir_ab,dir_bc;
  Vec3f::Cross3(dir_ab,perp,vab); dir_ab.Normalize();
  Vec3f::Cross3(dir_bc,perp,vbc); dir_bc.Normalize();
  angle = SignedAngleBetweenNormalized(dir_ab,dir_bc,perp);
  Vec3f intersection;
  Intersect(mid_ab,dir_ab,mid_bc,dir_bc,intersection);
  float da = DistanceBetweenTwoPoints(a,intersection);
  float db = DistanceBetweenTwoPoints(b,intersection);
  float dc = DistanceBetweenTwoPoints(c,intersection);
  answer_center = Pt(intersection.x(),intersection.y());
  answer_radius = (da + db + dc) * 1 / 3.0;
}


action_classification Classify(const std::deque<Pt> &trail_list, double motion_tolerance,
			       double &answer_radius, Pt &answer_center) {
  
  if (trail_list.size() < 10) {
    //std::cout << "EARLY_TRACKING" << std::endl;
    return EARLY_TRACKING;
  }
  
  // ========================================================
  // CHECK FOR STATIONARY POINT
  
  // first, put stuff in a vector (yes, inefficient)
  std::vector<Pt> trail(trail_list.size());
  int index = 0;
  for (std::deque<Pt>::const_reverse_iterator itr = trail_list.rbegin();
       itr != trail_list.rend();
       index++,itr++) {
    const Pt &p = *itr;
    trail[index] = p;
  }
  
  // compute the average position for the last n frames
  int n = 10;
  Pt avg_pos(0,0);
  for (int i = 0; i < n; i++) {
    avg_pos += trail[i];
  }
  avg_pos /= double(n);

  // compute the standard deviation of the position for the last n frames
  answer_radius = 0;
  Pt std_dev(0,0);
  for (int i = 0; i < n; i++) {
    Pt tmp = trail[i] - avg_pos;
    answer_radius += tmp.Length();
    std_dev += Pt(tmp.x*tmp.x,tmp.y*tmp.y);
  }
  std_dev = Pt(sqrt(std_dev.x/double(n-1)),sqrt(std_dev.y/double(n-1)));
  double std_dev_len = sqrt(std_dev.x*std_dev.x+std_dev.y*std_dev.y);
  answer_radius /= double(n);

  // if the length of the standard deviation is small
  if (std_dev_len < motion_tolerance) {
    //std::cout << "STATIONARY" << std::endl;
    answer_center = avg_pos;
    //answer_radius = answer_radius;   // NOT SURE WHAT THIS WAS DOING?   A BUG?
    return STATIONARY;
  }


  // compute the average position for ALL frames
  answer_center = compute_average(trail);
  
  // compute the average radius for ALL frames
  std::vector<double> dist(trail.size());
  for (unsigned int i = 0; i < trail.size(); i++) {
    Pt tmp = trail[i]-answer_center;
    dist[i] = tmp.Length(); 
  }
  answer_radius = compute_average(dist);


  // ========================================================
  // CHECK FOR CIRCLE

  // considering 3 points at a time, compute the best fit circle
  std::vector<double> radii(trail.size()-2);
  std::vector<Pt> centers(trail.size()-2);
  std::vector<double> angles(trail.size()-2);
  for (unsigned int i = 0; i < trail.size()-2; i++) {
    best_circle(trail[i],trail[i+1],trail[i+2],radii[i],centers[i],angles[i]);
  }
  double avg_radius = compute_average(radii);
  //double stddev_radius = compute_stddev(radii,avg_radius);
 
  double avg_angle = compute_average(angles);
  double stddev_angle = compute_stddev(angles,avg_angle);
  double angle_sum = compute_sum(angles);

  //std::cout << "avgradius:  " << avg_radius << "   stddevradius: " << stddev_radius << answer_center.x << " " << answer_center.y << std::endl;
  //std::cout << "angle sum: " << angle_sum << " avg_angle: " << avg_angle << " avgradius: " << avg_radius << " stddevangle:" << stddev_angle << std::endl;
  
  if (fabs(angle_sum) < 0.75* 2 * M_PI) { /* angle sum almost 2 PI */
    //std::cout << "circle not complete yet" << std::endl;
  } else if (fabs(avg_radius) > 600) {
    //  } else if (fabs(avg_radius > 300)) {
    //std::cout << "circle too big" << std::endl;
  } else if (fabs(avg_angle) < 0.10) {  // 0.15   /* circle too eratie */
    //std::cout << "circle too erratic" << std::endl;
  } else if (fabs(stddev_angle) > 0.5) {    /* circle too eratie */
    //std::cout << "circle too erratic2" << std::endl;
  } else {
    answer_radius = 0.5*(answer_radius+avg_radius);    
    answer_center = 0.5*(answer_center+compute_average(centers));
    if (avg_angle > 0) {
      //std::cout << "CLOCKWISE CIRCLE" << std::endl;
      return CLOCKWISE_CIRCLE;
    } else {
      //std::cout << "COUNTERCLOCKWISE CIRCLE" << std::endl;
      return COUNTERCLOCKWISE_CIRCLE;
    }
  }

  // ========================================================
  // CHECK FOR STRIKE

  // calculate the velocities
  std::vector<Pt> velocities(trail.size()-1);
  for (unsigned int i = 0; i < trail.size()-1; i++) {
    velocities[i] = trail[i+1]-trail[i];
  }

  // calculate the accelerations
  std::vector<Pt> accelerations(velocities.size()-1);
  for (unsigned int i = 0; i < velocities.size()-1; i++) {
    accelerations[i] = velocities[i+1]-velocities[i];
  }

  // collect the points of maximum acceleration
  //std::cout << "max accelerations: ";
  std::vector<int> max_accelerations;
  for (unsigned int i = 1; i < accelerations.size()-1; i++) {
    if (accelerations[i-1].Length() < accelerations[i].Length() &&
	accelerations[i+1].Length() < accelerations[i].Length()) {	
      max_accelerations.push_back(i);
    }
  }
  max_accelerations.push_back(accelerations.size());
  
  std::vector<Pt> interval_velocities(max_accelerations.size());
  int last = 0;
  for (unsigned int i = 0; i < max_accelerations.size(); i++) {
    Pt sum(0,0);
    int sum_count = 0;
    for (int j = last; j <= max_accelerations[i]; j++) {
      //std::cout << " " << j;
      sum += velocities[j];
      sum_count++;
    }
    sum /= double(sum_count);
    interval_velocities[i] = sum;
    last = max_accelerations[i] + 1;
    //std::cout << "|";
  }
  //std::cout << std::endl;

  /*
  for (unsigned int i = 0; i < interval_velocities.size(); i++) {
    std::cout << i << " " << interval_velocities[i].x << " " << interval_velocities[i].y << std::endl;
  }
  */

  int num_zigs = 0;

  int flag = false;
  Pt avg_vel(0,0);
  int avg_vel_count = 0;

  for (unsigned int i = 0; i < interval_velocities.size()-1; i++) {
    Pt a = interval_velocities[i];
    Pt b = interval_velocities[i+1];
    a.Normalize();
    b.Normalize();
    double dot = a.Dot(b);
    if (dot < -0.8) {
      num_zigs++;
      flag = !flag;
    }
    avg_vel_count += 2;
    if (flag) {
      avg_vel += interval_velocities[i];
      avg_vel -= interval_velocities[i+1];
    } else {
      avg_vel -= interval_velocities[i];
      avg_vel += interval_velocities[i+1];
    }
    //std::cout << dot << " ";
  }
  //std::cout << std::endl;

  answer_radius = 50;

  if (num_zigs >= 4) {
    avg_vel /= double(avg_vel_count);
    if (fabs(avg_vel.x) > 2*fabs(avg_vel.y)) {
      //std::cout << "HORIZONTAL Z STRIKE" << std::endl;
      return HORIZONTAL_Z_STRIKE;
    } else if (fabs(avg_vel.y) > 2*fabs(avg_vel.x)) {
      //std::cout << "VERTICAL Z STRIKE" << std::endl;
      return VERTICAL_Z_STRIKE;
    } else {
      //std::cout << "Z STRIKE" << std::endl;
      return Z_STRIKE;
    }
  }

  //std::cout << "OTHER CLASSIFICATION" << std::endl;
  return OTHER_CLASSIFICATION;
}


