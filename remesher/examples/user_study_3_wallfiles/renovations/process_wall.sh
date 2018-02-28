#!/bin/sh

~/GIT_CHECKOUT/remesher/build/remesh -i $1 -t $2 -o geometry/$3 -offline
dir=geometry
wallfile=$1
trinum=$2
objfile=$3
viewfile=$4
#pushd $dir
  north_name=${objfile%.obj}.north
  north=`cut -d' ' -f1 geometry/$north_name`


  #selection=`cut -d' '-d1 selection`
  #echo "selection" $selection
  
  #awk '{print $1" " $2" "$3" "$4" "$5" "}' ~/Checkout/JOSH_EMPAC_2010/remesh_test/out.time /ramdisk/tween/foo.north |cat| tr -d "\n" >/ramdisk/out.time
 # awk '{print $1" " $2" "$3" "$4" "$5" "}' ../out.time geometry/$north_name |cat| tr -d "\n" ../out.time
#  if [ $3 -eq 1 ]; then
    awk '{print $1" " $2" "$3" "$4" "$5" "}' ../original/march.time geometry/$north_name |cat |tr -d "\n" >test.txt
    mv test.txt ../out.time
    ~/Checkout/archlight3/bin/lsv -i geometry/$objfile -tf ../out.time  -vf ../original/viewfiles/$viewfile  -drawFillIn -exposure .02 >& /dev/null
mv snap00001.ppm images/$1_march_$viewfile.ppm
    
    awk '{print $1" " $2" "$3" "$4" "$5" "}' ../original/december.time geometry/$north_name |cat |tr -d "\n" >test.txt
    mv test.txt ../out.time
    ~/Checkout/archlight3/bin/lsv -i geometry/$objfile -tf ../out.time  -vf ../original/viewfiles/$viewfile  -drawFillIn -exposure .02 >& /dev/null
mv snap00001.ppm images/$1_dec_$viewfile.ppm
    
#popd
