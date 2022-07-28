add_executable(main main.cpp)
add_executable(main_json main_json.cpp)

target_link_libraries(main PRIVATE ${LIB_NAME})
target_link_libraries(main_json PRIVATE ${LIB_NAME})
