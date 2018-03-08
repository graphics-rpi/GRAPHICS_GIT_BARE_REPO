#!/bin/sh
MACHINE=`uname -s`
if [ "$MACHINE" == "Darwin" ]; 
then 
g++ -shared -o libgraphvisualization.dylib.1.0.0 -Wl,-dylib_install_name,libgraphvisualization.dylib.1 *.o ../paint/button.o ../../ir_tracking/ir_data_point.o ../paint/text.o ../../calibration/planar_interpolation_calibration/tracker.o lib_dummy_funcs.cpp -fPIC ../../calibration/planar_interpolation_calibration/munkres.o ../../calibration/planar_interpolation_calibration/planar_calibration.o ../../../../remesher/src/utils.o ./QuadTree/bbox.o -framework GLUT -framework OpenGL
else
g++ -shared -o libgraphvisualization.so.1.0.0 -Wl,-soname,libgraphvisualization.so.1 *.o ../paint/button.o ../../ir_tracking/ir_data_point.o ../paint/text.o ../../calibration/planar_interpolation_calibration/tracker.o lib_dummy_funcs.cpp -fPIC ../../calibration/planar_interpolation_calibration/munkres.o ../../calibration/planar_interpolation_calibration/planar_calibration.o ../../../../remesher/src/utils.o ./QuadTree/bbox.o 
fi




