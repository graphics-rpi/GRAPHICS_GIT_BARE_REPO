#ifndef _TABLE_H_
#define _TABLE_H_


#include <cstdlib>

class Table {
public:
  Table(CalibratedCamera *camera, const char *filename){
    this->camera = camera;
    load_file(filename);
  }

  Image<int> getBounds(){
    //    const double table_edge_border = 5.0;
    const double table_edge_border = 5.0;
    //const double table_edge_border =-5; // 0.0;
    //const double table_edge_border = 0; //5.0;


    int num_rows = camera->getRows();
    int num_cols = camera->getCols();

    double check_r = fabs((center_row - num_rows/2.0) / double(num_rows));
    double check_c = fabs((center_col - num_cols/2.0) / double(num_cols));

    double check_rad = fabs(std::min(num_rows,num_cols)/2.0 - radius_pixels) / double(radius_pixels);

    if (check_r > 0.2 ||
        check_c > 0.2 ||
        check_rad > 0.2) {
      std::cerr << "------------------------------------------------" << std::endl;
      std::cerr << "TABLE CALIBRATION LOOKS WRONG FOR THIS IMAGE????" << std::endl;
      std::cerr << "------------------------------------------------" << std::endl;
      center_row = num_rows/2.0;
      center_col = num_cols/2.0;
      radius_pixels = std::min(num_rows,num_cols)/2.0;
    }

    double rad = radius_pixels - table_edge_border;
    
    Image<int> bounds(num_rows, 2);
    double r2 = rad * rad;
    for (int row=0; row<num_rows; row++){
      double det = r2 - (row-center_row)*(row-center_row);
      if (det > 0.){
	int w =  int(sqrt(det)); //r2 - (row-center_row)*(row-center_row)));
	bounds(row, 0) = std::max(int(center_col - w),0);
	bounds(row, 1) = std::min(int(center_col + w),num_cols-1);
      } else {
	bounds(row, 0) = 0;
	bounds(row, 1) = 0;
      }
    }
    return bounds;
  }

  int getCenterRow(){
    return center_row;
  }

  int getCenterCol(){
    return center_col;
  }

  int getRadiusPixels(){
    return radius_pixels;
  }

  // return the table center in 3D world coordinates
  v3d getCenterWorld(){
    return camera->PointFromPixel(center_row, center_col, 0.);
  }

  // return the radius in meters
  double getRadius(){
    v3d c = getCenterWorld();
    // find a point on the edge of the circle
    v3d p = camera->PointFromPixel(center_row+radius_pixels, center_col, 0.);
    // distance between these points is the table radius
    return (p - c).length();
  }

  Image<byte>* getMaskImage(){
    Image<byte> *mask = new Image<byte>(camera->getRows(), camera->getCols());
    double rad2 = radius_pixels * radius_pixels;
    for (int row=0; row<mask->getRows(); row++){
      for (int col=0; col<mask->getCols(); col++){
	double r = (row - center_row) * (row - center_row) +
	  (col - center_col) * (col - center_col);
	if (r < rad2) {
	  (*mask)(row, col) = 255;
	} else {
	  (*mask)(row, col) = 0;
	}
      }
    }
    return mask;
  }

private:
  CalibratedCamera *camera;
  double center_row, center_col;
  double radius_pixels;
  int min_row, max_row;
  int min_col, max_col;

  void load_file(const char *filename){
    FILE *fp = fopen(filename, "rt");
    if (NULL == fp){
      fprintf(stderr, "Unable to open file %s\n", filename);
      throw -1; //exit(-1);
    }
    
    char comment[1024];
    char *ret = fgets(comment, 1024, fp);
    if (ret != comment) { throw -1; }
    ret = fgets(comment, 1024, fp);
    if (ret != comment) { throw -1; }
    int num_read = fscanf(fp, "%lg %lg", &center_row, &center_col);
    if (num_read != 2) { throw -1; }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    ret = fgets(comment, 1024, fp);
    if (ret != comment) { throw -1; }

    num_read = fscanf(fp, "%lg", &radius_pixels);
    if (num_read != 1) { throw -1; }

    for (int c=' ';c != EOF && c != '#'; c=getc(fp)); 
    ret = fgets(comment, 1024, fp);
    if (ret != comment) { throw -1; }

    num_read = fscanf(fp, "%d %d %d %d", &min_row, &max_row, &min_col, &max_col);
    if (num_read != 4) { throw -1; }

    fclose(fp);
  }
};

#endif
