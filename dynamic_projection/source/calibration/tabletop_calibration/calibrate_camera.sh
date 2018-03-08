#!/bin/bash
#
# calibrate_camera.sh : calibrate camera from target images
#
# ~40 images of target required, named image[n].ppm
# world.ppm should be an image of target flat on table surface

#
# USAGE: calibrate_camera <directory> <type>
#  directory: contains input images
#  type = {camera, slr}

BUILD_PATH="../../../build"

if [ $# -ne 2 ]
then
    echo "Usage: calibrate_camera.sh <directory> <slr|camera>"
    exit -1
fi

#
# OpenGL near, far plane distance
#
near=0.01
far=100.0

directory="$1"
type="$2"
width=`pamfile $directory/world.ppm | \
       perl -e '(($w,$h) = (<> =~ /(\d+) by (\d+)/)) && print "$w";'`
height=`pamfile $directory/world.ppm | \
        perl -e '(($w,$h) = (<> =~ /(\d+) by (\d+)/)) && print "$h";'`

#
# add flag for just updating extrinsics to control this if
#
if [  true ]; then

#
# find corners in all camera calibration images, calibrate camera
#
rm -f $directory/data*.txt
j=1
for i in $directory/image*.ppm
do
  n=`printf "%03d" $j`;
  echo -n $i data$n.txt ": "
  if $BUILD_PATH/process_2d_target $i $type > $directory/data$n.txt
  then
    let "j++"
  fi
done
fi

#
# process world coordinate system image
#
rm -f $directory/world.txt
$BUILD_PATH/process_2d_target $directory/world.ppm $type > $directory/world.txt

#
# create the model file
#
rm -f $directory/model.txt
./make_model.pl $type > $directory/model.txt

#
# run Zhang's calibration code
#
cd Zhang
rm -f calibration.txt
ls $directory/data*.txt $directory/world.txt | \
    xargs wine EasyCalib.exe -distortion \
                             -model $directory/model.txt \
                             -result $directory/calibration.txt 
cd ..

#
# parse output of Zhang's code to create camera matrixes
#
$BUILD_PATH/parse_zhang_output $directory/calibration.txt \
                     $width $height \
                     $directory/camera_calibration.dat


#
# create an opengl camera with calibration
#
$BUILD_PATH/make_opengl_camera $directory/camera_calibration.dat $near $far \
                     $directory/camera_calibration.glcam

#
# copy the calibration data to archdisplay
#
if [ "$type" == "camera" ]; then
    cp $directory/camera_calibration.dat $directory/.. #/state/table_top_calibration
    cp $directory/camera_calibration.glcam $directory/.. #../state/table_top_calibration
#else 
#    cp $directory/camera_calibration.dat ../archdisplay/slr_calibration.dat
#    cp $directory/camera_calibration.glcam ../archdisplay/slr_calibration.glcam
fi
