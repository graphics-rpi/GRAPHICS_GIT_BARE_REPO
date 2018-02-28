# cmake -DCMAKE_BUILD_TYPE=Debug .
# cmake -DCMAKE_BUILD_TYPE=RELWITHDEBINFO .

cmake_minimum_required (VERSION 2.6)
project (remesher)


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
      set(CMAKE_CXX_CREATE_SHARED_LIBRARY
          "${CMAKE_CXX_CREATE_SHARED_LIBRARY}"
	  "${DSYMUTIL_PROGRAM} <TARGET>")
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

add_library(remesh_lib 
remesher.cpp
puzzle.cpp offscreen_rendering2.cpp mesh.cpp utils.cpp 
# stackdump.cpp 
matrix.cpp camera.cpp glCanvas.cpp heap.cpp boundingbox.cpp 
element.cpp accelerationgrid.cpp triangle.cpp quad.cpp polygon.cpp walls.cpp 
wall.cpp wall_fingerprint.cpp walls_create_triangles.cpp surface_camera.cpp 
edge.cpp plane.cpp collect.cpp load.cpp load_material.cpp output.cpp render.cpp subdivision.cpp 
split.cpp collapse.cpp flip.cpp movevertices.cpp remesh.cpp image.cpp 
nosfile.cpp ortho_camera.cpp mincut.cpp patches.cpp
mtrand.cpp
bvh.cpp
${Test_HEADERS_MOC})

add_executable(remesh 
main.cpp 
)



# platform specific compiler flags to output all compiler warnings
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set_target_properties (remesh PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic")
set_target_properties (remesh_lib PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic")

#SET(CMAKE_C_COMPILER g++)
#SET(CMAKE_CXX_COMPILER clang++)
SET(CMAKE_CXX_COMPILER /usr/bin/g++-5)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

# suppressing some warnings in GLUI (should rewrite to remove GLUI!)
set_target_properties (remesh PROPERTIES COMPILE_FLAGS "-g -Wno-deprecated-declarations -Wall -Wno-overloaded-virtual -pedantic -std=c++11")
set_target_properties (remesh_lib PROPERTIES COMPILE_FLAGS "-g -Wno-deprecated-declarations -Wall -Wno-overloaded-virtual -pedantic -std=c++11")

# was this:
#set_target_properties (remesh PROPERTIES COMPILE_FLAGS "-g  -Wall -pedantic")
#set_target_properties (remesh_lib PROPERTIES COMPILE_FLAGS "-g  -Wall -pedantic")

# even older
#set_target_properties (remesh PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic")
#set_target_properties (remesh_lib PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic")
endif()

if (WIN32)
set_target_properties (remesh PROPERTIES COMPILE_FLAGS "/W4")
set_target_properties (remesh_lib PROPERTIES COMPILE_FLAGS "/W4")
endif()



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
SET(CMAKE_CXX_FLAGS "-std=c++11")
endif()

include_directories(
"/usr/include/GL/"
"/usr/local/include/GL/"
"/usr/include/GL/glui-2.36/src/" 
"/Library/Frameworks/GLUI.framework/Versions/A/Headers/"
)


# search for the libraries
find_package(GLUT)
if (NOT GLUT_FOUND)
   message(FATAL_ERROR "Cannot find GLUT library")
endif()
message(STATUS "Found OpenGL at \"${GLUT_LIBRARIES}\"")

find_package(OpenGL)
if (NOT OPENGL_FOUND)
   message(FATAL_ERROR "Cannot find OpenGL library")
endif()
message(STATUS "Found OpenGL at \"${OPENGL_LIBRARIES}\"")

find_library(GLUI_LIBRARIES "glui")
if (NOT GLUI_LIBRARIES)    # THIS IS BROKEN I THINK
#if (GLUI_LIBRARIES-NOTFOUND) #
   message(FATAL_ERROR "Cannot find GLUI library")
endif()
message(STATUS "Found GLUI at \"${GLUI_LIBRARIES}\"")


#FIND_PACKAGE(Qt4 REQUIRED)
#INCLUDE( ${QT_USE_FILE} )
#ADD_DEFINITIONS(${QT_DEFINITIONS})

#SET(Test_HEADERS test.h)
#QT4_WRAP_CPP(Test_HEADERS_MOC ${Test_HEADERS})


# use the libraries
add_lib_list(remesh_lib "${OPENGL_LIBRARIES}")
add_lib_list(remesh_lib "${GLUT_LIBRARIES}")
#target_link_libraries(remesh_lib ${GLUI_LIBRARIES} ${QT_LIBRARIES})
target_link_libraries(remesh_lib ${GLUI_LIBRARIES})
target_link_libraries(remesh remesh_lib)

