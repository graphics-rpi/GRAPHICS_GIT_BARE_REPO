#!/bin/bash
#
# calibrate_table_top.sh
#
# estimate the table top surface for calibration
#
#/home/grfx/Checkout/dynamic_projection/source/applications/army/grab_frame table_cal_distorted.ppm
CAMERA_IMAGE_DIR="../../../build/camera_images"
BUILD_DIR="../../../build"
/home/grfx/GIT_CHECKOUT/dynamic_projection/build/grab_frame_new ${CAMERA_IMAGE_DIR}/table_cal_distorted.ppm 200000
#/home/grfx/GIT_CHECKOUT/dynamic_projection/build/grab_single_frame_Vimba ${CAMERA_IMAGE_DIR}/table_cal_distorted.ppm 100000
${BUILD_DIR}/undistort_image  ${CAMERA_IMAGE_DIR}/table_cal_distorted.ppm \
                   ../../../state/table_top_calibration/camera_calibration.dat \
                   ${CAMERA_IMAGE_DIR}/table_cal.ppm
matlab -r calibrate_table_top
