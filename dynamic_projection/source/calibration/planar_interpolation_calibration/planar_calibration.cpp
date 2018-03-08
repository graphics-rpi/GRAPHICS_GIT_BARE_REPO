#include <iostream>
#include <fstream>
#include <map>
#include <iomanip>
#include <algorithm>
#include <list>
#include <sstream>

#include "planar_calibration.h"
#define square(x) ((x)*(x))

//int checkbounds[MAX_LASERS-17];
std::vector<std::string> PlanarCalibration::laser_names;//(MAX_LASERS,"XXX");
std::vector<std::string> PlanarCalibration::index_names;//(MAX_LASERS,"XXX");
std::vector<std::string> PlanarCalibration::colors;//(MAX_LASERS, "XXX");
std::vector<Vec3f> PlanarCalibration::fixed_laser_colors;//(MAX_LASERS, Vec3f(0,0,0));


void PlanarCalibration::initialize() {

  laser_names = std::vector<std::string>(MAX_LASERS,"XXX");
  index_names = std::vector<std::string>(MAX_LASERS,"XXX");
  colors = std::vector<std::string>(MAX_LASERS, "XXX");
  fixed_laser_colors = std::vector<Vec3f>(MAX_LASERS, Vec3f(0,0,0));

  assert (laser_names.size() == MAX_LASERS);

  for (int i = 0; i < MAX_LASERS; i++) {
    std::stringstream ss;
    ss << (i+1);
    laser_names[i] = ss.str();
  }

  for (int i = 0; i < MAX_LASERS; i++) {
    std::stringstream ss;
    ss << (i/4) << "," << (i%4);
    index_names[i] = ss.str();
  }

  for (int i = 0; i < MAX_LASERS; i++) {
    if      (i == 0) colors[i] = "#004CFF";
    else if (i == 1) colors[i] = "#FF0000";
    else if (i == 2) colors[i] = "#00FF00";
    else if (i == 3) colors[i] = "#B200FF";
    else if (i == 4) colors[i] = "#E5E500";
    else if (i == 5) colors[i] = "#00FFFF";
    else if (i == 6) colors[i] = "#99FF00";
    else if (i == 7) colors[i] = "#B200FF";
    else if (i == 8) colors[i] = "#0099FF";
    else if (i == 9) colors[i] = "#00FFFF";
    else if (i == 10) colors[i] = "#FFDE00";
    else if (i == 11) colors[i] = "#000000";
    else if (i == 12) colors[i] = "#444444";
    else if (i == 13) colors[i] = "#666666";
    else if (i == 14) colors[i] = "#888888";
    else if (i == 15) colors[i] = "#aaaaaa";
    else assert(0);
  }

  for (int i = 0; i < MAX_LASERS; i++) {
    if      (i == 0) fixed_laser_colors[i] = Vec3f(0,0.3,1);       // 1 = blue
    else if (i == 1) fixed_laser_colors[i] = Vec3f(1,0,0);       // 2 = red
    else if (i == 2) fixed_laser_colors[i] = Vec3f(0,1,0);       // 3 = green
    else if (i == 3) fixed_laser_colors[i] = Vec3f(0.7,0,1);     // 4  =  purple
    else if (i == 4) fixed_laser_colors[i] = Vec3f(0.9,0.9,0);       // 5 = yellow 
    else if (i == 5) fixed_laser_colors[i] = Vec3f(0,1,1);        // 6 = cyan
    else if (i == 6) fixed_laser_colors[i] = Vec3f(0.5,0.5,0.5);                      //            7 =     mgrey    NONE
    else if (i == 7) fixed_laser_colors[i] = Vec3f(1,0.5,0.5);                                    //                 4
    else if (i == 8) fixed_laser_colors[i] = Vec3f(0,0.5,1);     //                      9 = lt blue
    else if (i == 9) fixed_laser_colors[i] = Vec3f(0.7,1,0.7);                              //            6 =     white        same
    else if (i == 10) fixed_laser_colors[i] =Vec3f(1,1,0.5);     //                        11 =     white         same
    else if (i == 11) fixed_laser_colors[i] =Vec3f(0.7,1,1);     //                        12 =    white         same
    else if (i == 12) fixed_laser_colors[i] = Vec3f(0.2,0.2,0.2);                              //            6 =     white        same
    else if (i == 13) fixed_laser_colors[i] =Vec3f(0.4,0.4,0.4);     //                        11 =     white         same
    else if (i == 14) fixed_laser_colors[i] =Vec3f(0.6,0.6,0.6);     //                        12 =    white         same
    else if (i == 15) fixed_laser_colors[i] =Vec3f(0.8,0.8,0.8);     //                        12 =    white         same
    else assert(0);
  }
    
}


// ============
// CONSTRUCTORS
// ============

// construct an empty matrix to store the calibration grid data
PlanarCalibration::PlanarCalibration(int w, int h, 
				     int border_x, int border_y, int spacing_x, int spacing_y) {

  initialize();

  width = w;
  height = h;

  std::cout << "PC CONSTRUCTOR " << w << "x" << h << std::endl;
  grid_border_x = border_x;
  grid_border_y = border_y;
  grid_spacing_x = spacing_x;
  grid_spacing_y = spacing_y;
  grid_x = (width-2*grid_border_x)/grid_spacing_x+1;
  grid_y = (height-2*grid_border_y)/grid_spacing_y+1;
  //geometry_valid = std::vector<bool>(grid_x*grid_y,false);
  //geometry_data = std::vector<Pt>(grid_x*grid_y,Pt(-1,-1));
  clear_intensities();
}

void PlanarCalibration::clear_intensities() {

  for (unsigned int c = 0; c < all_my_data.size(); c++) {

    all_my_data[c].intensity_data = std::vector<std::vector<IntensityData> >
      (grid_x*grid_y,std::vector<IntensityData>(MAX_LASERS));
    
    all_my_data[c].laser_interpolable_data = std::vector<IR_Interpolable_Data>(MAX_LASERS);

    /*
    all_my_data[i].grid_brightness = std::vector<double>(grid_x*grid_y,-1);
    all_my_data[i].grid_radius = std::vector<double>(grid_x*grid_y,-1);
    all_my_data[i].laser_brightness = std::vector<double>(MAX_LASERS,-1);
    all_my_data[i].laser_radius = std::vector<double>(MAX_LASERS,-1);
    
    all_my_data[i].average_brightness = -1;
    all_my_data[i].average_radius = -1;
    */
  }
}


void PlanarCalibration::clear_laser_intensity(int laser) {

  assert (laser >= 0 && laser < MAX_LASERS);
  for (unsigned int j = 0; j < all_my_data.size(); j++) {
    for (int i = 0; i < grid_x*grid_y; i++) {
      all_my_data[j].intensity_data[i][laser] = IntensityData();
    }
  }
}


// read the data from a file
PlanarCalibration::PlanarCalibration(const std::string &geometry_filename, const std::string &intensity_filename, bool fatal)throw(int) {

  initialize();

  //std::cout << "PC all my data num " << all_my_data.size() << std::endl;
  
  try
    {
      read_geometry_data(geometry_filename, fatal);
    }
  catch(int i)
    {
      throw i;
    }
  

  LookForScreens();
  
  clear_intensities();
  bool success = false;
  int count = 10;
  while (count > 0) {
    success = read_intensity_data(intensity_filename);
    if (success) break;
    std::cout << "TRY TO READ AGAIN" << std::endl;
    count--;
  }
  if (success) //assert (success);
    compute_average_intensity();

  //std::cout << "planar calibration data read" << std::endl;
  
  //std::cout << "PC2 all my data num " << all_my_data.size() << std::endl;
  
}


void PlanarCalibration::read_geometry_data(const std::string &geometry_filename, bool fatal)throw(int) {
  std::ifstream istr(geometry_filename.c_str());
  if (!istr) {
    if(fatal)
      {
	std::cout << "FAILED TO OPEN FILE: " << geometry_filename << std::endl;
	
    	exit(1);
      }
    else
      {
    	throw -1;
      }
  }
  assert (istr);
  std::string token;
  istr >> token; assert (token == std::string("width")); istr >> width;
  istr >> token; assert (token == std::string("height")); istr >> height;
  istr >> token; assert (token == std::string("grid_border_x")); istr >> grid_border_x;
  istr >> token; assert (token == std::string("grid_border_y")); istr >> grid_border_y;
  istr >> token; assert (token == std::string("grid_spacing_x")); istr >> grid_spacing_x;
  istr >> token; assert (token == std::string("grid_spacing_y")); istr >> grid_spacing_y;
  grid_x = (width-2*grid_border_x)/grid_spacing_x+1;
  grid_y = (height-2*grid_border_y)/grid_spacing_y+1;

  int num_cameras;
  istr >> token; assert (token == std::string("num_cameras")); istr >> num_cameras;

  for (int n = 0; n < num_cameras; n++) {

    unsigned long ip;
    istr >> token; assert (token == std::string("camera")); istr >> ip;

    all_my_data.push_back(CAMERA_DATA());
    
    all_my_data[n].ip = ip;
    all_my_data[n].geometry_valid = std::vector<bool>(grid_x*grid_y,false);
    all_my_data[n].geometry_data = std::vector<Pt>(grid_x*grid_y,Pt(-1,-1));

    int invalid_point_count = 0;
    for (int i = 0; i < grid_x; i++) {
      for (int j = 0; j < grid_y; j++) {
	int ti, tj;
	istr >> ti >> tj;
	assert (i == ti && j == tj);
	double tmpx, tmpy;
	istr >> tmpx;
	if (tmpx < 0) {
	  all_my_data[n].geometry_valid[getIndex(i,j)] = false;
	  invalid_point_count++;
	} else {
	  istr >> tmpy;
	  assert (tmpy > 0);
	  all_my_data[n].geometry_valid[getIndex(i,j)] = true;
	  all_my_data[n].geometry_data[getIndex(i,j)] = Pt(tmpx,tmpy);
	}
      }
    }        
    if (invalid_point_count > 0) {
      std::cout << "WARNING: " << invalid_point_count << " invalid data points out of " << grid_x*grid_y << " total points." << std::endl;
    }
  }


  LookForScreens();
}


bool PlanarCalibration::read_intensity_data(const std::string &intensity_filename) {

  //std::cout << "READ INTENSITYXall my data num " << all_my_data.size() << std::endl;

  std::ifstream istr(intensity_filename.c_str());
  if (!istr) {
    std::cout << "FAILED TO OPEN FILE: " << intensity_filename << std::endl;
    return false;
  }
  assert (istr);
  std::string token;
  int gx,gy;
  istr >> token; assert (token == std::string("grid_x")); istr >> gx;
  istr >> token; assert (token == std::string("grid_y")); istr >> gy;
  assert (grid_x == gx);
  assert (grid_y == gy);

  int num_cameras;
  istr >> token >> num_cameras;
  assert (token == "num_cameras");
  assert (num_cameras == (int)all_my_data.size());

  //double brightness,radius;
  
  for (unsigned int i = 0; i < all_my_data.size(); i++) {

    unsigned long camera_ip;
    istr >> token >> camera_ip;

    assert (all_my_data[i].ip == camera_ip);

    double read_index;
    for (int index = 0; index < grid_x*grid_y; index++) {    
      istr >> read_index;
      //std::cout << "compare " << index << " " << read_index << std::endl;
      if (index != read_index) return false;
      assert (index == read_index);
      for (int laser = 0; laser < MAX_LASERS; laser++) {
	istr >> all_my_data[i].intensity_data[index][laser];
	all_my_data[i].intensity_data[index][laser].compute_avg(); 
      }
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream &ostr, const IntensityData &data) {
  ostr << "  " << data.num_points << "\n";
  for (int i = 0; i < data.num_points; i++) {
    int index = i;
    if (data.num_points == MAX_INTENSITY_SAMPLES) {
      index = (data.next+i)%MAX_INTENSITY_SAMPLES;
    }

    ostr << "    " << data.samples[index] << "\n";

  }
  return ostr;
}



std::istream& operator>>(std::istream &istr, IntensityData &data) {
  double num; 
  istr >> num;
  for (int i = 0; i < num; i++) {
    IR_Interpolable_Data i_d;
    istr >> i_d;
    data.addDataPoint(i_d);
  }
  return istr;
}


void PlanarCalibration::compute_average_intensity() {

  for (unsigned int c = 0; c < all_my_data.size(); c++) {

    // compute grid point averages
    all_my_data[c].grid_interpolable_data = std::vector<IR_Interpolable_Data>(grid_x*grid_y);
    for (int index = 0; index < grid_x*grid_y; index++) {    
      Weighted_Average_IR_Interpolable_Data waid;      
      for (int laser = 0; laser < MAX_LASERS; laser++) {
	if (all_my_data[c].intensity_data[index][laser].valid()) {
	  IR_Interpolable_Data tmp = all_my_data[c].intensity_data[index][laser].get_average();
	  assert (tmp.valid());
	  waid.addSample(tmp,1.0);
	}
      }
      all_my_data[c].grid_interpolable_data[index] = waid.getAverage();
    }

    
    // compute laser averages
    all_my_data[c].laser_interpolable_data = std::vector<IR_Interpolable_Data>(MAX_LASERS);
    for (int laser = 0; laser < MAX_LASERS; laser++) {
      Weighted_Average_IR_Interpolable_Data waid;
      for (int index = 0; index < grid_x*grid_y; index++) {    
	if (all_my_data[c].intensity_data[index][laser].valid()) {
	  IR_Interpolable_Data tmp = all_my_data[c].intensity_data[index][laser].get_average();
	  assert (tmp.valid());
	  waid.addSample(tmp,1.0);
	}
      }
      all_my_data[c].laser_interpolable_data[laser] = waid.getAverage();
    }
        
  }
}







#if 1
#define NO_NORMALIZATION 1
#define SPATIAL_NORMALIZATION 0
#define SPATIAL_AND_LASER_NORMALIZATION 0

#else
#if 0

#define NO_NORMALIZATION 0
#define SPATIAL_NORMALIZATION 1
#define SPATIAL_AND_LASER_NORMALIZATION 0

#else

#define NO_NORMALIZATION 0
#define SPATIAL_NORMALIZATION 0
#define SPATIAL_AND_LASER_NORMALIZATION 1

#endif
#endif


IR_Interpolable_Data PlanarCalibration::NormalizeIntensityData(int index, int laser, const IR_Interpolable_Data &in) const {

  assert (index >= 0 && index < grid_x*grid_y);
  
  // NEED TO FIX THIS FOR MULTIPLE CAMERAS

#if NO_NORMALIZATION
  // no normalization
  //std::cout << "NO NORMALIZATION" << std::endl;
  return in;
#endif 
 
#if SPATIAL_NORMALIZATION
  int ip_index = 0;
  IR_Interpolable_Data tmp = all_my_data[ip_index].grid_interpolable_data[index];
  IR_Interpolable_Data answer = in;
  answer.ScaleWithAverage(tmp);
  //std::cout << "SPATIAL NORMALIZATION" << std::endl;
  return answer;
#endif

#if SPATIAL_AND_LASER_NORMALIZATION
  int ip_index = 0;
  if (!all_my_data[ip_index].intensity_data[index][laser].valid()) {
    return in;
  }
  //  IR_Interpolable_Data tmp = all_my_data[ip_index].intensity_data[index][laser];
  IR_Interpolable_Data tmp = all_my_data[ip_index].intensity_data[index][laser].get_average();
  IR_Interpolable_Data answer = in;
  answer.ScaleWithAverage(tmp);
  //std::cout << "SPATIAL AND LASER NORMALIZATION" << std::endl;
  
  return answer;
#endif

  
}









/*
static void computeBestFitLine(const std::vector<Pt> &points, double &slope, double &intercept){
	double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
	int n = points.size();
	for (int i = 0; i < n; ++i)
	{
		sx += points[i].x;
		sy += points[i].y;
		sxx += points[i].x*points[i].x;
		sxy += points[i].x*points[i].y;
	}
	double delta = n*sxx - sx*sx;
	slope = (n*sxy - sx*sy)/delta;
	intercept = (sxx*sy - sx*sxy)/delta;
	/ *
	double sx = 0.0, sy = 0.0, stt = 0.0, sts = 0.0;
	int n = points.size();
	for (int i = 0; i < n; i++){
		sx += points[i].x;
		sy += points[i].y;
	}
	for (int i = 0; i < n; i++){
		double t = points[i].x - sx/n;
		stt += t*t;
		sts += t*points[i].y;
	}
	m = sts/stt;
	b = (sy - sx*m)/n;
	* /
}

static double distance(Pt p1, Pt p2){
	return sqrt((p1.x-p2.x)*(p1.x-p2.x)+(p1.y-p2.y)*(p1.y-p2.y));
}
*/

/*
void computeEllipse(const Pt &centroid, const std::vector<Pt> &data, Ellipse &ellipse) {

	//since computeBestLine computes the best line, but not through the
	//centroid, need to recompute b based on the centroid
	double m,b,bp;
	computeBestFitLine(data,m,b);

	double mp = -1.0/m;

	//recalculate y axis intercepts based on centroid 
	b = centroid.y - m*centroid.x;
	bp = centroid.y - mp*centroid.x;

	std::vector<double> flattened;

	double major=0,minor=0;

	for(unsigned i = 0; i < data.size(); i++){
		Pt intercept;
		intercept.x = (-mp*data[i].x+data[i].y-b)/(m-mp);
		intercept.y = m*intercept.x+b;
		flattened.push_back(distance(centroid,intercept));
	}

	if (flattened.size() > 0) {
		std::sort(flattened.begin(),flattened.end());
		int index = flattened.size()*0.95+1;
		if (index > (int)flattened.size()-1) index = flattened.size()-1;
		//ellipse.major_radius = flattened[index];
		major = flattened[index];
	} 

	flattened.clear();

	for(unsigned i = 0; i < data.size(); i++){
		Pt intercept;
		intercept.x = (-m*data[i].x+data[i].y-bp)/(mp-m); 
		intercept.y = mp*intercept.x+bp;
		flattened.push_back(distance(intercept,centroid));
	}

	if (flattened.size() > 0) {
		std::sort(flattened.begin(),flattened.end());
		int index = flattened.size()*0.95+1;
		if (index > (int)flattened.size()-1) index = flattened.size()-1;

		//ellipse.minor_radius = flattened[index];
		minor = flattened[index];
	} 

	double angle = atan(m);

	//ellipse.axis = Pt(sin(angle),cos(angle));

	Pt axis = Pt(cos(angle),sin(angle));

	axis.Normalize();

	ellipse = Ellipse(centroid,axis,major,minor);

/ * 
  std::vector<double> diffs;
  for (unsigned int index = 0; index < data.size(); index++) {
    Pt p = data[index];
    double diff = sqrt((centroid.x-p.x)*(centroid.x-p.x) + 
		       (centroid.y-p.y)*(centroid.y-p.y));
    diffs.push_back(diff);
  }  
  double radius = 0;
  if (diffs.size() > 0) {
    std::sort(diffs.begin(),diffs.end());
    int index = diffs.size()*0.95+1;
    if (index > (int)diffs.size()-1) index = diffs.size()-1;
    radius = diffs[index];
  } 

  ellipse.centroid = centroid;
  ellipse.axis = Pt(1,2);
  ellipse.axis.Normalize();
  ellipse.major_radius = 1.0*radius;
  ellipse.minor_radius = 0.5*radius;
* /
}
*/


// ==================
// WRITE DATA TO FILE
// ==================

void PlanarCalibration::write_geometry_to_file(const std::string &geometry_filename) {
  std::ofstream ostr(geometry_filename.c_str());
  if (!ostr) {
    std::cout << "FAILED TO OPEN FILE: " << geometry_filename << std::endl;
    exit(1);
  }
  assert (ostr);
  ostr << "width " << width << "\n";
  ostr << "height " << height << "\n";
  ostr << "grid_border_x " << grid_border_x << "\n";
  ostr << "grid_border_y " << grid_border_y << "\n";
  ostr << "grid_spacing_x " << grid_spacing_x << "\n";
  ostr << "grid_spacing_y " << grid_spacing_y << "\n";

  ostr << "num_cameras " << all_my_data.size() << "\n";

  for (unsigned int n = 0; n < all_my_data.size(); n++) {

    ostr << "camera " << all_my_data[n].ip << "\n";

    int invalid_point_count = 0;
    for (int i = 0; i < grid_x; i++) {
      for (int j = 0; j < grid_y; j++) {
	ostr << i << " " << j << " ";
	if (!all_my_data[n].geometry_valid[getIndex(i,j)]) {
	  ostr << "-1\n";
	  invalid_point_count++;
	} else {
	  ostr << all_my_data[n].geometry_data[getIndex(i,j)].x << " " << all_my_data[n].geometry_data[getIndex(i,j)].y << "\n";
	}
      }
    }
    if (invalid_point_count > 0) {
      std::cout << "WARNING: invalid point(s) in file!" << std::endl;
      std::cout << "       " << invalid_point_count << " invalid out of " << grid_x*grid_y << " total points" << std::endl;
    }
  }
}


void PlanarCalibration::write_intensity_to_file(const std::string &intensity_filename) {
  std::ofstream ostr(intensity_filename.c_str());
  if (!ostr) {
    std::cout << "FAILED TO OPEN FILE: " << intensity_filename << std::endl;
    exit(1);
  }
  assert (ostr);
  ostr << "grid_x " << grid_x << "\n";
  ostr << "grid_y " << grid_y << "\n";

  ostr << "num_cameras " << all_my_data.size() << std::endl;

  for (unsigned int i = 0; i < all_my_data.size(); i++) {
    ostr << "camera " << all_my_data[i].ip << std::endl;
    
    for (int index = 0; index < grid_x*grid_y; index++) {    
      ostr << index << "\n";
      for (int laser = 0; laser < MAX_LASERS; laser++) {
	ostr << all_my_data[i].intensity_data[index][laser] << "\n";
	// << "  " << intensity_brightness[index][laser]
	// << "  " << intensity_radius[index][laser] << "\n";
      }
    }

  }

}

// This function writes out a GNU Plot file for different lasers as data points
void PlanarCalibration::writeOutGNUPlotFile_AllLasersAllCalibrationSpots( std::vector< std::vector< std::vector< std::string > > >& data_filenames, const std::string &GNUPlot_filename_prefix )
{
  // Name the GNU Plot File
  std::string localFilename = "planar_calibration_gnu_plot_AllLasersAllCalibrationSpots";
  std::stringstream ss;
  ss << GNUPlot_filename_prefix << localFilename << ".gp";
  std::cout << "Writing GNUPlot data to " << ss.str() << std::endl;
  std::ofstream ostr( ss.str().c_str() );
  assert( ostr );
  // Write the file
  ostr << "# GNUPlot Data" << std::endl;
  ostr << "reset" << std::endl << "set terminal svg size 1500,1150" << std::endl << "# set terminal png transparent nocrop enhanced 8 size 1500,1150" << 
    std::endl << "set output \'AllLasers.svg\'" << std::endl << "# set title \"All Lasers\"" << std::endl;
  ostr << "# Define the line style" << std::endl;
  ostr << "set yrange [0:300]" << std::endl;
  ostr << "set nokey" << std::endl;
  ostr << "set format xy \"\"" << std::endl;
  ostr << "# set xlabel \"Distance From Center\"" << std::endl;
  ostr << "# set ylabel \"Intensity\"" << std::endl;
  // For each possible laser, write a line style
  for( int i = 0 ; i < MAX_LASERS ; i++ )
  {
    // The pointtype should maybe all be the same once we find something we line? ALSO, WE SHOULD CHANGE 12 IF WE GET MORE COLORS
    ostr << "set style line " << i + 1 << " lc rgbcolor \"" << colors[ i % MAX_LASERS ] << "\"" << std::endl;
// << "set style line " << i + 1 << " pointtype " << i + 1 << std::endl;
  }

  // Now, write out the data manipulation
  // Annoying thing to prevent too many commas
  bool first = true;
  for( unsigned int i = 0 ; i < all_my_data.size() ; i++ )
  {
    for( int index = 0 ; index < grid_x*grid_y ; index++ )
    { 
      for( int laser = 0 ; laser < MAX_LASERS ; laser++ )
      {
        for( int j = 2 ; j < all_my_data[ i ].intensity_data[ index ][ laser ].NumPoints() + 1 ; j++ )
        {
          if( first )
          {
            ostr << "plot  \'" << data_filenames[ i ][ index ][ laser ].c_str() << "\' u 1:" << j << " ls " << laser + 1 << " with lines";
            first = false;
          } // if
          else
          {
            ostr << ", \\" << std::endl << "\'" << data_filenames[ i ][ index ][ laser ].c_str() << "\' u 1:" << j << " ls " << laser + 1 << " with lines";
          } // if else
        } // for
      } // for
    } // for
  } // for
} // writeOutGNUPlotFile


// This function writes out a GNUPlot file for all the lasers at a single screen location
void PlanarCalibration::writeOutGNUPlotFile_DiffLasers( int spot, std::vector< std::vector< std::vector< std::string > > >& data_filenames, const std::string &GNUPlot_filename_prefix )
{
  assert( spot >= 0 && spot < grid_x * grid_y );
  std::string localFilename = "planar_calibration_gnu_plot_DiffLasers";
  std::stringstream ss;
  ss << GNUPlot_filename_prefix << localFilename << "_spot_" << spot << ".gp";
  std::cout << "Writing GNUPlot data to " << ss.str() << std::endl;
  std::ofstream ostr( ss.str().c_str() );
  assert( ostr );
  // Write the file
  ostr << "# GNUPlot Data" << std::endl;
  ostr << "reset" << std::endl << "set terminal svg size 1500,1150" << std::endl << "# set terminal png transparent nocrop enhanced 8 size 1500,1150" << 
    std::endl << "set output \'AllLasers_spot" << spot << ".svg\'" << std::endl << "# set title \"Variation Among Lasers\"" << std::endl;
  ostr << "# Define the line style" << std::endl;
  ostr << "set yrange [0:300]" << std::endl;
  ostr << "set nokey" << std::endl;
  ostr << "set format xy \"\"" << std::endl;
  ostr << "# set xlabel \"Distance From Center\"" << std::endl;
  ostr << "# set ylabel \"Intensity\"" << std::endl;

  // For each possible laser, write a line style
  for( int i = 0 ; i < MAX_LASERS ; i++ )
  {
    // The pointtype should maybe all be the same once we find something we line? ALSO, WE SHOULD CHANGE 12 IF WE GET MORE COLORS
    ostr << "set style line " << i + 1 << " lc rgbcolor \"" << colors[ i % MAX_LASERS ] << "\"" << std::endl;
// << "set style line " << i + 1 << " pointtype " << i + 1 << std::endl;
  }

  // Now, write out the data manipulation
  // Annoying thing to prevent too many commas
  bool first = true;
  for( unsigned int i = 0 ; i < all_my_data.size() ; i++ )
  {
    //for( int index = 0 ; index < grid_x*grid_y ; index++ )
    //{ 
      for( int laser = 0 ; laser < MAX_LASERS ; laser++ )
      {
        for( int j = 2 ; j < all_my_data[ i ].intensity_data[ spot ][ laser ].NumPoints() + 1 ; j++ )
        {
          if( first )
          {
            ostr << "plot \'" << data_filenames[ i ][ spot ][ laser ].c_str() << "\' u 1:" << j << " ls " << laser + 1 << " with lines";
            first = false;
          } // if
          else
          {
            ostr << ", \\" << std::endl << "\'" << data_filenames[ i ][ spot ][ laser ].c_str() << "\' u 1:" << j << " ls " << laser + 1 << " with lines";
          } // else
        } // for
      } // for
    //} // for
  } // for

} // writeOutGNUPlotFile_DiffLasers


// This function writes out a GNUPlot file for all locations and a single laser
void PlanarCalibration::writeOutGNUPlotFile_DiffSpots( int laser, std::vector< std::vector< std::vector< std::string > > >& data_filenames, const std::string &GNUPlot_filename_prefix )
{
  assert( laser >= 0 && laser < MAX_LASERS );
  std::string localFilename = "planar_calibration_gnu_plot_DiffSpots";
  std::stringstream ss;
  ss << GNUPlot_filename_prefix << localFilename << "_laser_" << laser << ".gp";
  std::cout << "Writing GNUPlot data to " << ss.str() << std::endl;
  std::ofstream ostr( ss.str().c_str() );
  assert( ostr );
  // Write the file
  ostr << "# GNUPlot Data" << std::endl;
  ostr << "reset" << std::endl << "set terminal svg size 1500,1150" << std::endl << "# set terminal png transparent nocrop enhanced 8 size 1500,1150" << 
    std::endl << "set output \'AllSpots_laser" << laser << ".svg\'" << std::endl << "# set title \"Variation Among Spots\"" << std::endl;
  ostr << "# Define the line style" << std::endl;
  ostr << "set yrange [0:300]" << std::endl;
  ostr << "set nokey" << std::endl;
  ostr << "set format xy \"\"" << std::endl;
  ostr << "# set xlabel \"Distance From Center\"" << std::endl;
  ostr << "# set ylabel \"Intensity\"" << std::endl;

  // For each possible laser, write a line style
  for( int i = 0 ; i < MAX_LASERS ; i++ )
  {
    // The pointtype should maybe all be the same once we find something we line? ALSO, WE SHOULD CHANGE MAX_LASERS IF WE GET MORE COLORS
    ostr << "set style line " << i + 1 << " lc rgbcolor \"" << colors[ i % 12 ] << "\"" << std::endl;
// << "set style line " << i + 1 << " pointtype " << i + 1 << std::endl;
  }

  // Now, write out the data manipulation
  // Annoying thing to prevent too many commas
  bool first = true;
  for( unsigned int i = 0 ; i < all_my_data.size() ; i++ )
  {
    for( int index = 0 ; index < grid_x*grid_y ; index++ )
    { 
      //for( int laser = 0 ; laser < MAX_LASERS ; laser++ )
      //{
        for( int j = 2 ; j < all_my_data[ i ].intensity_data[ index ][ laser ].NumPoints() + 1 ; j++ )
        {
// IF WE WANT THE POINTS BACK, CHANGES WITH LINES TO WITH LINESPOINTS
          if( first )
          {
            ostr << "plot \'" << data_filenames[ i ][ index ][ laser ].c_str() << "\' u 1:" << j << " ls " << index + 1 << " with lines";
            first = false;
          } // if
          else
          {
            ostr << ", \\" << std::endl << "\'" << data_filenames[ i ][ index ][ laser ].c_str() << "\' u 1:" << j << " ls " << index + 1 << " with lines";
          } // else
        } // for
      //} // for
    } // for
  } // for


} // writeOutGNUPlotFile_DiffSpots


// This function writes all separate GNUPlot data files
void PlanarCalibration::writeOutGNUPlotDataFiles( const std::string &GNUPlot_filename_prefix )
{
  // First, create a series of filenames for all of the lasers
  std::vector< std::vector< std::vector< std::string > > > data_filenames( 
    all_my_data.size(), std::vector< std::vector< std::string > >( 
      grid_x * grid_y, std::vector< std::string >( MAX_LASERS, "" )
    )
  );
  for( unsigned int i = 0 ; i < all_my_data.size() ; i++ )
  {
    for( int index = 0 ; index < grid_x*grid_y ; index++ )
    { 
      for( int laser = 0 ; laser < MAX_LASERS ; laser++ )
      {
         std::stringstream temp( "" );
         temp << "planar_calibration_gnu_plot_data_file_camera" << i << "_gridSpace" << index << "_laser" << laser << ".dat";
         data_filenames[ i ][ index ][ laser ] = temp.str();
      } // for
    } // for
  } // for

  // For each laser and grid location, output the series of histograms
  for( unsigned int i = 0 ; i < all_my_data.size() ; i++ )
  {
    for( int index = 0 ; index < grid_x*grid_y ; index++ )
    { 
      for( int laser = 0 ; laser < MAX_LASERS ; laser++ )
      {
        // Create a file
        std::stringstream newString;
        newString << GNUPlot_filename_prefix << data_filenames[ i ][ index ][ laser ];
        std::ofstream ostr( newString.str().c_str() );
        assert( ostr );        

        // Write the file
        ostr << "Bins ";
        for( int j = 0 ; j < all_my_data[ i ].intensity_data[ index ][ laser ].NumPoints() ; j++ )
        {
          ostr << "Hist" << j << " ";
        }
        ostr << std::endl;

        // Across the top, lots of labels
        for( int k = 0 ; k < NUM_BRIGHTNESS_BINS ; k++ )
        {
          ostr << k << " ";
          for( int j = 0 ; j < all_my_data[ i ].intensity_data[ index ][ laser ].NumPoints() ; j++ )
          {
            // Output the histogram
            ostr << all_my_data[i].intensity_data[index][laser].getSample( j ) -> brightness_histogram_[ k ] << " ";
          } // for
          ostr << std::endl;
        } // for
      } // for
    } // for
  } // for

  // Now, write out the GNUPlot Files
  writeOutGNUPlotFile_AllLasersAllCalibrationSpots( data_filenames, GNUPlot_filename_prefix );
  // Write out a file for each laser
  for( int i = 0 ; i < MAX_LASERS ; i++ )
  {
    writeOutGNUPlotFile_DiffSpots( i, data_filenames, GNUPlot_filename_prefix );
  }
  for( int i = 0 ; i < grid_x * grid_y ; i++ )
  {
//    if( i == 0 || i == 3 || i == 5 || i == 6 || i == 8 || i == 11 )
      writeOutGNUPlotFile_DiffLasers( i, data_filenames, GNUPlot_filename_prefix );
  }
} // writeGNUPlotDataFiles


// =========================
// BARYCENTRIC INTERPOLATION
// =========================

static bool BarycentricCoordinates(const Pt &a, const Pt &b, const Pt &c,
			    const Pt &v, double &alpha, double &beta, double &gamma) {
  double det = ((a.x-c.x)*(b.y-c.y)) - ((b.x-c.x)*(a.y-c.y));
  if (fabs(det) < 0.00000001) {
    alpha = beta = gamma = 0;
    return false;
  }
  alpha = (((b.y-c.y)*(v.x-c.x))-(b.x-c.x)*(v.y-c.y)) / det;
  beta = -(((a.y-c.y)*(v.x-c.x))-(a.x-c.x)*(v.y-c.y)) / det;
  gamma = 1 - alpha - beta;
  return true;
}

int PlanarCalibration::ClosestGridPoint(double x, double y, unsigned long ip) {
  
  int ip_index = getIpIndex(ip);

  // Find the closest grid point to the input (the first point of the triangle)
  double closest_dist = 10000000;  
  int answer = -1;
  for (int i = 0; i < grid_x*grid_y; i++) {
    if (!all_my_data[ip_index].geometry_valid[i]) continue; //{ std::cout << "ERROR: invalid data point " << std::endl; exit(0); } //continue;
    Pt pt = all_my_data[ip_index].geometry_data[i];
    double dist = sqrt((pt.x-x)*(pt.x-x)+(pt.y-y)*(pt.y-y));
    if (dist < closest_dist) {
      answer = i;      
      closest_dist = dist;
    }
  }
  if (closest_dist < grid_spacing_x && closest_dist < 0.2*grid_spacing_y) {
    return answer;
  }
  //std::cout << "bad point!! " << closest_dist << std::endl;
  return -1;
}

void PlanarCalibration::BarycentricInterpolation(double x, double y, unsigned long ip,
						 const IR_Interpolable_Data &interpolable_data_in,
						 IntTri &answer_triangle, Pt &answer_point,
						 std::vector<unsigned long> &ips_out,
						 std::vector<std::vector<IR_Interpolable_Data> >  &interpolable_data_out) {
  
  IntPt a,b,c;
  double closest_dist = 10000000;  
  assert (grid_x >= 1);
  assert (grid_y >= 1);  

  int ip_index = getIpIndex(ip);

  ips_out.clear();
  ips_out.push_back(ip);

  interpolable_data_out.clear();
  interpolable_data_out.push_back(std::vector<IR_Interpolable_Data>(MAX_LASERS));

  //std::cout << "grid_x " << grid_x << " grid_y " << grid_y << std::endl;

  // Find the closest grid point to the input (the first point of the triangle)
  for (int i = 0; i < grid_x; i++) {
    for (int j = 0; j < grid_y; j++) {
      if (!all_my_data[ip_index].geometry_valid[getIndex(i,j)]) continue;
      //{ std::cout << "ERROR: invalid data point " << std::endl; exit(0); } //continue;
      Pt pt = all_my_data[ip_index].geometry_data[getIndex(i,j)];
      double dist = sqrt((pt.x-x)*(pt.x-x)+(pt.y-y)*(pt.y-y));
      if (dist < closest_dist) {
	a = IntPt(i,j);
	closest_dist = dist;
      }
    }
  }
  
  assert (a.i >= 0 && a.i < grid_x);
  assert (a.j >= 0 && a.j < grid_y);

  // Find the next closest grid point on the x axis (the second point of the triangle)
  if (a.i == 0) {
    b = IntPt(a.i+1,a.j);
  } else if (a.i == grid_x-1) {
    b = IntPt(a.i-1,a.j);
  } else {
    assert (a.i-1 >= 0 && a.i-1 < grid_x);
    assert (a.i+1 >= 0 && a.i+1 < grid_x);
    assert (a.j >= 0 && a.j < grid_y);
    Pt b1 = all_my_data[ip_index].geometry_data[getIndex(a.i-1,a.j)];
    Pt b2 = all_my_data[ip_index].geometry_data[getIndex(a.i+1,a.j)];
    double dist1 = sqrt((b1.x-x)*(b1.x-x)+(b1.y-y)*(b1.y-y));
    double dist2 = sqrt((b2.x-x)*(b2.x-x)+(b2.y-y)*(b2.y-y));
    if (dist1 < dist2)
      b = IntPt(a.i-1,a.j);
    else
      b = IntPt(a.i+1,a.j);
  }
  
  // Find the next closest grid point on the y axis (the third point of the triangle)
  if (a.j == 0) {
    c = IntPt(a.i,a.j+1);
  } else if (a.j == grid_y-1) {
    c = IntPt(a.i,a.j-1);
  } else {
    Pt c1 = all_my_data[ip_index].geometry_data[getIndex(a.i,a.j-1)];
    Pt c2 = all_my_data[ip_index].geometry_data[getIndex(a.i,a.j+1)];
    double dist1 = sqrt((c1.x-x)*(c1.x-x)+(c1.y-y)*(c1.y-y));
    double dist2 = sqrt((c2.x-x)*(c2.x-x)+(c2.y-y)*(c2.y-y));
    if (dist1 < dist2)
      c = IntPt(a.i,a.j-1);
    else
      c = IntPt(a.i,a.j+1);
  }

  // Collect the 3 closest 
  Pt va = all_my_data[ip_index].geometry_data[getIndex(a.i,a.j)];
  Pt vb = all_my_data[ip_index].geometry_data[getIndex(b.i,b.j)];
  Pt vc = all_my_data[ip_index].geometry_data[getIndex(c.i,c.j)];
  
  double alpha,beta,gamma;
  bool success = BarycentricCoordinates(va,vb,vc,Pt(x,y),alpha,beta,gamma);
    
  Pt A = calibration_point(a);
  Pt B = calibration_point(b);
  Pt C = calibration_point(c);
  
  Pt tmp = A;
  if (success) {
    tmp = A*alpha + B*beta + C*gamma;
  }
  
  answer_point = tmp;
  
  answer_triangle.pts[0] = a;
  answer_triangle.pts[1] = b;
  answer_triangle.pts[2] = c;

  int a_index = getIndex(a.i,a.j);
  int b_index = getIndex(b.i,b.j);
  int c_index = getIndex(c.i,c.j);

  IR_Interpolable_Data interpolable_data_a = interpolable_data_in;
  IR_Interpolable_Data interpolable_data_b = interpolable_data_in;
  IR_Interpolable_Data interpolable_data_c = interpolable_data_in;

  for (int laser = 0; laser < MAX_LASERS; laser++) {

    NormalizeIntensityData(a_index,laser,interpolable_data_a);
    NormalizeIntensityData(b_index,laser,interpolable_data_b);
    NormalizeIntensityData(c_index,laser,interpolable_data_c);

    Weighted_Average_IR_Interpolable_Data waid;
    waid.addSample(interpolable_data_a,alpha);
    waid.addSample(interpolable_data_b,beta);
    waid.addSample(interpolable_data_c,gamma);

    interpolable_data_out[0][laser] = interpolable_data_in;
  }
}


void PlanarCalibration::MatchToLaserProfiles(const IR_Interpolable_Data &interpolable_data,
					     unsigned long ip,
					     std::vector<std::vector<double> > &intensity_differences,
					     bool /*print_lots*/) const {

  assert (intensity_differences[0].size() == MAX_LASERS);
  intensity_differences = std::vector<std::vector<double> >(1,std::vector<double>(MAX_LASERS,-1));

  // ======================================
  // hack, pretend the mouse is laser "12"
  /*
  if (brightness == -1 && radius == -1) {
    intensity_differences[0][11] = 1; 
    return;
  }
  */
  //  std::cout << "fix mouse hack" << std::endl;


  std::map<double,int> foo;

  assert (ip != 0);
  int ip_index = getIpIndex(ip);


  for (int laser = 0; laser < MAX_LASERS; laser++) {      

    //    intensity_differences[0][laser] = laser; //-1;

    if (!all_my_data[ip_index].laser_interpolable_data[laser].valid()) {//radius[laser] < 0) {
      intensity_differences[0][laser] = -1;
      continue;
    }

    else {
      //      std::cout << "laser " << laser << " is valid" << std::endl;

      const IR_Interpolable_Data &tmp = all_my_data[ip_index].laser_interpolable_data[laser];
      assert (tmp.valid());

      double diff = interpolable_data.CalculateDifference(tmp);
      //std::cout << "diff " << diff << std::endl;

      intensity_differences[0][laser] = diff;      
    }
  
    //std::cout << "LASER " << laser;

    /*
    Pt p = Pt(brightness,radius);

    const Ellipse &e = GetLaserEllipse(laser,ip);
    Pt centroid = e.getCentroid();
    Pt axis = e.getAxis();
    double major = e.getMajorRadius();
    //double minor = e.getMinorRadius();
    Pt foci1 = e.getFoci1();
    Pt foci2 = e.getFoci2();

    // implicit equation of ellipse
    double dist1 = 
      sqrt( square(p.x-foci1.x) + square(p.y-foci1.y)) +
      sqrt( square(p.x-foci2.x) + square(p.y-foci2.y));
    dist1 /= 2*major;

    double dist2 = 
      4*sqrt( square(p.x-centroid.x) + 
	    square(p.y-centroid.y) );

    //double dist = dist2;
    double dist;
    */

    //    std::cout << "WRITE THIS DIFFERENCING STUFF" << std::endl;

    /*
#if 0
    if (dist1 < 1) 
      dist = dist1;
    else if (dist1 < 2)
      dist = 1+0.5*(dist2+dist1);
    else
      dist = 2+dist2;
#else
    dist = dist1;
#endif
    //double dist = std::min(dist1,dist2);


    if (print_lots) {
      //      std::cout << "laser " << laser+1 << " major = " << major << " minor = " << minor << " dist " << dist;
      std::cout << "laser " << laser+1 << " centroid = " << centroid.x << " " << centroid.y;
    }


    if (print_lots) {
      std::cout << " dists " << dist1 << " " << dist2 << std::endl;
    }

    intensity_differences[0][laser] = dist;
    foo[dist] = laser;
    */
  }

  /*  
  if (print_lots) {
    std::cout << "MATCHES: ";
    // something buggy with this output
    for (std::map<double,int>::iterator itr = foo.begin(); itr != foo.end(); itr++) {
      std::cout << std::setw(2) << itr->second+1 << ":" 
    		<< std::setw(6) << std::setprecision(3) << std::fixed << itr->first << "  ";
    }
    std::cout << std::endl;

    std::cout << "         ";
    // something buggy with this output
    for (int i = 0; i < MAX_LASERS; i++) {
      if (intensity_differences[0][i] < 0) continue;
      std::cout << std::setw(2) << i+1 << ":" 
    		<< std::setw(6) << std::setprecision(3) << std::fixed << intensity_differences[0][i] << "  ";
    }
    std::cout << std::endl;

  }
  */
}


void PlanarCalibration::LookForScreens() {

    return;

  std::ifstream istr("/etc/X11/xorg.conf");
  if (!istr) {
    assert(0);
  }

  std::string line;
  bool twinview = false;
  bool xinerama = false;
  std::string metamodes;

  while (getline(istr,line)) {
    
    std::stringstream ss(line);
    
    std::string token, token2;
    ss >> token >> token2;
    if (token == "Option" && token2 == "\"TwinView\"") twinview = true;
    if (token == "Option" && token2 == "\"Xinerama\"") {
      ss >> token; if (token == "\"on\"") xinerama = true;
    }
    if (token == "Option" && token2 == "\"metamodes\"") {
      assert (twinview == true);
      assert (xinerama == true);
      ss >> token; assert (token == "\"DFP-0:");
      ss >> token; assert (token == "nvidia-auto-select");
      ss >> token; assert (token == "+0+0,");
      ss >> token; assert (token == "DFP-2:");
      ss >> metamodes; assert (metamodes[metamodes.size()-1] == '\"');
      metamodes.resize(metamodes.size()-1);
    }
  }
  
  assert (twinview == true);
  assert (xinerama == true);
  //std::cout << "metamodes  " << metamodes << std::endl;
  
    std::cout << "width = " << width << " height = " << height << std::endl;

  std::stringstream ss(metamodes);
  Screen s1,s2;

  char tok;
  int a,b,c,d;
  ss >> a >> tok;
  ss >> b >> tok;
  ss >> c >> tok >> d;

  //  std::cout << a << " " << b << " " << c << " " << d << std::endl;

  assert (a > 0 && b > 0 && c > 0 && d > 0);

  std::cout << a+c << " " << width << std::endl;
  assert (a + c == width);
  assert (b + d < height);

  s1.bottomleft = Pt(0,0);
  s1.topright = Pt(c,height); //height);
  s2.bottomleft = Pt(c,height-(d+b));
  s2.topright = Pt(width,height-d);

  screens.clear();
  screens.push_back(s1);
  screens.push_back(s2);

  // match ips

  assert (screens.size() == 2);
  assert (all_my_data.size() == 2);

  bool screen_camera[2][2] = { { true, true } , { true, true} };
  //  screen_camera[1][0] = false;
  //screen_camera[0][1] = false;

  for (unsigned int n = 0; n < all_my_data.size(); n++) {
    
    for (int i = 0; i < grid_x; i++) {
      for (int j = 0; j < grid_y; j++) {
	Pt pt = calibration_point(IntPt(i,j));
	for (unsigned int s = 0; s < screens.size(); s++) {
	  bool onscreen = screens[s].OnScreen(pt);
	  //std::cout << "onscreen " << s << " " << n << "   = " << onscreen << std::endl;
	  if (all_my_data[n].geometry_valid[getIndex(i,j)]) {	    
	    if (!onscreen) {
	      //std::cout << "extra " << pt << std::endl;
	    } else {
	      //std::cout << "good " << pt << std::endl;
	    }
	  } else {
	    if (onscreen) {
	      //std::cout << "crap missing " << pt << std::endl;
	      screen_camera[s][n] = false;	      
	    } else {
	      //std::cout << " - " << std::endl;
	    }
	    
	  }
	}
      }
    }
  }
  
  //std::cout << "screen cam 0 0 " << screen_camera[0][0] << std::endl;
  //std::cout << "screen cam 0 1 " << screen_camera[0][1] << std::endl;
  //std::cout << "screen cam 1 0 " << screen_camera[1][0] << std::endl;
  //std::cout << "screen cam 1 1 " << screen_camera[1][1] << std::endl;

  if (screen_camera[0][0] == 1 &&
      screen_camera[0][1] == 0 &&
      screen_camera[1][0] == 0 &&
      screen_camera[1][1] == 1) {
    screens[0].ip = all_my_data[0].ip;
    screens[1].ip = all_my_data[1].ip;
  } else {
    assert (screen_camera[0][0] == 0 &&
	    screen_camera[0][1] == 1 &&
	    screen_camera[1][0] == 1 &&
	    screen_camera[1][1] == 0);
    screens[0].ip = all_my_data[1].ip;
    screens[1].ip = all_my_data[0].ip;
  }

  //exit(0);

}
