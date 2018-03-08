#!/usr/local/bin/python2.7
import subprocess
import os
import time
from lsv_once import LSVOnce
from lsvo_once import LSVOOnce
from system_variables import SystemVariables
from glob import glob
import sys
import shutil
import argparse
from multiprocessing import Process

VISION_LOG="vision_log.txt"
HOME_DIR="/home/grfx/"
EXPOSURE=70
OUT_DIR="444"

from getch import _GetchUnix
getch = _GetchUnix()
SV=SystemVariables()

userStudy = 1
assume_lights_off = True

#========================================================================

class RemeshTweenOnce(object):
  def __init__(self):
    SV=SystemVariables()
    os.chdir("/ramdisk")

    command_list= [ SV.remesh_loc,
                    "-i", SV.wall_loc,
                    "-o", SV.obj_name,
                    "-quiet",
                    "-tweening",
                    "-no_remesh",
                    "-blending_subdivision", SV.blending_subdivisions,
                    #"-single_projector_blending",
                    "-use_locked_output_directory", SV.tween_output_dir,
                    "-surface_cameras_fixed_size", SV.camera_size,
                    "-offline",
                    "-floor_cameras_tiled", SV.num_floor_cams,
                    "-p", "8", SV.glcam0, SV.glcam1, SV.glcam2,  SV.glcam3, SV.glcam4, SV.glcam5,SV.glcam6, SV.glcam7 ]

    command_string = " ".join(command_list)
    print command_string
#    time.sleep(5)
    #os.system(command_string)
    proc4 = subprocess.call(command_string, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    # send over the out.view file so the remote renderers can actually read it
    subprocess.call("scp /ramdisk/out.view torch:/ramdisk/out.view", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    subprocess.call("scp /ramdisk/out.view chainsaw:/ramdisk/out.view", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


#========================================================================

class RemeshOnce(object):
  def __init__(self):

    #print "start remesh o"
    SV=SystemVariables()

    if not os.path.exists(SV.image_path):
      os.mkdir(SV.slow_loc)

    os.chdir(SV.slow_loc)

    # remove ppm files without a corresponding glcam
    for ppm_name in glob(SV.image_path+"*.ppm"):
      glcam_name = ppm_name[:-4]+".glcam"
      #print glcam_name
      if not os.path.exists(glcam_name):
        os.remove(ppm_name)
    filelist = glob("*.glcam")
    for f in filelist:
      os.remove(f)

    #os.chdir(SV.slow_loc)

    command_list= [ SV.remesh_loc,
                    "-quiet",
                    "-i", SV.wall_loc,
                    "-create_surface_cameras",
                    "-non_zero_interior_area",
                    "-o", SV.slow_obj_name,
                    "-offline",
                    "-t", SV.tri_target,
                    "-surface_cameras_fixed_size", SV.camera_size,
                    "-floor_cameras_tiled", SV.num_floor_cams,
                    "-colors_file", "~/Checkout/JOSH_EMPAC_2010/remesh_test/webform.colors"]

    command_string = " ".join(command_list)
    #os.system(command_string)
    proc5 = subprocess.call(command_string, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

#========================================================================
#Flags
"""
parser = argparse.ArgumentParser(description = "verbose/quiet")
parser.add_argument('-v', '-verbose', dest='is_verbose', help='print verbose information', action='store_true', default=True)
#parser.add_argument('-q', '-quiet', dest='is_quiet', help='print quiet information', action='store_true', default=False)
parser.add_argument('-q', '-quiet', dest='is_verbose', help='print quiet information', action='store_false', default=True)
parser.add_argument('-fc', '-false_color', dest='false_color', help='display false color', action='store_true', default=False)
parser.add_argument('-l', '--logging', help='creates a directory for user N', default=' ')


#set boolean 'loggingb' to true if not empty
if args.logging == ' ':
    loggingb = False
else:
    loggingb = True

args, unknown_args = parser.parse_known_args()

verb_arg_str = [' -quiet ', ' -verbose '][args.is_verbose]
#quiet_arg_str = ['', ' -quiet '][args.is_quiet]


#if the logging variable isn't empty, create the directory
if loggingb:
    subprocess.call("mkdir -p /home/grfx/logging/" + args.logging + "/" + str(userStudy), shell=True)
"""


#========================================================================

def ProcessImageHelper():
    subprocess.call("rm -f /ramdisk/out.wall", shell=True)
    if assume_lights_off:
        print "RUNDAYLIGHTING: process image helper (lights off)"
        proc3 = subprocess.call("./table_top_detect" + verb_arg_str + "-i /ramdisk/out_lights_off.ppm -o /ramdisk/out.wall -find_architectural_design -debug -debug_path /ramdisk/debug_images/ -message_file /ramdisk/table_top_detect_message_file.txt ", shell=True)#, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        #quiet
    else:
        print "RUNDAYLIGHTING: process image helper (lights on)"
        proc3 = subprocess.call("./table_top_detect" + verb_arg_str + "-i /ramdisk/out_lights_on.ppm -o /ramdisk/out.wall -find_architectural_design -debug -debug_path /ramdisk/debug_images/ -message_file /ramdisk/table_top_detect_message_file.txt ", shell=True)#, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def CheckMessage(filename,message):
    print("Checking Message: filename=%r, message=%r" % (filename, message))
    try:
        f = open(filename)
        content = f.readlines()
        print "RUNDAYLIGHTING: check message\n"
        print content[0]
        print message
        f.close()
        if content[0] == message:
            return True
        else:
            print "RUNDAYLIGHTING: " + filename + " does not match\n"
            return False
    except IOError as ex:
        print(repr(ex))
        print "RUNDAYLIGHTING: " + filename + " does not exist\n"
        sys.exit(0)

def ProcessImage():
    global assume_lights_off
    assume_lights_off = True
    os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/build")
    print "RUNDAYLIGHTING: process image"
    # process the picture the way the last one worked
    ProcessImageHelper();
    # try top open the picture...
    try:
        #if not CheckMessage("/ramdisk/table_top_detect_message_file.txt","[TABLE_TOP_DETECT] success"):
        #    raise IOError
        f = open("/ramdisk/out.wall")
        # great!!
        f.close()
    except IOError:
        # let's switch which picture we are using
        print "RUNDAYLIGHTING: ROOM CONDITION SWITCH!"
        assume_lights_off = not assume_lights_off
        ProcessImageHelper();
        try:
            f = open("/ramdisk/out.wall")
            # great!
            f.close()
        except IOError:
            print "RUNDAYLIGHTING: NEITHER LIGHTS ON OR OFF SEEMS TO WORK!"
            exit(1)




#========================================================================
#def callRemeshOnce():
#    RemeshOnce()
#
#def callRemeshTweenOnce():
#   RemeshTweenOnce()


def RemeshBoth():
    try:
        f = open("/ramdisk/out.wall")
        f.close()
        os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")


        p1 = Process(target=RemeshTweenOnce)
        p2 = Process(target=RemeshOnce)
        # Starts processes.
        p1.start()
        print "RUNDAYLIGHTING: remesh tweening"
        p2.start()
        print "RUNDAYLIGHTING: remeshing"
        # If you would like to wait for the results.
        p1.join()
        p2.join()
    except IOError:
        print "RUNDAYLIGHTING: ERROR: could not open /ramdisk/out.wall"
        exit(1);

#========================================================================

# THIS SHOULD GO AWAY!

def robust_rm(rmstr):
        print("Robust Removing: "+rmstr)
	subprocess.call("sudo rm -f "+rmstr, shell=True)


def BeforeFirstLoop():
    print("Begin First Loop")
    # ( This is now done in render controller for all future images... )
    if args.take_picture:
      print "RUNDAYLIGHTING: take picture"
      os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/build")
      robust_rm("/ramdisk/*ppm")
      proc1 = subprocess.call("./grab_frame_new /ramdisk/out_lights_on.ppm  60000", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
      proc2 = subprocess.call("./grab_frame_new /ramdisk/out_lights_off.ppm  300000", shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
      print "RUNDAYLIGHTING: not taking picture"

    os.chdir("/ramdisk")
    subprocess.call("rm -f "+SV.dayfile_switch, shell=True)
    subprocess.call("echo `cut -d' ' -f1-5 out.time` " "  `grep north out.wall | cut -d' ' -f2` > out.time",shell=True)


    ProcessImage()
    RemeshBoth()
    print "RUNDAYLIGHTING: lsving"
    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")

    try:
        if args.false_color:
            LSVOOnce(EXPOSURE,1)
        else:
            LSVOOnce(EXPOSURE,0)

    except IOError:
        print "RUNDAYLIGHTING: ERROR IN LSVOONCE"
        exit(1)

#========================================================================

#Beginning of main:  Please put all functions above here

parser = argparse.ArgumentParser(description = "verbose/quiet")
parser.add_argument('-v',  '-verbose', dest='is_verbose', help='print verbose information', action='store_true', default=True)
parser.add_argument('-q',  '-quiet', dest='is_verbose', help='print quiet information', action='store_false', default=True)
parser.add_argument('-fc', '-false_color', dest='false_color', help='display false color', action='store_true', default=False)
parser.add_argument('-n',  '-no_pic', dest='take_picture', help='system takes new pictures', action='store_false', default=True)
parser.add_argument('-l',  '--logging', help='creates a directory for user N', default=' ')
parser.add_argument('-u',  '-use_stored_textures', dest='use_stored_textures', help='doesn\'t transfer textures', action='store_true', default=False)

args, unknown_args = parser.parse_known_args()

verb_arg_str = [' -quiet ', ' -verbose '][args.is_verbose]

#set boolean 'loggingb' to true if not empty
if args.logging == ' ':
    loggingb = False
else:
    loggingb = True


def ensure_dir(dir):
    sys.stdout.write('Checking directory: '+str(dir))
    if not os.path.exists(dir):
      sys.stdout.write('... MAKING\n')
      os.makedirs(dir)
    else:
      sys.stdout.write('... ALREADY EXISTS\n')
    sys.stdout.flush()

dirs_to_make = '''
/tween
/ramdisk/images
/ramdisk/logs/placeholder
/ramdisk/debug_images
'''

for dir in dirs_to_make.strip().split('\n'):
    ensure_dir(dir)

#subprocess.call("mkdir -p /tween",              shell=True)
#subprocess.call("mkdir -p /ramdisk/images",             shell=True)
#subprocess.call("mkdir -p /ramdisk/debug_images",       shell=True)
#subprocess.call("mkdir -p /ramdisk/logs/placeholder",   shell=True)

subprocess.call("echo -1 >/ramdisk/images/counter.txt", shell=True)
#subprocess.call("rm -f    /ramdisk/tween/stop",         shell=True)
#subprocess.call("rm -f    /ramdisk/out.wall",           shell=True)
#subprocess.call("rm -f    /ramdisk/debug_images/*",     shell=True)

#THIS SHOULD GO AWAY
BeforeFirstLoop()

os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/tabletop/build")
print "RUNDAYLIGHTING: mpiing"

#if we don't want to take images (just use old one), no pic is passed to combined
picture_string= ['-no_pic ', ' '][args.take_picture]
stored_textures_string= [' ', '-use_stored_textures '][args.use_stored_textures]

#verb_arg_str = ['', ' -verbose '][args.chatty]
#subprocess.call("mpirun -hostfile ~/hosts --mca btl_tcp_if_include eth0  -np 9 combined" + verb_arg_str + picture_string + stored_textures_string + "-noblending & ",shell=True) #verbose

subprocess.call("mpirun -hostfile ~/hosts --mca btl_tcp_if_include eth0  -np 9 combined" + verb_arg_str + picture_string + stored_textures_string + " & ",shell=True) #verbose


#if the logging variable isn't empty, create the directory
if loggingb:
    subprocess.call("mkdir -p /home/grfx/logging/" + args.logging + "/" + str(userStudy), shell=True)
    os.chdir(HOME_DIR + "ramdisk/")
    subprocess.call("cp out.wall ~/logging/" + args.logging + "/" + str(userStudy), shell=True)

#for proc_ in iter([proc1, proc2, proc3, proc4, proc5]):
#   (out, err) = proc_.communicate()
#   os.chdir(HOME_DIR + "GIT_CHECKOUT/dynamic_projection/source/scripts")
#   subprocess.call("touch error.txt", shell = True)
#   if proc_.returncode != 0:
#       file = open('error.txt', 'wb')
#       file.write("Output: " + out + "\nErrors: " + err + "\n")
#       file.close()

#create directories and clean up old files
#proc = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

#raise Exception('  * Failed to execute '+cmd+'\n  * OUTPUT: '+out)

#========================================================================
# the main loop

while True:
  char=getch()

  #quit
  if char == "q":
    subprocess.call("killall combined", shell=True)
    break

  elif char == "t":
    subprocess.call("gedit ~/Checkout/JOSH_EMPAC_2010/remesh_test/out.time", shell=True)


  elif char == "p":
    subprocess.call("rm -f /ramdisk/tween/stop" , shell=True)
    os.chdir(SV.image_path)
    subprocess.call("touch "+SV.dayfile_switch, shell=True)
    print "RUNDAYLIGHTING: lsving"

    os.chdir(HOME_DIR + "Checkout/JOSH_EMPAC_2010/remesh_test/")
    LSVOOnce(EXPOSURE)

    subprocess.call("rm -f /ramdisk/tween/stop", shell=True)
    subprocess.call("rm -f "+SV.dayfile_switch, shell=True)

  #increase exposure
  elif char == "i":
    EXPOSURE=EXPOSURE*2
    LSVOOnce(EXPOSURE)
    print "RUNDAYLIGHTING:"
    print EXPOSURE

  #decrease exposure
  elif char == "o":
    EXPOSURE=EXPOSURE*.5
    LSVOOnce(EXPOSURE)
    print "RUNDAYLIGHTING:"
    print EXPOSURE

  #toggle color calibration
  elif char == "c":
	print "RUNDAYLIGHTING: echo toggling color weights"
	subprocess.call("touch /ramdisk/toggle_color_weights", shell=True)

  #restart
  elif char == "b" or char == "f" :

    print "RUNDAYLIGHTING: pressed b (or f)"

    # we 'ask' combined to take a picture
    # comined = remote_render + render_controller
    subprocess.call("touch /ramdisk/tween/stop", shell=True)
    while os.path.isfile("/ramdisk/tween/stop"):
      time.sleep(0.1)

    # now we can process the picture
    ProcessImage()
    RemeshBoth()
    print "RUNDAYLIGHTING: lsving"
    if char == "b":
      LSVOOnce(EXPOSURE,0)
    if char == "f":
      LSVOOnce(EXPOSURE,1)

    # and 'tell' combined we are done
    subprocess.call("rm -f /ramdisk/tween/stop2", shell=True)

    #if a logging variable was passed in...
    if loggingb:
        userStudy += 1
        subprocess.call("mkdir -p /home/grfx/logging/" + args.logging + "/" + str(userStudy), shell=True)
        os.chdir(HOME_DIR + "ramdisk/")
        subprocess.call("cp out.wall ~/logging/" + args.logging + "/" + str(userStudy), shell=True)


