cmake_minimum_required (VERSION 2.6)

# Maps to Visual Studio solution file (Tutorial.sln)
# The solution will have all targets (exe, lib, dll) 
# as Visual Studio projects (.vcproj)
project (Stars)

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

include(jstd/CMake/PrecompiledHeader.cmake)

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
elseif (CMAKE_COMPILER_IS_GNUCXX )
    message("Using C++11 please!")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(MSVC)

add_definitions(-D_AFXDLL -DUNICODE -D_UNICODE)
set(CMAKE_MFC_FLAG 2)

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

# Sub-directories where more CMakeLists.txt exist
set(STARS_SOURCES_CPP
	"pchheader.cpp"
	"Stars.cpp"
	"StarsWnd.cpp"
	)
set(STARS_SOURCES_H
	"Resource.h"
	"Stars.h"
	"StarsWnd.h"
	"targetver.h"
	"pchheader.h"
	)

add_subdirectory(jstd)

source_group("include" FILES ${STARS_SOURCES_H})
source_group("src" FILES ${STARS_SOURCES_CPP})

add_executable(Stars WIN32 ${STARS_SOURCES_CPP}
                           ${STARS_SOURCES_H})

add_precompiled_header(Stars pchheader.h FORCEINCLUDE pchheader.cpp)
include_directories(jstd)
target_link_libraries(Stars JStd)
