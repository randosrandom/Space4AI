# My Variables

option(SHARED_LIB_OPT "Settable from command line" OFF)
option(RELEASE_OPT "Settable from command line" OFF)

if(RELEASE_OPT)
	message("Compiling RELEASE, optimization flags will be activated")
	set(CMAKE_BUILD_TYPE Release)
else()
	message("Compiling in DEBUG mode")
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wsuggest-override -Wnon-virtual-dtor")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY lib)

# SRCS, TARGETS AND INCLUDES

set(LIB_NAME "S4AIDT")

set(SRCS
"${CMAKE_CURRENT_SOURCE_DIR}/Graph/Dag.cpp;\
${CMAKE_CURRENT_SOURCE_DIR}/System/SystemData.cpp")

set(INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}") # You can add others if you need ...

if(SHARED_LIB_OPT)
	message("Creating Shared Library")
	message(WARNING "Creating shared library requires default visibility, which however might generate warnings when linking the code with pybind11 library")
	add_library(${LIB_NAME} SHARED ${SRCS})
else()
	message("Compiling Static Library")
	add_library(${LIB_NAME} STATIC ${SRCS})
endif()

# target compile and linking target features
target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIRS})
target_compile_features(${LIB_NAME} PUBLIC cxx_std_17)
target_link_libraries(${LIB_NAME} PRIVATE pybind11::embed)
