###############################################################
## Some Common Variables
###############################################################

option(PARALLELIZATION "Set ON if you want the parallel version" OFF)

if(NOT PARALLELIZATION)
	message("Compiling Serial version of the library")
	set(LIB_NAME s4aidt) # library name
else()
	message("Compiling Parallel version of the library")
	set(LIB_NAME s4aidt_parallel)
endif()

option(SHARED "Set OFF if you want the shared library" ON)

set(MY_CXX_FLAGS -Wall -Wextra -Wpedantic -Wunused -Wsuggest-override -Wnon-virtual-dtor)
set (CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING # Default build type: Release.
	"Choose the type of build, options are: Debug MinSizeRel Release RelWithDebInfo."
	FORCE)
endif()
message("Compiling code in mode: " ${CMAKE_BUILD_TYPE})

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

################################################################
## Project
################################################################

cmake_minimum_required(VERSION 3.16.3)

project(DesignTime_PACS VERSION 1.0 LANGUAGES CXX)

find_package(pybind11 REQUIRED)
include_directories(${pybind11_INCLUDE_DIRS})

if(PARALLELIZATION)
	find_package(OpenMP REQUIRED)
endif()

# Properties for all targets
add_compile_options(${MY_CXX_FLAGS})

# Add folders
add_subdirectory(src)

add_subdirectory(apps)

include(CTest)
add_subdirectory(tests)

# Create Symlink to config folders (think if it is best to have hard copy ...)
execute_process(COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/config/ ${CMAKE_BINARY_DIR}/apps/config)
execute_process(COMMAND cmake -E create_symlink ${CMAKE_SOURCE_DIR}/tests/config_tests ${CMAKE_BINARY_DIR}/tests/config_tests)

# Documentation
find_package(Doxygen COMPONENTS dot)

if(DOXYGEN_FOUND)
	include(makeDoxygen)
else()
	message(WARNING "Doxygen or graphviz was not found, please check. Documentation will not be generated")
endif()
