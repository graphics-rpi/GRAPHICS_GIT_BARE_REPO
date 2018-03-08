#!/usr/bin/python
from system_variables import SystemVariables 
import subprocess
import os

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
    os.system(command_string)



