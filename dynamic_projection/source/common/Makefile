TARGETS = libcommon.a image_test 
#grab_frame_revised

CFLAGS = -std=c++0x -g -O3 -Wall -I.

all: $(TARGETS)

libcommon.a : CIE.o ASTM_E490.o GigEVision/libPvAPI.a
	cd GigEVision;ar -x libPvAPI.a
	cd DataQ;ar -x libdataqsdk.a
	ar rc $@ $^ GigEVision/*.o DataQ/*.o

image_test: image_test.cpp libcommon.a
	g++ $(CFLAGS) -o $@ $^ -L. -lcommon

#grab_frame_revised: grab_frame.cpp GigEVisionCamera.hpp GigEVisionUtil.cpp
#	g++ $(INCLUDE_PATH) $(CFLAGS) -LGigEVision -DGIGE_CAMERA -o $@ $^ -lPvAPI -lrt -lpthread -lpng -ljpeg
#-ldc1394
#$(LIB_PATH) $(LIBS)  

%.o : %.c %.h
	gcc $(CFLAGS) -c $<

%.o : %.c
	gcc $(CFLAGS) -c $<

%.o : %.cpp %.h %.hpp
	g++ $(CFLAGS) -c $<

%.o : %.cpp
	g++ $(CFLAGS) -c $<

clean:
	rm -f $(TARGETS) *.o *~ core
 
