# cmake -DCMAKE_BUILD_TYPE=Debug .
# cmake -DCMAKE_BUILD_TYPE=RELWITHDEBINFO .

cmake_minimum_required (VERSION 2.8.11)
project (remesher)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
#find_package(Qt5Widgets)

#FIND_PACKAGE(Qt5 REQUIRED)
# find_package(Qt5Widgets)
#  find_package(Qt5Declarative)
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/usr/local/Cellar/qt5/5.3.0/")
#find_package(Qt5Core NO_MODULE)
find_package(Qt5 COMPONENTS Widgets Declarative)

#set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "/INSTALL/Qt/5.3/ios")



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
endif ()


SET(helloworld_SOURCES main.cpp openglwindow.cpp)
SET(helloworld_HEADERS openglwindow.h)

#QT_WRAP_CPP(remesh helloworld_HEADERS_MOC ${helloworld_HEADERS})

# message(STATUS "QT USE FILE: ${QT_USE_FILE}")

#INCLUDE( ${QT_USE_FILE} )
ADD_DEFINITIONS(${QT_DEFINITIONS})

ADD_EXECUTABLE(remesh ${helloworld_SOURCES}  ${helloworld_HEADERS_MOC})



# platform specific compiler flags to output all compiler warnings
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set_target_properties (remesh PROPERTIES COMPILE_FLAGS "-g -O3 -Wall -pedantic")

#SET(CMAKE_C_COMPILER gcc43)
#SET(CMAKE_CXX_COMPILER g++43)
endif()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set_target_properties (remesh     PROPERTIES COMPILE_FLAGS "-g  -std=c++11 -Wall -pedantic")
endif()

if (WIN32)
set_target_properties (remesh PROPERTIES COMPILE_FLAGS "/W4")
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
SET(CMAKE_CXX_FLAGS "-std=c++0x")
endif()

include_directories(
"/usr/include/GL/"
"/usr/local/include/GL/"
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


#SET(Test_HEADERS test.h)
#QT4_WRAP_CPP(Test_HEADERS_MOC ${Test_HEADERS})


# use the libraries
add_lib_list(remesh "${OPENGL_LIBRARIES}")
add_lib_list(remesh "${GLUT_LIBRARIES}")
target_link_libraries(remesh ${QT_LIBRARIES})

target_link_libraries(remesh Qt5::Widgets)