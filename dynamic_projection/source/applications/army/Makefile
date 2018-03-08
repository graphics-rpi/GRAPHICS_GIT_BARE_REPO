
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
UNAME := LINUX
else 
ifeq ($(UNAME), FreeBSD)
UNAME := FREEBSD
endif
endif

ifeq ($(UNAME), LINUX)
# Linux
CC              = g++ -g -O3 -Wall -pedantic -D__LINUX__ -fPIC -std=c++0x
INCLUDE_PATH    = -I/usr/X11R6/include -I../../common
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib -L../../common/GigEVision -L/lib64
LIBS            = -lm -lGL -lGLU -lglut -lPvAPI -lpthread
else
ifeq ($(UNAME), FREEBSD)
# FreeBSD
CC              = g++ -g -O3 -Wall -pedantic -D__FREEBSD__
INCLUDE_PATH    = -I/usr/X11R6/include -I../../common #-I/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), Darwin)
# Mac OS X
CC              = g++ -g -Wall -pedantic 
INCLUDE_PATH    = -I/usr/X11R6/include -I../../common # -I/usr/local/include
LIBS            = -framework GLUT -framework OpenGL
else
ifeq ($(findstring CYGWIN,$(UNAME)),CYGWIN)
# Cygwin with native Windows libraries
CC              = g++ -g -O3 -Wall -pedantic 
INCLUDE_PATH    = -I/usr/include/w32api -I/usr/include/opengl
LIB_PATH        = -L/usr/lib/w32api
LIBS            = -lm -lopengl32 -lglu32 -lglut32 
else
$(error "Unknown OS: " $(UNAME))
endif
endif
endif
endif



CAMERA_TYPE=GIGE_CAMERA
#CFLAGS= -O3 -g -I../../common -std=c++0x
#LDFLAGS=  -L/usr/local/lib  -L/usr/lib64 -L../../common -lpthread -lX11

EXECUTABLES= army \
	     grab_frame \

#	     table_top_detect \
#	     find_soldiers \
#	     find_terrain \


all: $(EXECUTABLES)

army: army.cpp munkres.cpp 
	$(CC) $(INCLUDE_PATH) -o $@ $^ -lSDL -lGL -lGLU -lglut  $(LIB_PATH) $(LIBS) 

#find_soldiers: find_soldiers.cpp
#	$(CC) $(INCLUDE_PATH) -DGIGE_CAMERA -o $@ $^  $(LIB_PATH) $(LIBS) 

#	$(CC) $(INCLUDE_PATH) -DGIGE_CAMERA -o $@ $^  $(LIB_PATH) $(LIBS) 

#find_terrain: find_terrain.cpp
#	$(CC) $(INCLUDE_PATH) -DGIGE_CAMERA -o $@ $^  $(LIB_PATH) $(LIBS) 

#table_top_detect: table_top_detect.cpp
#	$(CC) $(INCLUDE_PATH) -DGIGE_CAMERA -o $@ $^  $(LIB_PATH) $(LIBS) 

#grab_frame: grab_frame.cpp ../../common/GigEVisionCamera.hpp
#	$(CC) $(INCLUDE_PATH) -DGIGE_CAMERA -o $@ $^ $(LIB_PATH) $(LIBS)  -ldc1394 -lrt

clean:
	rm -f find_soldiers find_terrain army grab_frame

#army: army.cpp munkres.cpp
#	g++ $(CFLAGS) -o $@ $^
