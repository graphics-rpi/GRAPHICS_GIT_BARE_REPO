ARGS=3

if [ $# -ne "$ARGS" ]
then
  echo "Usage: `basename $0` projector name hostname:xserver.xdisplay maskfile plane" 
  exit $E_BADARGS
fi

./structured_light_cal $2 projector_images/$1\_$3\_test.dat camera_images/camera_calibration.dat /home/grfx/Downloads/2_walls_mask.pbm $1 100 

./fisheye_to_3d projector_images/$1\_$3\_test.dat $1\_$3\_out.test $(($3*.3048)) x

echo $(($3*.3048))
