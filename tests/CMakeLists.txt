add_executable(test_basic main_test.cpp)
add_executable(test_logger main_test_logger.cpp)

target_link_libraries(test_basic PRIVATE ${LIB_NAME})
target_link_libraries(test_logger PRIVATE ${LIB_NAME})

add_test(NAME "TestingSimpleSystemWith5Components" COMMAND test_basic config_tests/config_test1.json)
add_test(NAME "TestingComplexSystemWith15Components" COMMAND test_basic config_tests/config_test2.json)
add_test(NAME "TestingDynamicPerfModel" COMMAND test_basic config_tests/config_test3.json)
add_test(NAME "TestingMultiSystemsRunAndLoggerFileSaving" COMMAND test_logger config_tests/config_test4.json)

if(PARALLELIZATION)
	add_executable(test_parallel main_test_parallel.cpp)
	target_link_libraries(test_parallel PRIVATE ${LIB_NAME})
	add_test(NAME "TestingParallelCompatibility" COMMAND test_parallel config_tests/config_test1_parallel.json)
	set_property(TEST "TestingParallelCompatibility" PROPERTY ENVIRONMENT OMP_NUM_THREADS=2)
endif()
