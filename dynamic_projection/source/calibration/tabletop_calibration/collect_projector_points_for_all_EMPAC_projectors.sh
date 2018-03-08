
ARGS=3

if [ $# -ne "$ARGS" ]
then
  echo "Usage: `basename $0` maskfile plane axis_to_fix" 
  exit $E_BADARGS
fi

./collect_projector_plane_points_EMPAC.sh left chainsaw:1.1 $1 $2 $3
./collect_projector_plane_points_EMPAC.sh center chainsaw:1.0 $1 $2 $3
./collect_projector_plane_points_EMPAC.sh right chainsaw:0.1 $1 $2 $3
./collect_projector_plane_points_EMPAC.sh top torch:1.0 $1 $2 $3
./collect_projector_plane_points_EMPAC.sh back torch:0.1 $1 $2 $3


