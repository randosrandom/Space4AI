# Variables related to library

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # shared library output folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # static library output folder

set(INCLUDE_DIRS "${CMAKE_SOURCE_DIR};${CMAKE_BINARY_DIR}")

set(PYTHONDIR ${CMAKE_SOURCE_DIR}/extras/Python) # PYTHONDIR needed in PerformancePredictors.hpp.in
configure_file(${CMAKE_SOURCE_DIR}/src/Performance/PerformancePredictors.hpp.in ${CMAKE_BINARY_DIR}/src/Performance/PerformancePredictors.hpp @ONLY)

# Parallelization
configure_file(${CMAKE_SOURCE_DIR}/src/Algorithms/RandomGreedyDT.cpp.in ${CMAKE_BINARY_DIR}/src/Algorithms/RandomGreedyDT.cpp)

set(SRCS
"${CMAKE_BINARY_DIR}/src/Algorithms/RandomGreedyDT.cpp;\
${CMAKE_SOURCE_DIR}/src/Graph/Dag.cpp;\
${CMAKE_SOURCE_DIR}/src/Performance/PerformanceModels.cpp;\
${CMAKE_SOURCE_DIR}/src/Performance/SystemPE.cpp;\
${CMAKE_SOURCE_DIR}/src/Solution/EliteResult.cpp;\
${CMAKE_SOURCE_DIR}/src/Solution/Solution.cpp;\
${CMAKE_SOURCE_DIR}/src/System/SystemData.cpp;\
${CMAKE_SOURCE_DIR}/src/System/System.cpp;")

set(EXT_SRCS
"${CMAKE_SOURCE_DIR}/external/chrono/chrono.cpp")

if(SHARED)
	message("Creating Shared Library")
	# message(WARNING "Creating shared library requires default visibility, which however might generate warnings when linking the code with pybind11 library")
	add_library(${LIB_NAME} SHARED ${SRCS} ${EXT_SRCS})
else()
	message("Creating Static Library")
	add_library(${LIB_NAME} STATIC ${SRCS} ${EXT_SRCS})
	set_target_properties(${LIB_NAME} PROPERTIES CXX_VISIBILITY_PRESET "hidden" CUDA_VISIBILITY_PRESET "hidden")
endif()

# target compile and linking target features

target_include_directories(${LIB_NAME} PUBLIC ${INCLUDE_DIRS}) # location of header files

target_link_libraries(${LIB_NAME} PUBLIC pybind11::embed) # linking to pybind11

if(PARALLELIZATION)
	target_link_libraries(${LIB_NAME} PUBLIC OpenMP::OpenMP_CXX)
endif()
