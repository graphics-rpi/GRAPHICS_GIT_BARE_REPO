#! /bin/bash
#name= $1
echo $1
if [ $# -ne 3 ]; then
   printf "please use ./generate_image obj_file north_file viewpoint #"
else

  
  obj_name=$1
  north_name=$2
  vf_name=$3
  #echo $dir
  #echo $obj_name
  #echo $north_name

  north=`cut -d' ' -f1 geometry/$north_name`
  #selection=`cut -d' '-d1 selection`
  #echo "selection" $selection
  
  #awk '{print $1" " $2" "$3" "$4" "$5" "}' ~/Checkout/JOSH_EMPAC_2010/remesh_test/out.time /ramdisk/tween/foo.north |cat| tr -d "\n" >/ramdisk/out.time
 # awk '{print $1" " $2" "$3" "$4" "$5" "}' ../out.time geometry/$north_name |cat| tr -d "\n" ../out.time
#  if [ $3 -eq 1 ]; then
#    rm ${dir}*.ppm
    awk '{print $1" " $2" "$3" "$4" "$5" "}' march.time geometry/$north_name |cat |tr -d "\n" >test.txt
    mv test.txt out.time
    ~/Checkout/archlight3/bin/lsv -i $obj_name -tf out.time  -vf $vf_name  -drawFillIn -exposure .1 >& /dev/null
    mv snap00001.ppm images/${1}_march.ppm
    echo "march"

    
    
    awk '{print $1" " $2" "$3" "$4" "$5" "}' december.time geometry/$north_name |cat |tr -d "\n" >test.txt
    mv test.txt out.time
    ~/Checkout/archlight3/bin/lsv -i $obj_name -tf out.time  -vf $vf_name  -drawFillIn -exposure .1 >& /dev/null
    mv snap00001.ppm images/${1}_december.ppm
    echo "december"
    
#  fi

  




  #gthumb $dir/*.ppm
fi
