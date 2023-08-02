cmake_minimum_required(VERSION 3.13)
project(packer)
set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/packer.cpp src/resource.h src/resource.rc)
add_executable(packer ${SOURCE_FILES})
target_include_directories(packer PRIVATE "${SMOL_PROJECT_ROOT}/include" "${SMOL_PROJECT_ROOT}/include/smol")
target_link_libraries(packer PRIVATE smol)
