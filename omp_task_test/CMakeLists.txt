cmake_minimum_required(VERSION 3.16)

project(OmpTaskTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(omp_task_test main.cpp)

if(OpenMP_CXX_FOUND)
    target_compile_options(omp_task_test PRIVATE ${OpenMP_CXX_FLAGS})
    target_include_directories(omp_task_test PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
    target_link_libraries(omp_task_test ${OpenMP_CXX_LIBRARIES})
endif()
