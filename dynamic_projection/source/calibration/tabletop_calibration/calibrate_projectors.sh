#!/bin/bash
#
# calibrate_projectors.sh : calibrate projectors {A,B,C,D} as cameras
#                           and produce opengl projection and modelview matrixes
#
# input: A_n.dat, B_n.dat, C_n.dat, D_n.dat : n = number of spacers under target
# configure spacer height below
#

PROJECTOR_IMAGE_DIR="/home/grfx/GIT_CHECKOUT/dynamic_projection/state/table_top_calibration/projector_images"
BUILD_DIR="../../../build"
CAMERA_CALIBRATION_DIR="/home/grfx/GIT_CHECKOUT/dynamic_projection/state/table_top_calibration/camera_images"
PROJECTOR_GLCAM_DIR="../../../state/table_top_calibration/projector_glcams"

# spacer height (meters) - the height of a standard VHS cassette tape
spacer=0.0246

# heights used for calibration (# of VHS video tapes)
#heights="0 1 2 3 4 5 6 7 8"
#heights="-1 0 1 2 3 4 5 6 7 8"
heights="0 4 8"

# projector resolution
image_width=1024
image_height=768

# opengl near/far plane distance
near=0.1
far=100.0

#
# use camera calibration to produce 3D points from detected target corners
#
for p in A B C D E F G H
do
    echo $p
    rm -f ${PROJECTOR_IMAGE_DIR}/projector_${p}_tsai.dat
    case "$p" in
        A)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
        B)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
        C)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
        D)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
    esac
    for h in  $heights
    do
	if [ "$h" == "-1" ]; then
	    height=0;
	else
            height="`echo "scale=3;$h*$spacer+0.0127"|bc`";
	fi
        echo ${p}_${h}.dat
        ${BUILD_DIR}/pixels_to_3d /home/grfx/GIT_CHECKOUT/dynamic_projection/source/calibration/tabletop_calibration/projector_images/${p}_${h}.dat \
                       $height $xoffset $yoffset $zoffset >> \
                       ${PROJECTOR_IMAGE_DIR}/projector_${p}_tsai.dat \
                       ${CAMERA_CALIBRATION_DIR}/camera_calibration.dat
    done
done

#
# run Tsai calibration code
#
for p in A B C D E F G H
do
    case "$p" in
        A)
            xoffset=0.0
            yoffset=0
            zoffset=0.0
            ;;
        B)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
        C)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
        D)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
	E)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
	F)
            xoffset=0
            yoffset=0
            zoffset=0
            ;;
    esac
    Tsai/ccal ${PROJECTOR_IMAGE_DIR}/projector_${p}_tsai.dat \
              $image_width $image_height \
              ${PROJECTOR_IMAGE_DIR}/projector_${p}_calibration.dat \
              $xoffset $yoffset $zoffset
    ${BUILD_DIR}/make_opengl_camera ${PROJECTOR_IMAGE_DIR}/projector_${p}_calibration.dat \
                         $near $far ${PROJECTOR_GLCAM_DIR}/projector_${p}.glcam
done


#
# copy calibration data to archdisplay directory
#
#cp ${PROJECTOR_IMAGE_DIR}/projector*.glcam ../archdisplay
