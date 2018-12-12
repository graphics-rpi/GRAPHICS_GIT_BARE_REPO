  ./lsvo \
      -i ${1}foo.obj \
      -t 3000 \
      -patches 1000 -offline \
      -remeshend -N -noqt -exp 70 -t 12:30 \
      -date 12 1 \
      -d=512x512 \
      -dumpOrthos `ls ${1}*.glcam|wc -l` `ls ${1}*.glcam` \
      -verbose \
      -weather CLEAR 
