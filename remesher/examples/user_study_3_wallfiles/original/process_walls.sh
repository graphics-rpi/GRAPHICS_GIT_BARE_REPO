#!/bin/sh

for user in A1 A2 A4 A5 A6 N1 N2 N3 N4 N5 N6 N7
do
  ./process_wall.sh I_$user.wall 2000 I_$user.obj I_${user}_chris.vf
  ./process_wall.sh I_$user.wall 2000 I_$user.obj I_${user}_dark.vf
done
