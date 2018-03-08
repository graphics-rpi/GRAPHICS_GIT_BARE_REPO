#!/usr/bin/python
import os, subprocess, time, shutil
from glob import glob
from system_variables import SystemVariables 
class LSVOOnce(object):
  def __init__(self, exposure, greyscale):
    SV=SystemVariables()
    os.chdir(SV.image_path)

    print "LSVOONCE: start lsvo"

 
    try:
      f = open ( "/home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/out.time" )
      words = f.readline().split()
      print "LSVOONCE: data from /ramdisk/tween/foo.north"
      print words
      month=words[1]
      day=words[2]
      timearray=words[3]
      timearray=timearray.split(":")
      hour=timearray[0]
      minute=timearray[1]
      print "LSVOONCE: time " + str(hour)+ " "+str(minute) 
      print "LSVOONCE: day "+str(day)
      try:
        f2 = open ( "/ramdisk/tween/foo.north" )
        words2 = f2.readline().split()
        print "LSVOONCE: data from f2"
        print words2
        try:
          f3 = open ("/ramdisk/out.time", 'w')
          if len(words2) > 0:
            f3.write(" ".join(words[0:5] + [words2[0]]))
          else:
            f3.write(" ".join(words[0:5] + ["0"]))
          f3.close()
        except IOError:
          print "LSVOONCE: ERROR: could not open /ramdisk/out.time"
          raise IOError()
        f2.close()
      except IOError:
        print "LSVOONCE: ERROR: could not open /ramdisk/tween/foo.north"
        raise IOError()
      f.close()
    except IOError:
      print "LSVOONCE: ERROR: could not open /home/grfx/Checkout/JOSH_EMPAC_2010/remesh_test/out.time"
      raise IOError()

    print "LSVOONCE: after initial files"

        

    for glcam_name in glob(SV.image_path+"*_0.glcam"):
      os.remove(glcam_name)
    print "LSVOONCE: after first for loop"
    # remove ppm files without a corresponding glcam
    for ppm_name in glob(SV.image_path+"*.ppm"):
      glcam_name=ppm_name[:-12]+".glcam"
      if not os.path.exists(glcam_name):
        os.remove(ppm_name)
      print "LSVOONCE: before counter"
#not playing a day
    try:
      f4=open(SV.counter_loc)

      #start_counter=int(f4.readline().split()[0])
      start_counter = 0
      words = f4.readline().split()
      if len(words) > 0:
        start_counter = int(words[0])

      f4.close()
    except IOError:
      print "LSVOONCE: ERROR: could not open "+SV.counter_loc

    print "LSVOONCE: after counter"

    glcams=glob(SV.slow_loc+"*.glcam")
    os.chdir(SV.lsvo_dir)
    for ppm_name in glob("*.ppm"):
      os.remove(SV.lsvo_dir+"/"+ppm_name)
    num_cams=len(glcams)
    if num_cams<1:
      print "LSVOONCE: error no cameras???"
      exit(0)
    
    camera_string=" ".join(glcams)
    glcams = ["/ramdisk/glcams/"+glcam.split('/')[-1]  for glcam in glcams]
    #glcams[0]
    #for glcam in glcams[1:]:
    #  camera_string+=" "+glcam
    #  print "glcam"+glcam
    exec_string="scp "+camera_string+" torch:/ramdisk/glcams"
    print "exec string "+exec_string
    subprocess.call(args=exec_string, shell=True)
    exec_string="scp "+camera_string+" chainsaw:/ramdisk/glcams"
    print "exec string "+exec_string
    subprocess.call(args=exec_string, shell=True)
    exec_string="scp /ramdisk/slow/foo.obj /ramdisk/slow/foo.lsvmtl  chainsaw:/ramdisk/slow"
    print "exec string "+exec_string
    subprocess.call(args=exec_string, shell=True)    
    exec_string="scp /ramdisk/slow/foo.obj /ramdisk/slow/foo.lsvmtl torch:/ramdisk/slow"
    print "exec string "+exec_string
    subprocess.call(args=exec_string, shell=True)
    exec_string="scp /ramdisk/out.wall chainsaw:/ramdisk"
    print "exec string "+exec_string
    subprocess.call(args=exec_string, shell=True)
    exec_string="scp /ramdisk/out.wall torch:/ramdisk"
    print "exec string "+exec_string
    subprocess.call(args=exec_string, shell=True)
    print "before lsvo"
    #path.split('/')[-1]
    time.sleep(1)
    if(greyscale==1):
      exec_string=[
          "./lsvo",
          "-i",  SV.slow_loc+SV.slow_obj_name,  
          "-t", "3000", "-patches", "500", "-offline", "-remeshend",
          "-exp", str(exposure),
          "-B", "-noqt" ,         
          "-t", hour, minute, "-date", month, day,
          "-d="+str(SV.camera_size)+"x"+str(SV.camera_size),
          "-toobright", ".7", "-toodim", ".257", "-greyscale",
          "-dumpOrthos", str(len(glcams))]+ glcams + ["-dumpPeople", SV.wall_loc ] 
    else:
      exec_string=[
          "./lsvo",
          "-i",  SV.slow_loc+SV.slow_obj_name,  
          "-t", "3000", "-patches", "500", "-offline", "-remeshend",
          "-N", "-noqt",  
          "-exp", str(exposure),
          "-t", hour, minute, "-date", month, day,
          "-d="+str(SV.camera_size)+"x"+str(SV.camera_size),
          "-dumpOrthos", str(len(glcams))]+ glcams + ["-dumpPeople", SV.wall_loc ] 

# camera_string]
    exec_string=" ".join(exec_string)
    machines = ['torch', 'chainsaw']
    

    for x in machines:
      machine_string="ssh "+x+" \"cd " + SV.lsvo_dir + " && rm *.ppm\""
      subprocess.call(args=machine_string, shell=True)
    ps = []

    for x in machines:
      print x + " " + exec_string
      outfile = open('lsvo_log_' + x + '.txt', "w") 
      #machine_string="ssh "+x+" \""+exec_string+"\""
      machine_string="ssh "+x+" \"cd " + SV.lsvo_dir + "&&" + exec_string+ " && rm /ramdisk/images/*.ppm\""
      #time.sleep(10)
      NEWPROC=subprocess.Popen(args=machine_string, shell=True)#, stdout=outfile)
      ps.append(NEWPROC)
      outfile.close()

    for p in ps:
        p.wait()

    for x in machines:
      machine_string = "ssh " + x + " \"cd " + SV.lsvo_dir + " && mv *.ppm /ramdisk/images\""
      subprocess.call(args=machine_string, shell=True)
"""     
for ppm_name in glob("*.ppm"):
      if "primary" in ppm_name:
        
        #  NEWPROC=subprocess.call("mogrify -resize 50% "+ppm_name, shell=True)
          shutil.move(ppm_name, "/ramdisk/images/"+ppm_name[:-12]+".ppm")
      else:
          shutil.move(ppm_name, "/ramdisk/images")
"""

if __name__ == "__main__":
  LSVOOnce(.1,0)

