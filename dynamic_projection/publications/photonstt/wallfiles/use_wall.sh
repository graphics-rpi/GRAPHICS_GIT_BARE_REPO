#!/bin/bash  

if [ $# -ne 1 ]
then
  echo "please use ./use_wall wallfile.wall"
else
  cp $1 /ramdisk/out.wall
fi
