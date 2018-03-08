#!/usr/bin/python
import os
import subprocess
from glob import glob
from system_variables import SystemVariables 
import shutil

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
    os.system(command_string)


