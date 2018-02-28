pushd build
 ./lsvo -i ../example_input/foo.obj \
          -t 3000 -patches 500 -offline -remeshend \
          -exp 70 \
          -t 12 0 -date 1 1 \
          -d=512x512 #-N \
#          -dumpOrthos 21 \
#          ../example_input/surface_camera_floor_0_0.glcam \
#          ../example_input/surface_camera_WALL0_0.glcam \
#          ../example_input/surface_camera_WALL0_2.glcam \
#          ../example_input/surface_camera_WALL1_0.glcam \
#          ../example_input/surface_camera_WALL1_2.glcam \
#          ../example_input/surface_camera_WALL2_0.glcam \
#          ../example_input/surface_camera_WALL2_2.glcam \
#          ../example_input/surface_camera_WALL3_0.glcam \
#          ../example_input/surface_camera_WALL3_2.glcam \
#          ../example_input/surface_camera_WALL4_0.glcam \
#          ../example_input/surface_camera_WALL4_2.glcam \
#          ../example_input/surface_camera_WALL5_0.glcam \
#          ../example_input/surface_camera_WALL5_2.glcam \
#          ../example_input/surface_camera_WALL6_0.glcam \
#          ../example_input/surface_camera_WALL6_2.glcam \
#          ../example_input/surface_camera_WALL7_0.glcam \
#          ../example_input/surface_camera_WALL7_2.glcam \
#          ../example_input/surface_camera_WALL8_0.glcam \
#          ../example_input/surface_camera_WALL8_2.glcam \
#          ../example_input/surface_camera_WALL9_0.glcam \
#          ../example_input/surface_camera_WALL9_2.glcam \
#          #-dumpPeople ../example_input/out.wall -N -noqt
popd

