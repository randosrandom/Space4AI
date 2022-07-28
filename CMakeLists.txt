###############################################################
## Some Common Variables
###############################################################

set(LIB_NAME S4AIDT) # library name

set(MY_CXX_FLAGS -Wall -Wextra -Wsuggest-override -Wnon-virtual-dtor)
set (CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING	# Default build type: Debug.
	"Choose the type of build, options are: Debug MinSizeRel Release RelWithDebInfo."
	FORCE)
endif()

set(SHARE_DIR "${CMAKE_SOURCE_DIR}/config")    # Directory containing test and configuration files.

################################################################
## Project
################################################################

cmake_minimum_required(VERSION 3.16.3)

project(DesignTime_PACS VERSION 1.0 LANGUAGES CXX)

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS})

# Properties for all targets
add_compile_options(${MY_CXX_FLAGS})

# Add folders
add_subdirectory(src)

add_subdirectory(apps)

include(CTest)
add_subdirectory(tests)

# Create Symlink to config folders (think if it is best to have hard copy ...)
execute_process(COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/config/config_apps ${CMAKE_BINARY_DIR}/apps/config_apps)
execute_process(COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/config/config_tests ${CMAKE_BINARY_DIR}/tests/config_tests)

