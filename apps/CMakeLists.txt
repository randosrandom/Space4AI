add_executable(dt_solver main_dt_solver.cpp)
target_link_libraries(dt_solver PRIVATE ${LIB_NAME})

add_executable(rt_solver main_rt_solver.cpp)
target_link_libraries(rt_solver PRIVATE ${LIB_NAME})

add_executable(runnEx main_runnEx_analysis.cpp)
target_link_libraries(runnEx PRIVATE ${LIB_NAME})

add_executable(scaAna main_ScalabAnalysis.cpp)
target_link_libraries(scaAna PRIVATE ${LIB_NAME})

if(PARALLELIZATION)
	add_executable(dt_scalability_analysis main_dt_scalability_analysis.cpp)
	target_link_libraries(dt_scalability_analysis PRIVATE ${LIB_NAME})
endif()
