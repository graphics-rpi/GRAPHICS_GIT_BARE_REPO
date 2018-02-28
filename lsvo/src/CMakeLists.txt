cmake_minimum_required (VERSION 2.6)
project (lsvoer)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "./cmake/")
message("CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
message("CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ")
# BARB ADDED THIS TO TRY TO MAKE MAC WORK
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
#  SET(CMAKE_CXX_COMPILER g++43)
#  SET(CMAKE_C_COMPILER gcc43)
else()
  SET(CMAKE_CXX_COMPILER g++)
  SET(CMAKE_C_COMPILER gcc)
endif()

SET(REMESH_PATH "" CACHE FILEPATH "blah blah blah")
#########################################################
# OPTIX_add_sample_executable
#
# Convience function for adding samples to the code.  You can copy the contents of this
# funtion into your individual project if you wish to customize the behavior.  Note that
# in CMake, functions have their own scope, whereas macros use the scope of the caller.
#function(OPTIX_add_sample_executable target_name)


function (add_lib_list target liblist)
  foreach (lib ${liblist})
    message(STATUS "WARNING: THIS IS CRAP: ${target} ${lib}")
    if (lib)
      target_link_libraries(${target} "${lib}")
    else()
      message(STATUS "WARNING: missing library: ${lib}")
    endif()
  endforeach()
endfunction()


#endfunction()

#add_executable(lsvo main.cpp AwesomeGLM.cpp GLUTDisplay.cpp SimpleScene.cpp)

IF("${CMAKE_SYSTEM}" MATCHES "Linux")
  SET(CMAKE_CXX_FLAGS "-std=c++0x")
endif()

include_directories(

  #local includes
  "../include/"
  #optix
  #"/Optix/Optix_2_5/include/"
  #cuda
  "/usr/local/cuda/include/"
  #dynamic projection common
  "../../dynamic_projection/source/common/"
  )



##########
# Process our custom setup scripts here.

# Include all CMake Macros.
include(Macros)
# Determine information about the compiler
include (CompilerInfo)
# Check for specific machine/compiler options.
include (ConfigCompilerFlags)

# Turn off the warning that NVCC issues when generating PTX from our CUDA samples.  This
# is a custom extension to the FindCUDA code distributed by CMake.
OPTION(CUDA_REMOVE_GLOBAL_MEMORY_SPACE_WARNING "Suppress the \"Advisory: Cannot tell what pointer points to, assuming global memory space\" warning nvcc makes." ON)
# Default to building 32 bit PTX.  This can still be changed from the GUI later on.
option(CUDA_64_BIT_DEVICE_CODE "Compile device code in 64 bit mode" OFF)

# Find at least a 2.3 version of CUDA.
find_package(CUDA 2.3 REQUIRED)

# Present the CUDA_64_BIT_DEVICE_CODE on the default set of options.
mark_as_advanced(CLEAR CUDA_64_BIT_DEVICE_CODE)

set(flag "--use_fast_math")
list(FIND CUDA_NVCC_FLAGS ${flag} index)
if(index EQUAL -1)
  list(APPEND CUDA_NVCC_FLAGS ${flag})
  set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
endif()

if (CUDA_VERSION VERSION_LESS "3.0")
  set(flag "--keep")
  list(FIND CUDA_NVCC_FLAGS ${flag} index)
  if(index EQUAL -1)
    list(APPEND CUDA_NVCC_FLAGS ${flag})
    set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS} CACHE LIST "Semi-colon delimit multiple arguments." FORCE)
  endif()
endif()

# Search for the OptiX libraries and include files.
find_package(OptiX REQUIRED)
find_package(GLUI REQUIRED)
#find_package( Boost 1.51.0 )
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

include(FindSUtilGLUT)
# Add the path to the OptiX headers to our include paths.
include_directories(
  ${GLUI_INCLUDE_DIR}
  ${OptiX_INCLUDE}
  ${OptiX_INCLUDE}/optixu
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CUDA_INCLUDE_DIRS} 
  ${QT_INCLUDE_DIR}
  "/usr/local/include/"  
  "../../remesher/src"
  "../include/"
  ${GLEW_INCLUDE_DIR}
  ${GLUT_INCLUDE_DIR}
  ${GLUI_INCLUDE_DIR}
  #${Boost_INCLUDE_DIRS}
  )


# configure_file(${CMAKE_CURRENT_SOURCE_DIR}/sampleConfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/sampleConfig.h @ONLY)







set(SAMPLES_CUDA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cuda)

source_group("CUDA Files" REGULAR_EXPRESSION ".+\\.cu$")



  # See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
  #OPTIX_add_sample_executable( lsvo
  SET(lsv_lib_SOURCES 

    DeviceMemoryLogger.cpp
    GLM.cpp
    GLUTDisplay.cpp
    HDRLoader.cpp
    ImageDisplay.cpp
    ImageLoader.cpp
    MeshLoader.cpp
    Mouse.cpp
    PPMLoader.cpp
    SampleScene.cpp
    SimpleScene.cpp
    PinholeCamera.cpp
    sutil.c
    
    constantbg.cu
    eyeRayPass.cu
    importonPass.cu
    gatherPass.cu
    normal_shader.cu
    photonPass.cu
    pinhole_camera.cu
    quickRayPass.cu
    regatherPass.cu
    triangleMesh.cu
    momentTransferPass.cu

     )
    
   SET(lsv_SOURCES
       main.cpp)
    

#   SET(patchgetter_SOURCES
#       patchgetter.cpp)

SET(lsv_HEADERS ../include/qtApp.hpp)



# These calls will group PTX and CUDA files into their own directories in the Visual
# Studio projects.
source_group("PTX Files"  REGULAR_EXPRESSION ".+\\.ptx$")
source_group("CUDA Files" REGULAR_EXPRESSION ".+\\.cu$")

option(CMAKE_SUPPRESS_REGENERATION "suppress regeration of build scripts during build" ON)
mark_as_advanced(CMAKE_SUPPRESS_REGENERATION)
cmake_policy(PUSH)
# Separate the sources from the CMake and CUDA options fed to the macro.  This code
# comes from the CUDA_COMPILE_PTX macro found in FindCUDA.cmake.  We are copying the
# code here, so that we can use our own name for the target.  target_name is used in the
# creation of the output file names, and we want this to be unique for each target in
# the SDK.

CUDA_GET_SOURCES_AND_OPTIONS(source_files cmake_options options lsv_lib_SOURCES)

set(target_name
  "lsvo_lib")
# Create the rules to build the PTX from the CUDA files.
CUDA_WRAP_SRCS( ${target_name} PTX generated_files ${lsv_lib_SOURCES} ${cmake_options}
  OPTIONS ${options} )

# Here is where we create the rule to make the executable.  We define a target name and
# list all the source files used to create the target.  In addition we also pass along
# the cmake_options parsed out of the arguments.

add_library(lsvo_lib ${lsv_lib_SOURCES} ${cmake_options})

# Most of the samples link against the sutil library and the optix library.  Here is the
# rule that specifies this linkage.
target_link_libraries( ${target_name}
  "${optix_LIBRARY}"
  "${optixu_LIBRARY}"
  "${optix_rpath}"
  "${GLUI_LIBRARY}"
  )

set_target_properties (${target_name} PROPERTIES COMPILE_FLAGS "-g -O3 -Wall")

cmake_policy(POP)
find_package(Qt4 REQUIRED)
find_package(GLEW REQUIRED)
INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})  


QT4_WRAP_CPP(lsv_HEADERS_MOC ${lsv_HEADERS})


add_executable(lsvo
  ${lsv_SOURCES}
  ${lsv_HEADERS_MOC}
  )

#       add_executable(patchgetter
#    ${patchgetter_SOURCES}
#    ${lsv_HEADERS_MOC}
#    )

set_target_properties (lsvo PROPERTIES COMPILE_FLAGS "-g -O3 -Wall")
#set_target_properties (patchgetter PROPERTIES COMPILE_FLAGS "-g -O3 -Wall")

#       message(STATUS "Libraries ${QT_LIBRARIES}")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  message(STATUS "WE'RE ON WINDOWS!!")
  target_link_libraries( lsvo
    ${target_name}
    "${QT_LIBRARIES}"
    "${GLUI_LIBRARY}"  
    "${PNG_LIBRARY}"  
    "${GLUT_glut_LIBRARY}"
    "${GLEW_LIBRARY}"
    "${REMESH_LIB}"
    )
elseif(APPLE)

  add_lib_list(lsvo "${QT_LIBRARIES}")

  add_lib_list(lsvo "-L/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/;/opt/local/lib/libGLEW.1.9.0.dylib;-framework GLUI")
  add_lib_list(lsvo "lsvo_lib")
  add_lib_list(lsvo "${GLUI_LIBRARY};${GLUT_glut_LIBRARY};${REMESH_PATH};GLU;GL")

else()
  add_lib_list(lsvo "${QT_LIBRARIES}" )
  MESSAGE("${QT_LIBRARIES}")
  add_lib_list(lsvo "${GLEW_GLEW_LIBRARY}" )
  #    add_lib_list(patchgetter "${QT_LIBRARIES}" )
  
  SET(LINK_LIBRARY debug lsvo debug lsvo_d)
  #SET(LINK_LIBRARY debug lsvo )

  message("GLUT library is ${GLUT_glut_LIBRARY}")
  target_link_libraries( lsvo
    ${target_name}
#    "${QT_LIBRARIES}"
    "${GLUI_LIBRARY}"  
    "${PNG_LIBRARY}"      
    "${GLEW_GLEW_LIBRARY}"
    "${GLUT_glut_LIBRARY}"
    "${REMESH_PATH}"
    "jpeg"
    "GL"
    "GLU"
    "pthread"
    "/usr/local/cuda-5.5/lib64/libcudart.so"
    )
endif()

#    target_link_libraries( patchgetter
#      ${target_name}
#      "${GLUI_LIBRARY}"  

#      "${REMESH_PATH}"

#         
#    )
