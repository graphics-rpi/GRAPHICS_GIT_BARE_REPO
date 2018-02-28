#!/bin/sh

for user in A3 #A1 A2 A3 A4 A5 A6 N2 N3 N4 N6
do
  ./process_wall.sh $user.wall 2000 I_$user.obj I_${user}_chris.vf
  ./process_wall.sh $user.wall 2000 I_$user.obj I_${user}_dark.vf
done
