SHELL = /bin/bash

# ===============================================================

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
# Note: under CC -frounding-math is needed by CGAL.
#       under LIBS, gmp is needed by CGAL (arbitrary precision arithmetic)
CC              = g++ -g -O3 -Wall -pedantic -D__LINUX__ -fPIC -std=c++0x -frounding-math
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib -LCGAL/
LIBS            = -lm -lGL -lGLU -lglut -lCGAL -lgmp
else
ifeq ($(UNAME), FREEBSD)
# FreeBSD
CC              = g++ -g -O3 -Wall -pedantic -D__FREEBSD__
INCLUDE_PATH    = -I/usr/X11R6/include -L/usr/local/include
LIB_PATH        = -L/usr/X11R6/lib -L/usr/local/lib
LIBS            = -lm -lGL -lGLU -lglut
else
ifeq ($(UNAME), Darwin)
# Mac OS X
CC              = g++ -g -Wall -pedantic -frounding-math
LIBS            = -framework GLUT -framework OpenGL -lCGAL
LIB_PATH        = -L/INSTALL/CGAL-3.9/lib
#-framework CGAL -framework boost
INCLUDE_PATH    = -I/INSTALL/CGAL-3.9/include/ -I/INSTALL/boost_1_48_0
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

# ===============================================================
#SRCS	= main.cpp stroke.cpp button.cpp text.cpp Polygon.cpp\
#Tyler: added osc lib files

SRCS	= main.cpp stroke.cpp button.cpp text.cpp \
	../../../../remesher/src/matrix.cpp \
	../../../../remesher/src/utils.cpp \
	../../ir_tracking/ir_data_point.cpp \
	../../calibration/planar_interpolation_calibration/munkres.cpp \
	../../calibration/planar_interpolation_calibration/tracker.cpp \
	../../calibration/planar_interpolation_calibration/planar_calibration.cpp \
#	osc/OscOutboundPacketStream.cpp \
#	osc/OscPrintReceivedElements.cpp \
#	osc/OscReceivedElements.cpp \
#	osc/OscTypes.cpp \
#	ip/IpEndpointName.cpp \
#	ip/NetworkingUtils.cpp \
#	ip/UdpSocket.cpp
	
	
OBJS	= $(SRCS:.cpp=.o)
EXE	= paint

# ===============================================================
# targets

.PHONY: all depend clean

all: depend $(EXE) 

depend:
	$(CC) $(INCLUDE_PATH) -E -M $(SRCS) > Makefile.depend

clean:
	rm -f *~ *bak $(OBJS) $(EXE) $(EXE).exe Makefile.depend

# ===============================================================
# compilation rules

$(EXE): Makefile $(OBJS)
	$(CC) $(INCLUDE_PATH) -o $@ $(OBJS) $(LIB_PATH) $(LIBS) 

.cpp.o: Makefile
	$(CC) $(INCLUDE_PATH) $< -c -o $@

# ===============================================================

-include Makefile.depend
