#!/bin/sh

MONTH=3
DAY=21
HOUR=8
MINUTES=0
for month in {1..12}
do
  for hour in 8 
  do  
    pushd /home/nasmaj/GIT_CHECKOUT/lsvo/build/
    "Processing groundtruth file..";
    echo $month $hour 
    ./patchgetter 7 -i /home/nasmaj/GIT_CHECKOUT/remesher/examples/user_study_3_wallfiles/groundtruth.wall -offline -t 3000 -patches 500 -remeshend -t $hour $MINUTES -exp 30 -date $month $DAY -o /home/nasmaj/GIT_CHECKOUT/remesher/examples/user_study_3_wallfiles/output/${hour}_${month}_groundtruth.txt >& /dev/null
    popd
  
    Dirlist=$(find . -maxdepth 1 -mindepth 1 -type d )
    for direc in original renovations ; do
      direc2=${direc##*/}
      pushd $direc
      echo "direc" $direc
      for f in N6.wall ; #I_A2.wall I_N7.wall 
      do
#         echo "blah"
        pushd /home/nasmaj/GIT_CHECKOUT/lsvo/build/
#        echo "Processing $f file.."; 
#        echo "Processing ./patchgetter 7 -i /home/nasmaj/GIT_CHECKOUT/remesher/examples/user_study_3_wallfiles/$direc/$f file.."; 
        ./patchgetter 7 -i /home/nasmaj/GIT_CHECKOUT/remesher/examples/user_study_3_wallfiles/$direc/$f -offline -t 3000 -patches 500 -remeshend -t $hour $MINUTES -exp 30 -date $month $DAY -o /home/nasmaj/GIT_CHECKOUT/remesher/examples/user_study_3_wallfiles/output/${hour}_${month}_$f.txt >& /dev/null
        popd
      done #file
  
      popd
    done #direc
  done #min
done #hour
