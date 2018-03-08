#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "please use ./run_no_projection NAME_OF_DIR"
  exit
fi

DIR=/largedrive/army_study_no_projection/$1
mkdir $DIR
x=0
while [ "$x" -lt "120" ];
do

  ./grab_frame out.ppm 
  cp out.ppm $DIR/iteration_$x.ppm
  ./grab_frame out.ppm 10000
  cp out.ppm $DIR/iteration_${x}_lightson.ppm
  sleep 15
  let x=x+1

done

