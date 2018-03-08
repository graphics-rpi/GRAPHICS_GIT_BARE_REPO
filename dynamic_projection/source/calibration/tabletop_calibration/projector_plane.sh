#!/bin/bash
#
# generate projector calibration points for a single plane height
#
# usage:
#
# projector_plane.sh <# of spacers>
#
BUILD_DIR="../../../build"
PROJECTOR_IMAGE_DIR="../../../state/table_top_calibration/projector_images"
CAMERA_IMAGE_DIR="/home/grfx/GIT_CHECKOUT/dynamic_projection/state/table_top_calibration"

${BUILD_DIR}/structured_light_cal chainsaw:1.0 projector_images/A_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/A_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal chainsaw:1.1 projector_images/B_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/B_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal chainsaw:0.0 projector_images/C_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/C_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal torch:1.0 projector_images/D_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/D_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal torch:1.1 projector_images/E_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/E_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal torch:0.0 projector_images/F_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/F_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal chainsaw:0.1 projector_images/G_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/G_$1_test.pgm 
sleep 10

${BUILD_DIR}/structured_light_cal torch:0.1 projector_images/H_$1.dat \
                       ${CAMERA_IMAGE_DIR}/camera_calibration.dat
mv test.pgm ${PROJECTOR_IMAGE_DIR}/H_$1_test.pgm 

