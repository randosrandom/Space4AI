add_executable(test1 main_test1.cpp)

target_link_libraries(test1 PRIVATE ${LIB_NAME})

add_test(NAME "TestingDynamicPerformanceEvaluator" COMMAND test1 ./config_tests/config_test1.json)
