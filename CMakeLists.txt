cmake_minimum_required(VERSION 3.16.3)

project(DesignTime_PACS VERSION 1.0 LANGUAGES CXX)

find_package(pybind11 REQUIRED)

add_subdirectory(src)
