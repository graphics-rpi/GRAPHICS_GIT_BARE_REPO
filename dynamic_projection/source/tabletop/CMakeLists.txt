cmake_minimum_required (VERSION 2.6)
project (mpi_renderer_thing) 
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake"})
SET(CMAKE_CXX_COMPILER mpicxx)
set( CMAKE_CXX_FLAGS "-g -O2 --std=c++0x" )
#set( CMAKE_EXE_LINKER_FLAGS "-pg" )
include_directories(../common)
include_directories(render_controller)
include_directories(remote_renderer)

#Defines needed for proper compilation of mpi_renderer
add_definitions( -DCONTRAPTION )
add_definitions( -DCOMPRESSED_TEX )
add_definitions( -DWRITE_FILE )
add_definitions( -DMPI_DEFINED )
add_definitions( -DUSE_MPI )
add_definitions( -DGIGE_CAMERA )

#Source files
add_executable(mpi_renderer
remote_renderer/main.cpp
remote_renderer/shader.cpp
../applications/paint/text.cpp
remote_renderer/remote_render.cpp
)

add_executable(render_controller
render_controller/main.cpp
render_controller/render_controller.cpp
../common/GigEVisionUtil.cpp
)

add_executable(combined
main.cpp
remote_renderer/shader.cpp
render_controller/render_controller.cpp
../common/GigEVisionUtil.cpp
../applications/paint/text.cpp
remote_renderer/remote_render.cpp
)

#Source files
add_executable(standalone_remote_renderer
remote_renderer/main.cpp
remote_renderer/shader.cpp
../applications/paint/text.cpp
remote_renderer/remote_render.cpp
)

find_package(X11)
if (NOT X11_FOUND)
   message(FATAL_ERROR "Cannot find X11 library")
endif()
message(STATUS "Found X11 at \"${X11_LIBRARIES}\"")

find_package(OpenGL)
if (NOT OPENGL_FOUND)
   message(FATAL_ERROR "Cannot find OpenGL library")
endif()
message(STATUS "Found OpenGL at \"${OPENGL_LIBRARIES}\"")

find_package(GLEW)
if (NOT GLEW_FOUND)
   message(FATAL_ERROR "Cannot find GLEW library")
endif()
message(STATUS "Found OpenGL at \"${GLEW_LIBRARIES}\"")

find_package(Boost COMPONENTS regex thread-mt)
if (NOT OPENGL_FOUND)
   message(FATAL_ERROR "Cannot find BOOST library")
endif()
message(STATUS "Found OpenGL at \"${Boost_LIBRARIES}\"")



find_package(GLUT)
if (NOT GLUT_FOUND)
   message(FATAL_ERROR "Cannot find GLUT library")
endif()
message(STATUS "Found OpenGL at \"${GLUT_LIBRARIES}\"")


find_package(Z)
if (NOT Z_FOUND)
   message(FATAL_ERROR "Cannot find Z library")
endif()
message(STATUS "Found OpenGL at \"${Z_LIBRARIES}\"")

# search for the libraries
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


target_link_libraries(mpi_renderer ${X11_LIBRARIES})
target_link_libraries(mpi_renderer ${GLEW_LIBRARIES})
target_link_libraries(mpi_renderer ${X11_Xrandr_LIB})
target_link_libraries(mpi_renderer ${OPENGL_LIBRARIES})
target_link_libraries(mpi_renderer ${Boost_LIBRARIES})
target_link_libraries(mpi_renderer ${GLUT_LIBRARIES})
target_link_libraries(mpi_renderer ${Z_LIBRARIES})
target_link_libraries(mpi_renderer ${PNG_LIBRARY})
target_link_libraries(mpi_renderer jpeg)

target_link_libraries(render_controller ${CMAKE_CURRENT_SOURCE_DIR}/../common/GigEVision/libPvAPI.a rt)
target_link_libraries(render_controller ${OPENGL_LIBRARIES})
target_link_libraries(render_controller ${GLUT_LIBRARIES})
target_link_libraries(render_controller ${Z_LIBRARIES})
target_link_libraries(render_controller ${PNG_LIBRARY})
target_link_libraries(render_controller jpeg)

target_link_libraries(combined ${X11_LIBRARIES})
target_link_libraries(combined ${GLEW_LIBRARIES})
target_link_libraries(combined ${X11_Xrandr_LIB})
target_link_libraries(combined ${OPENGL_LIBRARIES})
target_link_libraries(combined ${Boost_LIBRARIES})
target_link_libraries(combined ${GLUT_LIBRARIES})
target_link_libraries(combined ${Z_LIBRARIES})
target_link_libraries(combined ${PNG_LIBRARY})
target_link_libraries(combined jpeg)
target_link_libraries(combined ${CMAKE_CURRENT_SOURCE_DIR}/../common/GigEVision/libPvAPI.a rt)
target_link_libraries(combined)
#target_link_libraries(combined ${CMAKE_CURRENT_SOURCE_DIR}/../common/libcommon.a)

target_link_libraries(standalone_remote_renderer ${X11_LIBRARIES})
target_link_libraries(standalone_remote_renderer ${GLEW_LIBRARIES})
target_link_libraries(standalone_remote_renderer ${X11_Xrandr_LIB})
target_link_libraries(standalone_remote_renderer ${OPENGL_LIBRARIES})
target_link_libraries(standalone_remote_renderer ${Boost_LIBRARIES})
target_link_libraries(standalone_remote_renderer ${GLUT_LIBRARIES})
target_link_libraries(standalone_remote_renderer ${Z_LIBRARIES})
target_link_libraries(standalone_remote_renderer ${PNG_LIBRARY})
target_link_libraries(standalone_remote_renderer jpeg)


set_target_properties (render_controller PROPERTIES COMPILE_FLAGS "")
set_target_properties (combined PROPERTIES COMPILE_FLAGS "")
set_target_properties (mpi_renderer PROPERTIES COMPILE_FLAGS "")
set_target_properties (standalone_remote_renderer PROPERTIES COMPILE_FLAGS "")

