###########################################################################
#CMake build script for Puzzle, ....
cmake_minimum_required (VERSION 2.8)
project (dynamic_projection)

# We've placed a few FindXXX.cmake files in the the source directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeHelpers")

# NOTE: build & run mapview by running:
#  ../source/make_mapview




#############################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Incredibly, for both clang and g++, while a single compile-and-link
   # invocation will create an executable.dSYM/ dir with debug info,
   # with separate compilation the final link does NOT create the
   # dSYM dir.
   # The "dsymutil" program will create the dSYM dir for us.
   # Strangely it takes in the executable and not the object
   # files even though it's the latter that contain the debug info.
   # Thus it will only work if the object files are still sitting around.
   # Note that valgrind provides "--dsymutil=yes" which will run
   # dsymutil for you.
   find_program(DSYMUTIL_PROGRAM dsymutil)
   if (DSYMUTIL_PROGRAM)
      set(CMAKE_CXX_LINK_EXECUTABLE
          "${CMAKE_CXX_LINK_EXECUTABLE}"
	  "${DSYMUTIL_PROGRAM} <TARGET>")
#      set(CMAKE_CXX_CREATE_SHARED_LIBRARY
#          "${CMAKE_CXX_CREATE_SHARED_LIBRARY}"
#	  "${DSYMUTIL_PROGRAM} <TARGET>")
  else ()
     message(STATUS "WARNING: dsymutil not found: symbols may not be created")
   endif ()

   # detect darwin version!  (for hash.h ext/hash_map vs unordered_map
   EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
   STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
   MESSAGE(STATUS "DARWIN_VERSION=${DARWIN_VERSION}")
   IF (DARWIN_VERSION LESS 13)
   ELSE ()
     ADD_DEFINITIONS(-DAPPLE_MAVERICKS)
   ENDIF ()

endif ()

# a work-around function to handle a list of libraries that include a
#  NOTFOUND library
function (add_lib_list target liblist)
  foreach (lib ${liblist})
    if (lib)
      target_link_libraries(${target} "${lib}")
    else()
      message(STATUS "WARNING: missing library: ${lib}")
    endif()
  endforeach()
endfunction()

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
SET(CMAKE_CXX_FLAGS "-std=c++0x")
endif()

#include_directories(
#  "/usr/include/GL/"
#  "/usr/local/include/GL/"
#  "/usr/include/GL/glui-2.36/src/" 
#  "/Library/Frameworks/GLUI.framework/Versions/A/Headers/"
#  "./common"
#)


##########################################
# SEARCH FOR THE LIBRARIES



# OPEN GL
FIND_PACKAGE(OpenGL REQUIRED)
if(OPENGL_FOUND)
  include_directories(${OPENGL_INCLUDE_DIRS})
endif(OPENGL_FOUND)
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")


# GLEW
find_package(GLEW REQUIRED)
if(GLEW_FOUND)
  include_directories(${GLEW_INCLUDE_DIRS})
endif(GLEW_FOUND)
message(STATUS "GLEW_LIBRARIES: ${GLEW_LIBRARIES}")


# GLFW
set(ENV{PKG_CONFIG_PATH} /usr/local/lib/pkgconfig:/usr/lib/pkgconfig:$ENV{PKG_CONFIG_PATH})
find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
  pkg_search_module(GLFW REQUIRED glfw3)
  include_directories(${GLFW_INCLUDE_DIRS})
else(PKG_CONFIG_FOUND)
  message("Did not find pkg-config, trying FindGLFW.cmake")
  find_package(GLFW REQUIRED)
  if(GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
  else(GLFW_FOUND)
  endif(GLFW_FOUND)	
endif(PKG_CONFIG_FOUND)
message(STATUS "GLFW_LIBRARIES: ${GLFW_LIBRARIES}")
message(STATUS "GLFW_STATIC_LIBRARIES: ${GLFW_STATIC_LIBRARIES}")
message(STATUS "GLFW_LDFLAGS: ${GLFW_LDFLAGS}")
message(STATUS "GLFW_STATIC_LDFLAGS: ${GLFW_STATIC_LDFLAGS}")


# PNG
find_package(PNG)
if (PNG_FOUND)
  include_directories(${PNG_INCLUDE_DIR})
else ()
  find_library(PNG_LIBRARY NAMES png libpng.so PATHS "${UBUNTU_LIB_PATH}")
  if (NOT PNG_LIBRARY_FOUND)
    # XXX: find_library not working
    IF (EXISTS "${UBUNTU_LIB_PATH}/libpng.so")
      set(PNG_LIBRARY "${UBUNTU_LIB_PATH}/libpng.so")
    else()
      message(FATAL_ERROR "Could not find libpng.so")
    endif()
  endif ()
  if (NOT EXISTS "/usr/include/png.h")
    message(FATAL_ERROR "Could not find png.h")
  endif ()
endif ()
message(STATUS "INCLUDE: ${PNG_INCLUDE_DIR}")
message(STATUS "LIBRARY: ${PNG_LIBRARY}")


# JPEG
find_package(JPEG)
if (JPEG_FOUND)
  include_directories(${JPEG_INCLUDE_DIR})
else ()
  find_library(JPEG_LIBRARY NAMES png libpng.so PATHS "${UBUNTU_LIB_PATH}")
  if (NOT JPEG_LIBRARY_FOUND)
    # XXX: find_library not working
    IF (EXISTS "${UBUNTU_LIB_PATH}/libpng.so")
      set(JPEG_LIBRARY "${UBUNTU_LIB_PATH}/libpng.so")
    else()
      message(FATAL_ERROR "Could not find libpng.so")
    endif()
  endif ()
  if (NOT EXISTS "/usr/include/png.h")
    message(FATAL_ERROR "Could not find png.h")
  endif ()
endif ()
message(STATUS "INCLUDE: ${JPEG_INCLUDE_DIR}")
message(STATUS "LIBRARY: ${JPEG_LIBRARY}")


# GLM
FIND_PACKAGE(GLM)
IF (GLM_FOUND)
    INCLUDE_DIRECTORIES(${GLM_INCLUDE_DIRS})
ENDIF()
message(STATUS "GLM_INCLUDE: ${GLM_INCLUDE_DIRS}")



# Freetype
FIND_PACKAGE(Freetype REQUIRED)
IF(FREETYPE_FOUND)
    INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})
ENDIF()
message(STATUS "FREETYPE_INCLUDE: ${FREETYPE_INCLUDE_DIRS}")
message(STATUS "FREETYPE_LIBRARY: ${FREETYPE_LIBRARY}")



if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

else()
    find_package(LAPACK)
    if (NOT LAPACK_FOUND)
       message(FATAL_ERROR "Cannot find LAPACK library")
    endif()
    message(STATUS "Found LAPACK at \"${LAPACK_LIBRARIES}\"")


    #find_package(ATLAS)
    #if (NOT ATLAS_FOUND)
    #   message(FATAL_ERROR "Cannot find ATLAS library")
    #endif()
    #message(STATUS "Found ATLAS at \"${ATLAS_LIBRARIES}\"")


    #find_package(SDL)
    #if (NOT SDL_FOUND)
    #   message(FATAL_ERROR "Cannot find SDL library")
    #endif()
    #message(STATUS "Found SDL at \"${SDL_LIBRARY}\"")
endif()



#find_package(OpenAL)
#if (NOT OPENAL_FOUND)
#   message(FATAL_ERROR "Cannot find OpenAL library")
#endif()
#message(STATUS "Found OpenAL at \"${OPENAL_LIBRARY}\"")

find_library(ALUT_LIBRARY NAMES alut)
if (NOT ALUT_LIBRARY)
   #message(FATAL_ERROR "Cannot find alut library")
   message(STATUS "WARNING: Cannot find alut library")
else()
   message(STATUS "Found alut at \"${ALUT_LIBRARY}\"")
endif()

find_package(Threads)
#if (NOT GLUT_FOUND)
#   message(FATAL_ERROR "Cannot find GLUT library")
#endif()
#message(STATUS "Found OpenGL at \"${GLUT_LIBRARIES}\"")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}")
#find_package(Vimba)
#find_package(VimbaCPP)

#############################################################################
###############################################################################

#IF(UNIX)
#  IF( APPLE )
#    SET(EXTRA_WARNINGS "-Wcast-align -Winit-self -Wswitch -Wwrite-strings -std=c++11 -stdlib=libc++ ")
#  ELSE()
#    SET(EXTRA_WARNINGS "-Wcast-align -Winit-self -Wmissing-declarations -Wswitch -Wwrite-strings ")
#  ENDIF()
#  
#  SET( CMAKE_CXX_FLAGS_DEBUG "-O3 -g " )
#  
#  IF(LINUX)
#    SET(CMAKE_CXX_FLAGS "-O3 -g -Wall -Wextra -fopenmp -msse2 -rdynamic ${EXTRA_WARNINGS} -L${LIBRARY_OUTPUT_PATH}")
#    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__LINUX__ -std=c++0x")
#  ENDIF()
#ENDIF()


###############################################################################


# pass list of source files as args beyond extra_cflags
function (tobuild exename extra_libs extra_cflags)
  add_executable(${exename} ${ARGN})
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(base_cflags "-g -O3 -Wall -pedantic")
    TARGET_LINK_LIBRARIES(${exename} rt)
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    #set(base_cflags "-g -O3 -Wall -pedantic -std=c++11 -stdlib=libc++ ")
    set(base_cflags "-g -O3 -Wall -pedantic")
  elseif (WIN32)
    set(base_cflags "/W4")
  endif()  
  set_target_properties(${exename} PROPERTIES COMPILE_FLAGS "${base_cflags} ${extra_cflags}") 
  add_lib_list(${exename} "${extra_libs}")
  #add_lib_list(${exename} "${OPENGL_LIBRARIES};${GLUT_LIBRARIES};${extra_libs}")
endfunction (tobuild)

#############################################################################

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else()

add_executable(grab_frame_new
 common/grab_frame.cpp
 common/GigEVisionCamera.hpp
 common/GigEVisionUtil.cpp
 common/GigEVisionCamera.cpp
)

add_executable(led_pixel_finder
  common/GigEVisionUtil.cpp 
  common/GigEVisionCamera.cpp
  ir_tracking/led_pixel_finder.cpp 
  ir_tracking/viewer.cpp 
  ir_tracking/ir_data_point.cpp
)

add_executable(structured_light_cal
  calibration/tabletop_calibration/structured_light_cal.cpp
  common/GigEVisionUtil.cpp
  common/GigEVisionCamera.cpp
)

add_executable(grab_multiple_frames
  calibration/tabletop_calibration/grab_multiple_frames.cpp
  common/GigEVisionUtil.cpp
  common/GigEVisionCamera.cpp
)

add_executable(undistort_image
  calibration/tabletop_calibration/undistort_image.cpp
  common/GigEVisionUtil.cpp
  common/GigEVisionCamera.cpp
)

add_executable(pixels_to_3d
  calibration/tabletop_calibration/pixels_to_3d.cpp
)

add_executable(process_2d_target
  calibration/tabletop_calibration/process_2d_target.cpp
)

add_executable(parse_zhang_output
  calibration/tabletop_calibration/parse_zhang_output.cpp
)

add_executable(make_opengl_camera
  calibration/tabletop_calibration/make_opengl_camera.cpp
)


#structured_light_cal: structured_light_cal.cpp ../../common/GigEVisionUtil.cpp ../../common/GigEVisionCamera.cpp


add_executable(led_pixel_finder_viewer
  common/GigEVisionUtil.cpp 
  common/GigEVisionCamera.cpp
  ir_tracking/led_pixel_finder.cpp 
  ir_tracking/viewer.cpp 
  ir_tracking/ir_data_point.cpp
)

add_executable(multi_cursor_logger
  multi_cursor/mouseRead.cpp
  multi_cursor/key_and_mouse_logger.cpp
)
endif()


#############################################################################
# VARIOUS TARGETS!

tobuild(image_test "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
 common/image_test.cpp
)

#	g++ $(INCLUDE_PATH) $(CFLAGS) -LGigEVision -DGIGE_CAMERA -o $@ $^ -lPvAPI -lrt -lpthread -lpng -ljpeg
#-ldc1394
#$(LIB_PATH) $(LIBS)  

#tobuild(cubemap_to_fisheye "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
#  cubemap_to_fisheye/cubemap_to_fisheye.cpp
#)


#tobuild(table_top_detect "${JPEG_LIBRARY};${PNG_LIBRARY}" "-DDEBUG"
tobuild(table_top_detect "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  table_top_detection/main.cpp 
  table_top_detection/object.cpp 
  table_top_detection/scene.cpp  
  table_top_detection/soldier.cpp 
  table_top_detection/quad.cpp 
  table_top_detection/wall.cpp 
  table_top_detection/curved_wall.cpp 
  table_top_detection/colortoken.cpp
  common/color_balance.cpp
)

if (0)
tobuild(planar_interpolation_calibration "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  calibration/planar_interpolation_calibration/main.cpp 
  calibration/planar_interpolation_calibration/munkres.cpp 
  calibration/planar_interpolation_calibration/tracker.cpp 
  calibration/planar_interpolation_calibration/planar_calibration.cpp 
  applications/paint/text.cpp 
  ir_tracking/ir_data_point.cpp 
  ../../remesher/src/matrix.cpp 
  ../../remesher/src/utils.cpp
  multi_cursor/key_and_mouse_logger.cpp
)

tobuild(tiled_display_client "" ""
  calibration/planar_interpolation_calibration/socket_client.cpp
)

tobuild(tiled_display_server "" ""
  calibration/planar_interpolation_calibration/socket_server.cpp
)
endif()

if(0)
tobuild(paint "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  applications/paint/main.cpp 
  applications/paint/path.cpp 
  applications/paint/button.cpp
  applications/paint/DrawableObject.cpp 
	applications/paint/ClickableObject.cpp
  applications/paint/text.cpp 
  ../../remesher/src/matrix.cpp 
  ../../remesher/src/utils.cpp 
  ir_tracking/ir_data_point.cpp 
  calibration/planar_interpolation_calibration/munkres.cpp 
  calibration/planar_interpolation_calibration/tracker.cpp 
  calibration/planar_interpolation_calibration/planar_calibration.cpp 
)
endif()

# SEAN KIM
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else()
tobuild(magnification "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  applications/magnification/main.cpp
  applications/magnification/mesh.cpp
  applications/magnification/canvas.cpp
  applications/magnification/camera.cpp
  applications/magnification/cursor.cpp
  applications/magnification/shader_manager.cpp
  ../../remesher/src/matrix.cpp
  applications/paint/path.cpp 
  ../../remesher/src/utils.cpp
  ir_tracking/ir_data_point.cpp
  calibration/planar_interpolation_calibration/munkres.cpp 
  calibration/planar_interpolation_calibration/tracker.cpp 
  calibration/planar_interpolation_calibration/planar_calibration.cpp 
  multi_cursor/interaction.cpp
  multi_cursor/key_and_mouse_logger.cpp
  multi_cursor/multiMouse.cpp
  multi_cursor/laser.cpp
  multi_cursor/cursor.cpp
)
endif()

tobuild(puzzle "${GLEW_LIBRARIES};${GLFW_LIBRARIES};${OPENGL_gl_LIBRARY};${OPENGL_glu_LIBRARY};${FREETYPE_LIBRARIES};${PNG_LIBRARY};${JPEG_LIBRARY}" ""
  applications/puzzle/main.cpp 
  applications/paint/TextManager.cpp
  applications/paint/ButtonManager.cpp
  applications/paint/shader_manager.cpp
  applications/puzzle/PuzzlePiece.cpp
  applications/paint/DrawableObject.cpp 
  applications/paint/ClickableObject.cpp
  applications/paint/button.cpp 
  applications/paint/text.cpp 
  applications/paint/path.cpp 
  ../../remesher/src/matrix.cpp 
  ../../remesher/src/utils.cpp 
  ir_tracking/ir_data_point.cpp 
  calibration/planar_interpolation_calibration/munkres.cpp 
  calibration/planar_interpolation_calibration/tracker.cpp 
  calibration/planar_interpolation_calibration/planar_calibration.cpp 
  multi_cursor/key_and_mouse_logger.cpp
  multi_cursor/multiMouse.cpp
  multi_cursor/laser.cpp
  multi_cursor/interaction.cpp
  multi_cursor/cursor.cpp
)


if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else()
  tobuild (grab_single_frame_Vimba "${VimbaC_LIBRARY};${VimbaCPP_LIBRARY};${PNG_LIBRARY}" ""
    common/grab_single_frame_Vimba.cpp
    )
endif()

#tobuild(synenv_viewer "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  #applications/synenv_viewer/main.cpp 
  #applications/puzzle/PuzzlePiece.cpp
  #applications/paint/DrawableObject.cpp 
  #applications/paint/ClickableObject.cpp
  #applications/paint/button.cpp 
  #applications/paint/text.cpp 
  #applications/paint/path.cpp 
  #../../remesher/src/matrix.cpp 
  #../../remesher/src/utils.cpp 
  #ir_tracking/ir_data_point.cpp 
  #calibration/planar_interpolation_calibration/munkres.cpp 
  #calibration/planar_interpolation_calibration/tracker.cpp 
  #calibration/planar_interpolation_calibration/planar_calibration.cpp 
  #multi_cursor/key_and_mouse_logger.cpp
#)

#tobuild(pong "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  #applications/pong/pong.cpp
  #applications/pong/main.cpp 
  #applications/paint/button.cpp
  #applications/paint/DrawableObject.cpp 
  #applications/paint/ClickableObject.cpp
  #applications/paint/text.cpp 
  #applications/paint/path.cpp 
  #../../remesher/src/matrix.cpp 
  #../../remesher/src/utils.cpp 
  #calibration/planar_interpolation_calibration/munkres.cpp 
  #calibration/planar_interpolation_calibration/tracker.cpp 
  #calibration/planar_interpolation_calibration/planar_calibration.cpp 
  #ir_tracking/ir_data_point.cpp 
#)

IF(WIN32)
 set(SystemTypePath ip/win32)
ELSE(WIN32)
 set(SystemTypePath ip/posix)
ENDIF(WIN32)

if(0)
ADD_LIBRARY(OSC
  common/oscpack/ip/IpEndpointName.cpp 
  common/oscpack/${SystemTypePath}/NetworkingUtils.cpp
  common/oscpack/${SystemTypePath}/UdpSocket.cpp
  common/oscpack/osc/OscPrintReceivedElements.cpp
  common/oscpack/osc/OscReceivedElements.cpp 
  common/oscpack/osc/OscTypes.cpp 
  common/oscpack/osc/OscOutboundPacketStream.cpp
)
endif()

#tobuild(volume_control "${JPEG_LIBRARY};${PNG_LIBRARY};OSC" ""
  #applications/volume_control/main.cpp 
  #applications/volume_control/vcDraw.cpp
  #multi_cursor/key_and_mouse_logger.cpp
  #applications/paint/DrawableObject.cpp
  #applications/paint/ClickableObject.cpp
  #applications/paint/button.cpp 
  #applications/paint/text.cpp 
  #applications/paint/path.cpp 
  #../../remesher/src/matrix.cpp 
  #../../remesher/src/utils.cpp 
  #calibration/planar_interpolation_calibration/munkres.cpp 
  #calibration/planar_interpolation_calibration/tracker.cpp 
  #calibration/planar_interpolation_calibration/planar_calibration.cpp 
  #ir_tracking/ir_data_point.cpp 
#)

if(0)
add_executable(drum_feature
  applications/drum_feature/main.cpp
  multi_cursor/interaction.cpp
  multi_cursor/cursor.cpp
  applications/drum_feature/drumInterface.cpp
  multi_cursor/key_and_mouse_logger.cpp
  applications/paint/DrawableObject.cpp
  applications/paint/ClickableObject.cpp
  applications/paint/button.cpp 
  applications/paint/text.cpp 
  applications/paint/path.cpp 
  ../../remesher/src/matrix.cpp
  ../../remesher/src/utils.cpp
  calibration/planar_interpolation_calibration/munkres.cpp 
  calibration/planar_interpolation_calibration/tracker.cpp 
  calibration/planar_interpolation_calibration/planar_calibration.cpp
  ir_tracking/ir_data_point.cpp  
)
endif()

#tobuild(
#  audio_interface 
#  "${JPEG_LIBRARY};${PNG_LIBRARY};OSC;${CMAKE_THREAD_LIBS_INIT}" ""
#  applications/audio_interface/main.cpp
#  multi_cursor/key_and_mouse_logger.cpp
#  multi_cursor/interaction.cpp
#  multi_cursor/cursor.cpp
#  multi_cursor/multiMouse.cpp
#  applications/audio_interface/PolyInterface.cpp
#  applications/audio_interface/PolyButton.cpp
#  applications/audio_interface/Track.cpp
#  applications/audio_interface/TrackRegion.cpp
#  applications/audio_interface/TrashCan.cpp
#  applications/audio_interface/Bank.cpp
#  applications/audio_interface/Controls.cpp
#  applications/audio_interface/RecordDialog.cpp
#  applications/paint/DrawableObject.cpp
#  applications/paint/ClickableObject.cpp
#  applications/paint/button.cpp 
#  applications/paint/text.cpp 
#  applications/paint/path.cpp 
#  ../../remesher/src/matrix.cpp
#  ../../remesher/src/utils.cpp
#  calibration/planar_interpolation_calibration/munkres.cpp 
#  calibration/planar_interpolation_calibration/tracker.cpp 
#  calibration/planar_interpolation_calibration/planar_calibration.cpp
#  ir_tracking/ir_data_point.cpp  
#  multi_cursor/multiMouse.cpp
#  multi_cursor/laser.cpp
#)

if(0)
# ADDED BY CHRIS STUETZLE
tobuild(graph_interaction "${JPEG_LIBRARY};${PNG_LIBRARY}" ""
  applications/graph_interaction/main.cpp
  applications/graph_interaction/node.cpp
  applications/graph_interaction/graph.cpp
  applications/graph_interaction/object_group.cpp
  multi_cursor/interaction.cpp
  multi_cursor/cursor.cpp
  multi_cursor/multiMouse.cpp
  multi_cursor/laser.cpp
  multi_cursor/key_and_mouse_logger.cpp
  applications/paint/DrawableObject.cpp
  applications/paint/ClickableObject.cpp
  applications/paint/button.cpp 
  applications/paint/text.cpp 
  applications/paint/path.cpp 
  ../../remesher/src/matrix.cpp
  ../../remesher/src/utils.cpp
  calibration/planar_interpolation_calibration/munkres.cpp 
  calibration/planar_interpolation_calibration/tracker.cpp 
  calibration/planar_interpolation_calibration/planar_calibration.cpp
  ir_tracking/ir_data_point.cpp
)
endif()

#############################################################################

# platform specific compiler flags to output all compiler warnings
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set_target_properties (grab_frame_new PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA")
set_target_properties (led_pixel_finder PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DMYNDEBUG -DNDEBUG") 
set_target_properties (led_pixel_finder_viewer PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic") 
set_target_properties (structured_light_cal PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (grab_multiple_frames PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (undistort_image PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (pixels_to_3d PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (process_2d_target PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (parse_zhang_output PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (make_opengl_camera PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic -DGIGE_CAMERA") 
set_target_properties (multi_cursor_logger PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic")
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
endif()

if (WIN32)
set_target_properties (grab_frame_new PROPERTIES COMPILE_FLAGS "/W4")
set_target_properties (led_pixel_finder PROPERTIES COMPILE_FLAGS "/W4")
set_target_properties (led_pixel_finder_viewer PROPERTIES COMPILE_FLAGS "/W4")
set_target_properties (multi_cursor_logger PROPERTIES COMPILE_FLAGS "/W4")
endif()

#############################################################################

# use the libraries
if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
else()

add_lib_list(grab_frame_new "${OPENGL_LIBRARIES}")
#add_lib_list(grab_frame_new "${GLUT_LIBRARIES}")
target_link_libraries(grab_frame_new ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(grab_frame_new ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(grab_frame_new rt)
target_link_libraries(grab_frame_new ${JPEG_LIBRARY})
target_link_libraries(grab_frame_new ${PNG_LIBRARY})

add_lib_list(led_pixel_finder "${OPENGL_LIBRARIES}")
#add_lib_list(led_pixel_finder "${GLUT_LIBRARIES}")
target_link_libraries(led_pixel_finder ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(led_pixel_finder ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(led_pixel_finder rt)
target_link_libraries(led_pixel_finder ${JPEG_LIBRARY})
target_link_libraries(led_pixel_finder ${PNG_LIBRARY})

add_lib_list(led_pixel_finder_viewer "${OPENGL_LIBRARIES}")
#add_lib_list(led_pixel_finder_viewer "${GLUT_LIBRARIES}")
target_link_libraries(led_pixel_finder_viewer ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(led_pixel_finder_viewer ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(led_pixel_finder_viewer rt)
target_link_libraries(led_pixel_finder_viewer ${JPEG_LIBRARY})
target_link_libraries(led_pixel_finder_viewer ${PNG_LIBRARY})

add_lib_list(structured_light_cal "${OPENGL_LIBRARIES}")
#add_lib_list(structured_light_cal "${GLUT_LIBRARIES}")
target_link_libraries(structured_light_cal ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(structured_light_cal ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(structured_light_cal rt)
target_link_libraries(structured_light_cal ${JPEG_LIBRARY})
target_link_libraries(structured_light_cal ${PNG_LIBRARY})
target_link_libraries(structured_light_cal ${SDL_LIBRARY})

add_lib_list(grab_multiple_frames "${OPENGL_LIBRARIES}")
#add_lib_list(grab_multiple_frames "${GLUT_LIBRARIES}")
target_link_libraries(grab_multiple_frames ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(grab_multiple_frames ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(grab_multiple_frames rt)
target_link_libraries(grab_multiple_frames ${JPEG_LIBRARY})
target_link_libraries(grab_multiple_frames ${PNG_LIBRARY})
target_link_libraries(grab_multiple_frames ${SDL_LIBRARY})

add_lib_list(undistort_image "${OPENGL_LIBRARIES}")
#add_lib_list(undistort_image "${GLUT_LIBRARIES}")
target_link_libraries(undistort_image ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(undistort_image ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(undistort_image rt)
target_link_libraries(undistort_image ${JPEG_LIBRARY})
target_link_libraries(undistort_image ${PNG_LIBRARY})
target_link_libraries(undistort_image ${SDL_LIBRARY})

add_lib_list(pixels_to_3d "${OPENGL_LIBRARIES}")
#add_lib_list(pixels_to_3d "${GLUT_LIBRARIES}")
target_link_libraries(pixels_to_3d ${PROJECT_SOURCE_DIR}/common/libcommon.a)
target_link_libraries(pixels_to_3d ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(pixels_to_3d rt)
target_link_libraries(pixels_to_3d ${JPEG_LIBRARY})
target_link_libraries(pixels_to_3d ${PNG_LIBRARY})
target_link_libraries(pixels_to_3d ${SDL_LIBRARY})

add_lib_list(process_2d_target "${LAPACK_LIBRARIES}")
#THE next two lines suck....fix them
target_link_libraries(process_2d_target "-L/usr/lib64/atlas")
target_link_libraries(process_2d_target atlas)
target_link_libraries(process_2d_target m)
target_link_libraries(process_2d_target ${JPEG_LIBRARY})
target_link_libraries(process_2d_target ${PNG_LIBRARY})


target_link_libraries(multi_cursor_logger ${CMAKE_THREAD_LIBS_INIT})


endif()


# some linux compilations require this hack to get the libraries in the right order
if (APPLE)
else()
  if (UNIX)
    set(MISSING_FLAGS "-lX11 -lXxf86vm -lXrandr -lpthread -lXi")
    target_link_libraries(puzzle "${OPENGL_gl_LIBRARY}" "${GLEW_LIBRARIES}"  "${GLFW_LIBRARIES} ${MISSING_FLAGS}")
  endif()
endif()

## this will hopefully work whether you have the static or the dynamic GLFW libraries
# string replace hack to fix a bug in the pkg_config information
string(REPLACE ";" " " flags_static "${GLFW_STATIC_LDFLAGS}")
string(REPLACE ";" " " flags_dynamic "${GLFW_LDFLAGS}")
set_property(TARGET puzzle APPEND_STRING PROPERTY LINK_FLAGS "${flags_static} ${flags_dynamic}")




# build the tabletop directory executables too
#add_subdirectory(tabletop)
