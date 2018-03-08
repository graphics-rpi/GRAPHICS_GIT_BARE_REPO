#!/usr/bin/python
import os, subprocess, time, shutil
from glob import glob
from system_variables import SystemVariables 
class LSVOnce(object):
  def __init__(self, exposure):
    SV=SystemVariables()
    os.chdir(SV.image_path)

    print "start lsvo"

    f = open ( "/home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/out.time" )
    f2 = open ( "/ramdisk/tween/foo.north" )
    words = f.readline().split()
    words2 = f2.readline().split()
    f3 = open ("/ramdisk/out.time", 'w')
    f3.write(" ".join(words[0:5] + [words2[0]]))
    f.close()
    f2.close()
    f3.close()

    print "after initial files"

        

    for glcam_name in glob(SV.image_path+"*_0.glcam"):
      os.remove(glcam_name)

    # remove ppm files without a corresponding glcam
    for ppm_name in glob(SV.image_path+"*.ppm"):
      glcam_name=ppm_name[:-12]+".glcam"
      if not os.path.exists(glcam_name):
        os.remove(ppm_name)
    if os.path.exists(SV.dayfile_switch):
      os.chdir(SV.slow_loc)
#      if os.path.exists(stopfile):
#        os.remove(stopfile)
#      /home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/play_a_day.pl /ramdisk/test.day $1  
      exec_string=" ".join([SV.play_a_day, SV.dayfile_loc, str(exposure)])     
      print "about to play day"
      subprocess.call(exec_string,shell=True)
      print "done playign a day \n"
    else: 
#not playing a day
      f4=open(SV.counter_loc)
      start_counter=int(f4.readline().split()[0])
      f4.close()

      glcam_string=" ".join(glob(SV.slow_loc+"*.glcam"))


      exec_string=[
        SV.lsv_loc,
        "-i",  SV.slow_loc+SV.slow_obj_name,
        "-ml",  SV.slow_loc+SV.slow_obj_name[:-4]+".lsvmtl",
        "-exp", ".10",
        "-tf", SV.timefile_loc,
        "-size", SV.camera_size, SV.camera_size,
        "-screen", "../window_screens/PagodaScreen_tileable.png",
        "-hidegui", "-vm"] +glcam_string.split()
#-wire
      print exec_string
      print " ".join(exec_string)
      #sleep 10
      NEWPROC=subprocess.Popen(args=exec_string)
      while True:
          #print "looping\n"

          time.sleep(.02)
          f4=open(SV.counter_loc)
          counter=int(f4.readline().split()[0])
          f4.close()
          if counter!=start_counter:
            break
      print "PID "+str(NEWPROC.pid)
      os.kill(NEWPROC.pid,9)
      NEWPROC.terminate()

      for ppm_name in glob(SV.slow_loc+"*.ppm"):
        shutil.move(ppm_name,SV.image_path)
        

if __name__ == "__main__":
  LSVnce(.1)

#awk '{print $1" " $2" "$3" "$4" "$5" "}' /ramdisk/out.time /ramdisk/slow/foo.north |cat| tr -d "\n" >/ramdisk/#out.time
#awk '{print $1" " $2" "$3" "$4" "$5" "}' ~/Checkout/JOSH_EMPAC_2010/remesh_test/out.time ~/tween/fixed.north |cat| tr -d "\n" >/ramdisk/out.time
    #rm *_0.glcam
 #  pid=$!
#    for ppm_name in /ramdisk/images/*.ppm
#    do
#       remove path to ppm
#      glcam_name=${ppm_name##*/}
#       remove _texture.ppm from name and add glcam
#      glcam_name=${glcam_name%_texture.ppm}.glcam
#      echo $glcam_name
#      if [ ! -f $glcam_name ]; then
#        rm -f $ppm_name
#      fi
#    done

    #if .day file exists, we are in play a day mode
#    if [ -f $dayfile ]; then
#   		rm /ramdisk/tween/stop
#   		cd /ramdisk/images
# 			/home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/play_a_day.pl /ramdisk/test.day $1
#      sudo rm $dayfile
#    else

# 		  count=`cat /ramdisk/images/counter.txt`
#		  string=`ls *.glcam`
#     	cp $timefile /home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/time.bak
      #if [ $2 -eq 2 ]; then
     	  	#	string="$lsv -i ~/slow/foo.obj -ml ~/slow/foo.lsvmtl -il -exposure $1 -tf $timefile -size 1024 1024  -hidegui -vm $string &"#-wire
      #else
#        if [ "$2" = "naive" ]; then
#          echo naive
#				  string="$lsv -i ~/slow/foo.obj -ml /ramdisk/slow/foo.lsvmtl -exposure $1 -tf $timefile -size $resolution $resolution -hidegui -il naive -vm $string &"#-wire
#        elif [ "$2" = "lab" ]; then
#          echo lab
#					string="$lsv -i ~/slow/foo.obj -ml ~/slow/foo.lsvmtl -exposure $1 -tf $timefile -size $resolution $resolution -hidegui -il lab_cuda -alpha 0.95 -vm $string &"#-wire
#				elif [ "$2" = "ypbpr" ]; then
#          echo ypbpr
#					string="$lsv -i ~/slow/foo.obj -ml ~/slow/foo.lsvmtl -exposure $1 -tf $timefile -size $resolution $resolution -hidegui -il -vm $string &"#-wire
#				else
#					echo uncompensated
#					string="$lsv -i /ramdisk/slow/foo.obj -ml /ramdisk/slow/foo.lsvmtl -exposure $1 -tf $timefile -size $resolution $resolution -hidegui -vm $string &"#-wire
#				fi
#        	echo $string
#        	eval $string

#       		PID=$!
#	      	while [ $count -eq `cat $counter` ]
#        	do
             
#          	 sleep .02
            # echo sleeping $count `cat $counter`


#        	done


#        	echo $PID
#        	kill -9 $PID
					#ls *.glcam | xargs $lsv -i slow/foo.obj -ml slow/foo.lsvmtl -exposure 0.1 -tf out.time -size 256 256 -wire -vm  
#    fi

#echo "end lsv o"
#ls *.glcam | xargs lsv 
