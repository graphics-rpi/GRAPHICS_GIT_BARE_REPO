#!/bin/sh

for f in output/*; 
do
  cat $f >> combined.txt
  #printf "\n" >> combined.txt

done #hour
