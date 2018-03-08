#ifndef __PLANAR_CALIBRATION_H__
#define __PLANAR_CALIBRATION_H__

#include <cmath>
#include <cassert>
#include <vector>
#include <string>
#include <iostream>

#include <glm/glm.hpp>

#include "../../../../remesher/src/vectors.h"
#include "../../ir_tracking/ir_data_point.h"

#define MAX_LASERS 16

/*
extern const char* laser_names[MAX_LASERS];
extern const char* index_names[MAX_LASERS];
extern const Vec3f fixed_laser_colors[MAX_LASERS];
*/



#define MAX_INTENSITY_SAMPLES 100

// ================================================================================
// A simple class to store 2D integer coordinates
class IntPt {
public:
	IntPt(int _i=-1, int _j=-1) : i(_i), j(_j) {}
	int i,j;
};



inline bool sort_by_second(const std::pair<IntPt,double> &a, const std::pair<IntPt,double> &b) {
	return a.second < b.second;
}


// ================================================================================
// A simple class to store a triangle with integer coordinates
class IntTri {
public:
	IntPt pts[3];
};


// ================================================================================
// A simple class to store 2D points in double precision

class Pt {
public:
	Pt(double _x=-1, double _y=-1) : x(_x), y(_y) {}
	double x;
	double y;
	Pt operator-(const Pt &pt) const { return Pt(x-pt.x,y-pt.y); }
	Pt operator+(const Pt &pt) const { return Pt(x+pt.x,y+pt.y); }
	Pt operator*(double d) const { return Pt(d*x,d*y); }
	Pt operator/(double d) const { return Pt(x/d,y/d); }
	void operator+=(const Pt &pt) { *this = *this+pt;  }
	void operator-=(const Pt &pt) { *this = *this-pt;  }
	void operator/=(double d) { *this = *this/d; }
	bool operator==(const Pt &pt)
	{
		if(abs(x - pt.x) < 0.0001 && abs(y - pt.y) < 0.0001)
			return true;
		else
			return false;
	}
	double Length() { return sqrt(x*x + y*y); }
	void Normalize() { *this /= Length(); }
	double Dot(const Pt &pt) const { return x*pt.x + y*pt.y; }

	friend std::ostream& operator<<(std::ostream &ostr, const Pt &p){
	  ostr << "PT < " << p.x << " " << p.y << "> "; 
	  fflush(stdout);
	  return ostr;
	}


};


inline Pt operator*(double d,const Pt &pt) { return Pt(d*pt.x,d*pt.y); }


class Screen {
 public:
  Pt bottomleft;
  Pt topright;
  unsigned long ip;

  bool OnScreen(const Pt &p) {
    /*
    std::cout << "bottom " << bottomleft.y << std::endl;
    std::cout << "top " << topright.y << std::endl;
    std::cout << "left " << bottomleft.x << std::endl;
    std::cout << "right " << topright.x << std::endl;
    assert (bottomleft.y < topright.y);
    assert (bottomleft.x < topright.x);
    */
    if (p.x >= bottomleft.x && p.y >= bottomleft.y && 
	p.x < topright.x && p.y < topright.y) return true;
    return false;
  }

};



/*class Ellipse {
public:
	Ellipse() {
		centroid = Pt(-1,-1);
		major_radius = 0;
		minor_radius = 0;
	}
	Ellipse(const Pt &centroid_, const Pt &axis_, double major, double minor) {
		centroid = centroid_;
		if (major > minor) {
			axis = axis_;
			major_radius = 1.1*major;
			minor_radius = 1.1*minor;
		} else {
			axis = Pt(axis_.y,-axis_.x);
			major_radius = 1.1*minor;
			minor_radius = 1.1*major;
		}
		double tmp = sqrt(major_radius*major_radius - minor_radius*minor_radius);
		foci1 = centroid + tmp*axis;
		foci2 = centroid - tmp*axis;
	}
	const Pt& getCentroid() const { return centroid; }
	const Pt& getAxis() const { return axis; }
	double getMajorRadius() const { return major_radius; }
	double getMinorRadius() const { return minor_radius; }
	const Pt& getFoci1() const { return foci1; }
	const Pt& getFoci2() const { return foci2; }
private:
	Pt centroid;
	Pt axis;
	double major_radius;
	double minor_radius;
	Pt foci1;
	Pt foci2;
};
*/

class IntensityPt {
public:
  IntensityPt(const Pt &p,unsigned long i,const IR_Interpolable_Data &i_d) 
    : pt(p),ip(i),interpolable_data(i_d) {} 
  Pt pt;
  unsigned long ip;
  IR_Interpolable_Data interpolable_data;
};

struct IntensityData {
public:
  IntensityData() {
    //std::cout <<"init int data"<< std::endl;
    num_points = 0; next = 0; valid_avg = false; } 
  void addDataPoint(const IR_Interpolable_Data &interpolable_data) {
    valid_avg = false;
    if (num_points < MAX_INTENSITY_SAMPLES) {
      assert (next >= 0 && next < MAX_INTENSITY_SAMPLES);
      assert (next == num_points);
      samples[next] = interpolable_data; 
      num_points++;
      next++;
      if (next==MAX_INTENSITY_SAMPLES) next = 0;
    } else {
      assert (next >= 0 && next < MAX_INTENSITY_SAMPLES);
      samples[next] = interpolable_data; 
      next++;
      if (next==MAX_INTENSITY_SAMPLES) next = 0;
    }
  }
  friend std::ostream& operator<<(std::ostream &ostr, const IntensityData &data);
  friend std::istream& operator>>(std::istream &istr, IntensityData &data);
  
  int NumPoints() const { return num_points; }
  void getData(int i, IR_Interpolable_Data &interpolable_data) const {
    assert(i >= 0 && i < num_points);
    interpolable_data = samples[i]; } 

  const IR_Interpolable_Data& get_average() const {
    //if (num_points == 0) return -1;
    if (!valid_avg) compute_avg();
    assert (valid_avg);    
    return average;
  }
  
  bool valid() const {
    if (!valid_avg) {
      //      assert (num_points == 0);
    }
    return valid_avg;
  }


  // ADDED BY CHRIS STUETZLE
  IR_Interpolable_Data* getSample( int i ) { return &samples[ i ]; }

  void compute_avg() const {
    assert (!valid_avg);
    if (num_points == 0) return;
    assert (num_points > 0);
    // const cast hack to allow caching of the averages!
    IntensityData *t = (IntensityData*)this;

    Weighted_Average_IR_Interpolable_Data waid;
    
    for (int i = 0; i < num_points; i++) {
      waid.addSample(samples[i],1.0);
    }
    t->average = waid.getAverage();
    t->valid_avg = true;
  }

private:
  
  
  int num_points;
  int next;

  IR_Interpolable_Data samples[MAX_INTENSITY_SAMPLES]; //interpolable_data
  IR_Interpolable_Data average; //[MAX_INTENSITY_SAMPLES]; //interpolable_data

  bool valid_avg;
};

// ==========================================================================================
// ==========================================================================================


//enum on_screen_enum { on_screen_keep, on_screen_reject, on_screen_steal, on_screen_ignore };

class PlanarCalibration {

public:

static std::vector<std::string> laser_names;
static std::vector<std::string> index_names;
static std::vector<std::string> colors;
static std::vector<Vec3f> fixed_laser_colors;


	// CONSTRUCTORS
	// construct an empty matrix to store the calibration grid data
	PlanarCalibration(int w, int h, int border_x, int border_y, int spacing_x, int spacing_y);
	// read the data from a file, the fatal if set to true quits the program
	//if the file is not readable, setting it to false will have the function
	//throw an exception
	PlanarCalibration(const std::string &geometry_filename, const std::string &intensity_filename, bool fatal = true)throw(int);


	void initialize();

	// dimensions of grid
	int getGridX() const { return grid_x; }
	int getGridY() const { return grid_y; }

	void index_to_grid(int index, int &i, int &j) const {
		assert (index >= 0 && index < grid_x*grid_y);
		i = index % grid_x;
		j = index / grid_x;
	}
	int getIndex(int i, int j) const {
	  //std::cout << "getIndex " << i << " " << j << std::endl;
		assert (i >= 0 && i < grid_x);
		assert (j >= 0 && j < grid_y);
		return j*grid_x + i;
	}

	int getIpIndex(unsigned long ip) const {

	  assert (ip != 0);


	  for (unsigned int i = 0; i < all_my_data.size(); i++) {
	    //std::cout << " ip check " << i << "  " << all_my_data[i].ip << " " << ip << std::endl;
	    if (all_my_data[i].ip == ip) return i;
	  }
	  //	  std::cout << "IP INDEXall my data num " << all_my_data.size() << std::endl;

	  std::cout << "ADD NEW IP " << ip << " total cameras: " << all_my_data.size() << std::endl;

	  int answer = all_my_data.size();

	  // hack cast to get around const
	  PlanarCalibration *tmp = (PlanarCalibration*)(this);
	  tmp->all_my_data.push_back(CAMERA_DATA());
	  tmp->all_my_data[answer].ip = ip;
	  tmp->all_my_data[answer].geometry_valid = std::vector<bool>(grid_x*grid_y,false);
	  tmp->all_my_data[answer].geometry_data = std::vector<Pt>(grid_x*grid_y,Pt(-1,-1));
	  tmp->clear_intensities(); 

	  return answer;
	}

	bool OnScreen(const Pt &p, unsigned long ip) {
	  bool answer = true;
	  for (unsigned int i=0; i < screens.size(); i++) {
	    if (screens[i].ip == ip) {
	      if (screens[i].OnScreen(p)) {
		//std::cout << "KEEP" << std::endl;
		answer = true;
	      } else {
		std::cout << "OTHER SCREEN, IGNORE " << std::endl;
		answer = false;
	      }
	    } else {
	      if (screens[i].OnScreen(p)) {
		std::cout << "STEAL!" << std::endl;
	      } else {
		//std::cout << "IGNORE" << std::endl;
	      }
	    }
	  }
	  return answer;
	}

	void setGeometryData(int index, const Pt &p, unsigned long ip) {
	  int ip_index = getIpIndex(ip);
	  assert (index >= 0 && index < grid_x*grid_y);
	  
	  assert ((int)all_my_data[ip_index].geometry_data.size() > index);
	  assert ((int)all_my_data[ip_index].geometry_valid.size() > index);

	  all_my_data[ip_index].geometry_data[index] = p;
	  all_my_data[ip_index].geometry_valid[index] = true;
	}


	IR_Interpolable_Data NormalizeIntensityData(int index, int laser, const IR_Interpolable_Data &in) const;

	void addIntensityData(int index, int which_laser, unsigned long ip,
			      const IR_Interpolable_Data &interp_data) {
	  int ip_index = getIpIndex(ip);
	  assert (index >= 0 && index < grid_x*grid_y);
	  assert (which_laser >= 0 && which_laser < MAX_LASERS);
	  all_my_data[ip_index].intensity_data[index][which_laser].addDataPoint(interp_data); 

	  //assert (all_my_data[ip_index].intensity_data[index][which_laser].valid()); //addDataPoint(interp_data); 
	}

	void write_geometry_to_file(const std::string &geometry_filename);
	void write_intensity_to_file(const std::string &intensity_filename);
  // Three functions for writing out GNU Plot data
  void writeOutGNUPlotFile_AllLasersAllCalibrationSpots( std::vector< std::vector< std::vector< std::string > > >& filenames, const std::string &GNUPlot_filename_prefix );
  void writeOutGNUPlotFile_DiffLasers( int spot, std::vector< std::vector< std::vector< std::string > > >& filenames, const std::string &GNUPlot_filename_prefix );
  void writeOutGNUPlotFile_DiffSpots( int laser, std::vector< std::vector< std::vector< std::string > > >& filenames, const std::string &GNUPlot_filename_prefix );
  void writeOutGNUPlotDataFiles( const std::string &GNUPlot_filename_prefix );

	void LookForScreens();

	void read_geometry_data(const std::string &geometry_filename, bool fatal = true) throw(int);
	bool read_intensity_data(const std::string &intensity_filename);
	void compute_average_intensity();

	Pt calibration_point(const IntPt &pt) const {
		return Pt(grid_border_x + pt.i * grid_spacing_x,
				grid_border_y + pt.j * grid_spacing_y);
	}

	void BarycentricInterpolation(double x, double y, unsigned long ip,
				      const IR_Interpolable_Data &interpolable_data_in,
				      IntTri &answer_triangle, Pt &answer_point,
				      std::vector<unsigned long> &ips_out,
				      std::vector<std::vector<IR_Interpolable_Data> >  &interpolable_data_out);

	int ClosestGridPoint(double x, double y, unsigned long ip);

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	void clear_intensities();

	void clear_laser_intensity(int i);

	void MatchToLaserProfiles(const IR_Interpolable_Data &interpolable_data, unsigned long ip,
				  std::vector<std::vector<double> > &intensity_differences,
				  bool print_lots) const;

	/*
	const Ellipse& GetLaserEllipse(int laser, unsigned long ip) const {
	  int ip_index = getIpIndex(ip);
	  assert (laser >= 0 && laser < MAX_LASERS);
	  return all_my_data[ip_index].ellipses[laser];
	}
	*/

private:

	// REPRESENTATION
	int width;
	int height;
	int grid_border_x;
	int grid_border_y;
	int grid_spacing_x;
	int grid_spacing_y;
	int grid_x;
	int grid_y;

	std::vector<Screen> screens;
	
	class CAMERA_DATA {
	public:
	  unsigned long ip;
	  std::vector<Pt> geometry_data;
	  std::vector<bool> geometry_valid;

	  std::vector<std::vector<IntensityData> > intensity_data;

	  //std::vector<std::vector<IR_Interpolable_Data> > intensity_data;
	  
	  //std::vector<IR_Interpolable_Data> avg_grid_data;
	  //std::vector<IR_Interpolable_Data> avg_laser_data;


	  //	  std::vector<double> grid_brightness;
	  //std::vector<double> grid_radius;
	  //std::vector<double> laser_brightness;
	  //std::vector<double> laser_radius;

	  std::vector<IR_Interpolable_Data> laser_interpolable_data;
	  std::vector<IR_Interpolable_Data> grid_interpolable_data;
	  
	  //std::vector<Ellipse> ellipses;
	  	  
	  //double average_brightness;
	  //double average_radius;
	  
	  //	  double min_brightness;
	  //double max_brightness;
	  //double min_radius;
	  //double max_radius;
	};

 public:
	std::vector<CAMERA_DATA> all_my_data;
 private:
	//_brightness;
	//std::vector<std::vector<double> > intensity_radius;



public:
	void overall_scale(double &b, double &r) {
		/*
    double diff_b = max_brightness - min_brightness;
    double diff_r = max_radius - min_radius;

    b = (b-min_brightness+0.1*diff_b) / (max_brightness - min_brightness + 0.2*diff_b);
    r = (r-min_radius+0.1*diff_r) / (max_radius - min_radius+0.2*diff_r);
		 */

	  /*
	  b *= 0.37; //width
	  r *= 0.48; //height
	  */
	  b *= 0.25; //width
	  r *= 0.32; //height

	}


};

// ==========================================================================================
// ==========================================================================================

inline double DistanceBetweenTwoPoints(const Pt &p1, const Pt &p2) {
	return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}

//void computeEllipse(const Pt &centroid, const std::vector<Pt> &data, Ellipse &ellipse);



#endif
