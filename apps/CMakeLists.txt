add_executable(dt_solver main_dt_solver.cpp)
target_link_libraries(dt_solver PRIVATE ${LIB_NAME})

if(PARALLELIZATION)
	add_executable(dt_scalability_analysis main_dt_scalability_analysis.cpp)
	target_link_libraries(dt_scalability_analysis PRIVATE ${LIB_NAME})
endif()
