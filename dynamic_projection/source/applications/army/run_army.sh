#!/bin/bash

REMESH_LOG=remesh_log.txt
VISION_LOG=vision_log.txt
REMESH_PATH=~/GIT_CHECKOUT/remesher/build/remesh

glcam0=/home/grfx/Checkout/archdisplay/projector_A.glcam
glcam1=/home/grfx/Checkout/archdisplay/projector_B.glcam
glcam2=/home/grfx/Checkout/archdisplay/projector_C.glcam
glcam3=/home/grfx/Checkout/archdisplay/projector_D.glcam
glcam4=/home/grfx/Checkout/archdisplay/projector_E.glcam
glcam5=/home/grfx/Checkout/archdisplay/projector_F.glcam

#cp ~/app_data/floor.ppm ~/images/surface_camera_wall.ppm
cp ~/building_facade_cropped.ppm ~/images/surface_camera_wall.ppm
#cp ~/black.ppm ~/images/surface_camera_platform.ppm
cp ~/bricks.ppm ~/images/surface_camera_platform.ppm
cp ~/bricks.ppm ~/images/surface_camera_ramp.ppm
cp ~/black.ppm ~/images/surface_camera_black.ppm
#cp ~/app_data/none.ppm ~/images/surface_camera_floor.ppm

mkdir /home/grfx/tween
mkdir /home/grfx/images
echo 1 >/home/grfx/images/counter.txt
rm /home/grfx/tween/stop
DIR=/largedrive/army_study/$1
mkdir $DIR

if [ "$#" -ne 1 ]; then
  echo "please use ./run_army NAME_OF_DIR"
  exit
fi

x=1
old_tty_setting=$(stty -g) # Save old terminal settings.
stty -icanon -echo # Disable canonical mode and echo

# blank_projectors
# run vision module
./grab_frame out.ppm >> $VISION_LOG 2>&1
cp out.ppm $DIR/iteration_0_state_0.ppm

pushd /home/grfx/GIT_CHECKOUT/dynamic_projection/source/table_top_detection/
./table_top_detect -i ../applications/army/out.ppm -find_army_terrain -o ../applications/army/terrain.army
./table_top_detect -i ../applications/army/out.ppm -find_army_soldiers -o ../applications/army/temp.army >> $VISION_LOG
popd

  mv temp.army soldiers.army
#./find_terrain out.ppm terrain.army >> $VISION_LOG 2>&1
#./find_soldiers out.ppm soldiers.army >> $VISION_LOG 2>&1

echo REMESHING TERRAIN
./remesh_geometry.sh
#cd ~/Checkout/JOSH_EMPAC_2010/remesh_test

echo REMESHING HEIGHTFIELD
${REMESH_PATH} -i terrain.army -army -width 512 -height 512 -offline

cp army_floorplan.ppm $DIR

echo STARTING ARMY APP
rm soldiers.army
touch soldiers.army
./army --log-images $DIR > army_debug.txt & #-i army_floorplan.ppm -o ~/tween/surface_camera_floor.ppm &

# copy over initial floor texture
# cp army.ppm ~/images/surface_camera_floor.ppm

# start remote renderers
cd ~/Checkout/rendering_test_with_sockets/
mpirun -hostfile ~/hosts --mca btl_tcp_if_include eth0  -np 6 mpi_renderer > render_debug.txt  &

# initial sleep to let the mpi processes start
sleep 7

# start the render controller which talks with the mpi processes
cd ~/Checkout/JOSH_EMPAC_2010/sockets_EOTT/
./render_controller -army  > controller_debug.txt & #-noblending & #-multidisplay

#different keys allow different interactions
keys=$(dd bs=1 count=1 2> /dev/null)
let "subStep=0"
while [ "$keys" != "q" ]
do
  if  [ "$keys" == "b" ]; then
	  
    echo "pressed b"
    
    touch /ramdisk/tween/stop # pause the render controller
    sleep 2


#    while [ ! -e "/ramdisk/stopped"]
#    do
#      sleep .1
#    done

    echo "grabbing frame"
    cd ~/GIT_CHECKOUT/dynamic_projection/source/applications/army
	  ./grab_frame out.ppm >> $VISION_LOG # 2>&1       

	  #cat /ramdisk/out.time >> /home/grfx/logs/$2/$x.time
    if [ -e "state.txt" ]
    then
	subStep=0
      state=`cat state.txt`
      rm state.txt
    fi      	
    
    iteration=`cat round.txt`
    cp out.ppm $DIR/iteration_${iteration}_state_${state}_${subStep}.ppm
    let "subStep=$subStep+1"
	  date > $DIR/time_${iteration}_state_${state}_${subStep}.txt
    pushd /home/grfx/GIT_CHECKOUT/dynamic_projection/source/table_top_detection/
	    # find soldier positions
    echo "removing stopfile"
    rm /ramdisk/tween/stop	# unpause the render controller

    echo "running tabletop detect"
      ./table_top_detect -i ../applications/army/out.ppm -find_army_soldiers -o ../applications/army/temp.army >> $VISION_LOG
    echo "ending tabletop detect"
    popd
    mv temp.army soldiers.army
    #use this if you need to take images while running program
    #./take_projection_pix.sh $DIR/projected_${iteration}_state_${state}_${subStep}
    
    
  #./find_soldiers out.ppm soldiers.army >> $VISION_LOG # 2>&1	
  fi

  if  [ "$keys" == "~" ]; then
	  echo "pressed F5"

	  # send "OK" message to army app
	  touch army_OK

  fi

  echo you pressed $keys
  keys=$(dd bs=1 count=1 2> /dev/null)
done

echo you pressed $keys

ssh torch killall mpi_renderer
ssh chainsaw killall mpi_renderer
killall render_controller
killall army
stty "$old_tty_setting" # Restore old terminal settings.
