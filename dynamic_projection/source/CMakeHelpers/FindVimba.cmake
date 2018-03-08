# - Try to find ImageMagick++
# Once done, this will define
#
#  Magick++_FOUND - system has Magick++
#  Magick++_INCLUDE_DIRS - the Magick++ include directories
#  Magick++_LIBRARIES - link these to use Magick++

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(VimbaC_PKGCONF VimbaC)
#libfind_pkg_check_modules(VimbaC++_PKGCONF VimbaCPP)

# Include dir
find_path(Vimba_INCLUDE_DIR
  NAMES Vimba.h
  PATHS ${Vimba_PKGCONF_INCLUDE_DIRS} "."
)

# Finally the library itself
find_library(VimbaC_LIBRARY
  NAMES VimbaC
  PATHS ${Vimba_PKGCONF_LIBRARY_DIRS} "${CMAKE_SOURCE_DIR}/common/VimbaC"
)

#find_library(VimbaCPP_LIBRARY
#  NAMES VimbaCPP
#  PATHS ${Vimba_PKGCONF_LIBRARY_DIRS} "."
#)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Vimba_PROCESS_INCLUDES Vimba_INCLUDE_DIRS)
#set(Vimba_PROCESS_LIBS VimbaC_LIBRARY VimbaCPP_LIBRARY )
libfind_process(Vimba)
