add_executable(test_basic main_test.cpp)
add_executable(test_logger main_test_logger.cpp)
add_executable(test_dt_localsearch main_test_dt_localsearch.cpp)
add_executable(test_rt_localsearch main_test_rt_localsearch.cpp)

target_link_libraries(test_basic PRIVATE ${LIB_NAME})
target_link_libraries(test_logger PRIVATE ${LIB_NAME})
target_link_libraries(test_dt_localsearch PRIVATE ${LIB_NAME})
target_link_libraries(test_rt_localsearch PRIVATE ${LIB_NAME})


if(PARALLELIZATION)
	add_executable(test_parallel main_test_parallel.cpp)
	target_link_libraries(test_parallel PRIVATE ${LIB_NAME})
	add_test(NAME "TestingParallelCompatibility" COMMAND test_parallel config_tests/config_test1_parallel.json)
	set_property(TEST "TestingParallelCompatibility" PROPERTY ENVIRONMENT OMP_NUM_THREADS=2)
endif()

add_test(NAME "TestingSimpleSystemWith5Components" COMMAND test_basic config_tests/config_test5c.json)
add_test(NAME "TestingComplexSystemWith15Components" COMMAND test_basic config_tests/config_test15c.json)
add_test(NAME "TestingMultiSystemsRunAndLoggerFileSaving" COMMAND test_logger config_tests/config_test_logger.json)
add_test(NAME "TestingDesignTimeLocalSearch" COMMAND test_dt_localsearch config_tests/config_test_dt_localsearch.json)
add_test(NAME "TestingRunTimeRandomGreedyAndLocalSearch" COMMAND test_rt_localsearch config_tests/config_test_rt_localsearch.json)

