cmake_minimum_required(VERSION 3.16)

project(OmpTestNested LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(omp_test_nested main.cpp)

if(OpenMP_CXX_FOUND)
    target_link_libraries(omp_test_nested
        PRIVATE OpenMP::OpenMP_CXX
    )
    target_link_options(omp_test_nested
        PRIVATE ${OpenMP_CXX_FLAGS}
    )
endif()
