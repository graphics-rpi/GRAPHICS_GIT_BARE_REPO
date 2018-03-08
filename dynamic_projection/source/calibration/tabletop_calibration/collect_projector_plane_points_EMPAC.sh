ARGS=5

if [ $# -ne "$ARGS" ]
then
  echo "Usage: `basename $0` projector_name hostname:xserver.xdisplay maskfile plane axis_to_fix" 
  exit $E_BADARGS
fi

./structured_light_cal $2 projector_images/$1\_$4\_test.dat camera_images/camera_calibration.dat $3 $1 100 

./fisheye_to_3d projector_images/$1\_$4\_test.dat Tsaidata/$1\_$4\_out.test $4 $5

echo "$5 fixed"


