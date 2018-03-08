ARGS=2

if [ $# -ne "$ARGS" ]
then
  echo "Usage: `basename $0` projector name hostname:xserver.xdisplay"
  exit $E_BADARGS
fi

./structured_light_cal $2 projector_images/$1\_test.dat camera_images/camera_calibration.dat /home/grfx/Downloads/floor\ \(2\).pbm $1 99 

#./structured_light_cal $2 projector_images/$1\_test_screen.dat camera_images/camera_calibration.dat /home/grfx/Downloads/screen.pgm $1 100 

./structured_light_cal $2 projector_images/$1\_test_screen.dat camera_images/camera_calibration.dat /home/grfx/Downloads/2_walls_mask.pbm $1 100 

#floor plane Z
./fisheye_to_3d projector_images/$1\_test.dat $1\_out.test 0 z

#screen plane -Y
./fisheye_to_3d projector_images/$1\_test_screen.dat $1\_out_screen.test 5.2578 x

cat $1\_out.test $1\_out_screen.test > $1\_Tsai.in

Tsai/ccal $1\_Tsai.in 1024 768 projector_images/$1\_post_ccal_center_test.dat 0 0 0
#Tsai/ccal out.test 1024 768 projector_images/post_ccal_center_test.dat 0 0 0  

./make_opengl_camera projector_images/$1\_post_ccal_center_test.dat \
                         .1 100 $1\_test_out.glcam

# ~/Checkout/remesher/remesh -i ~/Checkout/archdisplay/nothing.wall -o foo.led -tweening -blending_subdivision 20 -no_remesh -use_locked_output_directory ~/tween -surface_cameras_fixed_size 512 -floor_cameras_tiled 1 -offline -p 1 /home/grfx/Checkout/camera_calibration/test_out.glcam




