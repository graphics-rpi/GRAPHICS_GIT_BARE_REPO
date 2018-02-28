#!/bin/bash 


pushd build
./lsvo -i ~/Desktop/test/out.obj -t 3000 -patches 500 -offline -remeshend -t 12 0 -exp 30 -date 1 1 -noqt -d=512x512 -N -dumpOrthos -1 ~/Desktop/test/surface_camera_floor_0_0.glcam
#-ortho ~/lsvo_tmp/slow/surface_camera_WALL1_0.glcam 
#-d=512x512 -dump dump.ppm

#./lsvo -i ../tabletop_models/lsvo_tmp/slow/foo.obj -no_remesh -t 2000 -patches 100 -offline -remeshend -t 12 0 -exp 30 -date 1 1 -greyscale -toobright .7 -toodim .3
#./lsvo -i ../tabletop_models/lsvo_tmp/slow/foo.obj -no_remesh  -offline -remeshend -t 12 0 -exp 30 -date 1 1 -greyscale -toobright .7 -toodim .3

#popd
