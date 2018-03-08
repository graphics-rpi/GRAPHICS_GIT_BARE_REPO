#!/usr/bin/python
import subprocess
import os
import time
from remesh_tween_once import RemeshTweenOnce
from remesh_once import RemeshOnce
from lsv_once import LSVOnce
from lsvo_once import LSVOOnce
from system_variables import SystemVariables 
import sys
VISION_LOG="vision_log.txt"
HOME_DIR="/home/grfx/"
EXPOSURE=70
OUT_DIR="444"
from getch import _GetchUnix

getch = _GetchUnix()
SV=SystemVariables()

subprocess.call("mkdir "    + HOME_DIR + "tween", shell=True)
subprocess.call("mkdir "    + HOME_DIR + "images", shell=True)
subprocess.call("echo -1 >" + HOME_DIR + "images/counter.txt", shell=True)
subprocess.call("rm "       + HOME_DIR + "tween/stop", shell=True)
subprocess.call("mkdir "    + HOME_DIR + "logs/placeholder", shell=True)

os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/common")

#This is now done in render controller
subprocess.call("./grab_frame /ramdisk/out.ppm 300000 >> " + VISION_LOG + " 2>&1", shell=True)

os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/build")

subprocess.call("./table_top_detect -i /ramdisk/out.ppm -o /ramdisk/out.wall -find_architectural_design >> " + VISION_LOG + " 2>&1", shell=True)

os.chdir("/ramdisk")

subprocess.call("rm "+SV.dayfile_switch, shell=True)

subprocess.call("echo `cut -d' ' -f1-5 out.time` " "  `grep north out.wall | cut -d' ' -f2` > out.time",shell=True)

os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
print "remesh tweening"
RemeshTweenOnce()

print "remeshing"
RemeshOnce()

print "lsving"


os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
if len(sys.argv)>1:
  LSVOOnce(EXPOSURE,1)
else:
  LSVOOnce(EXPOSURE,0)
#time.sleep(200)

os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/tabletop/build")
print "mpiing"
subprocess.call("mpirun -hostfile ~/hosts --mca btl_tcp_if_include eth0  -np 9 combined  -noblending & ",shell=True)

#print "sleeping"
#time.sleep(7)
#os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/tabletop/build/")
#print "starting render controller"
#subprocess.call("./render_controller -noblending >/home/grfx/control_log.txt &", shell=True)
#subprocess.call("./render_controller >/home/grfx/control_log.txt &", shell=True)
X=1

while True:
  char=getch()
  if char == "q":
    subprocess.call("killall combined", shell=True)
    break

  elif char == "t":
    subprocess.call("gedit ~/Checkout/JOSH_EMPAC_2010/remesh_test/out.time", shell=True)


  elif char == "p":
    #subprocess.call("touch /ramdisk/play.day", shell=True)
    #subprocess.call("gedit /ramdisk/test.day", shell=True)
    #subprocess.call("cat /ramdisk/test.day >> /home/grfx/logs/" + OUT_DIR \
    #  + "/" + str(X) + ".time", shell=True)
    #os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    #LSVOnce(EXPOSURE)
    #sleep( 20)
    subprocess.call("rm /ramdisk/tween/stop" , shell=True)
    os.chdir(SV.image_path)
    subprocess.call("touch "+SV.dayfile_switch, shell=True)
    print "lsving"

    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    #LSVOnce(EXPOSURE)
    LSVOOnce(EXPOSURE)

    subprocess.call("rm /ramdisk/tween/stop", shell=True)
    subprocess.call("rm "+SV.dayfile_switch, shell=True)
    #subprocess.call("	/home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/play_a_day.pl /ramdisk/test.day .1", shell=True)
#      sudo rm $dayfile

  elif char == "i":
    EXPOSURE=EXPOSURE*2
    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    #LSVOnce(EXPOSURE)
    LSVOOnce(EXPOSURE)

    print EXPOSURE

  elif char == "o":
    EXPOSURE=EXPOSURE*.5
    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    #LSVOnce(EXPOSURE)
    LSVOOnce(EXPOSURE)



  # added by Andrew (Dolce) to toggle color calibration
  elif char == "c":
	  print "echo toggling color weights"
	  subprocess.call("touch /ramdisk/toggle_color_weights", shell=True)

  elif char == "b" or char == "f" :
    print "pressed b"
    subprocess.call("touch /ramdisk/tween/stop", shell=True)
    #subprocess.call("touch /ramdisk/tween/stop2", shell=True)
    #time.sleep(3)
    os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/common")

    #subprocess.call("./grab_frame /ramdisk/out.ppm >> " + VISION_LOG + " 2>&1", shell=True)
    while os.path.isfile("/ramdisk/tween/stop"):
      time.sleep(1)
    os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/build")

    subprocess.call("./table_top_detect -i /ramdisk/out.ppm -o /ramdisk/out.wall -find_architectural_design >> " + VISION_LOG + " 2>&1", shell=True)
				
    # update the time file with the new north direction
    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    
    print "remesh tweening"
    RemeshTweenOnce()

    print "remeshing"
    RemeshOnce()

    print "lsving"

    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    #LSVOnce(EXPOSURE)
    if char == "b":
      LSVOOnce(EXPOSURE,0)
    if char == "f":
      LSVOOnce(EXPOSURE,1)

    subprocess.call("rm /ramdisk/tween/stop2", shell=True)


#    subprocess.call("rm /ramdisk/tween/stop", shell=True)
"""
os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/build")

subprocess.call("./table_top_detect -i /ramdisk/out.ppm -o /ramdisk/out.wall -find_architectural_design >> " + VISION_LOG + " 2>&1", shell=True)

os.chdir("/ramdisk")

subprocess.call("rm "+SV.dayfile_switch, shell=True)

subprocess.call("echo `cut -d' ' -f1-5 out.time` " "  `grep north out.wall | cut -d' ' -f2` > out.time",shell=True)

os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
print "remesh tweening"
RemeshTweenOnce()

print "remeshing"
RemeshOnce()

print "lsving"

os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
LSVOOnce(EXPOSURE)

  else:
    print char

subprocess.call("ssh torch killall mpi_renderer", shell=True)
subprocess.call("ssh chainsaw killall mpi_renderer", shell=True)
subprocess.call("killall render_controller", shell=True)
"""
"""  elif char == "n":
    os.chdir( HOME_DIR + "/Checkout/JOSH_EMPAC_2010/remesh_test/")
    subprocess.call("./lsv_once.sh " + str(EXPOSURE) + " naive", shell=True)

  elif char == "l":
    os.chdir(HOME_DIR + "/Checkout/JOSH_EMPAC_2010/remesh_test/")
    subprocess.call("./lsv_once.sh " + str(EXPOSURE) + " lab", shell=True)

  elif char == "y":
    os.chdir( HOME_DIR + "/Checkout/JOSH_EMPAC_2010/remesh_test/")
    subprocess.call("./lsv_once.sh " + str(EXPOSURE) + " ypbpr", shell=True)

  elif char == "u":
    os.chdir(HOME_DIR + "/Checkout/JOSH_EMPAC_2010/remesh_test/")
    subprocess.call("./lsv_once.sh " + str(EXPOSURE), shell=True)
"""

