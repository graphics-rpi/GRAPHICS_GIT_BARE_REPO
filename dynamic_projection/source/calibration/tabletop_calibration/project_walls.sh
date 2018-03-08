#!/bin/bash
IMAGE_DIR=/ramdisk

./projector_test $1 projector_images/projector_A.glcam \
                    $IMAGE_DIR/projectorA.ppm
kill -s 37 `cat ../archdisplay/PID_A.txt`

./projector_test $1 projector_images/projector_B.glcam \
                    $IMAGE_DIR/projectorB.ppm
kill -s 37 `cat ../archdisplay/PID_B.txt`

./projector_test $1 projector_images/projector_C.glcam \
                    $IMAGE_DIR/projectorC.ppm
kill -s 37 `cat ../archdisplay/PID_C.txt`

./projector_test $1 projector_images/projector_D.glcam \
                    $IMAGE_DIR/projectorD.ppm
kill -s 37 `cat ../archdisplay/PID_D.txt`

