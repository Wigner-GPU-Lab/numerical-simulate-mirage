cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
project(RungeKuttaRayBending)

#add_compile_options(-ggdb -D_GLIBCXX_DEBUG -std=c++17)
add_compile_options(-O2 -std=c++17)

add_definitions(-DEIGEN_MATRIX_PLUGIN="Matrix_initializer_list.h" -DEIGEN_ARRAY_PLUGIN="Array_initializer_list.h")

INCLUDE_DIRECTORIES ( "cli11/include/CLI"
                      "eigen3"
                      "png++"
                      "eigen-initializer_list/src" )
ADD_LIBRARY (RungeKuttaRayBendingLib SHARED RungeKuttaRayBending.cpp mathUtil.cpp)
target_link_libraries(RungeKuttaRayBendingLib quadmath png gsl pthread)

add_executable(main main.cpp simpleRaytracer.cpp)
target_link_libraries(main RungeKuttaRayBendingLib png gsl)

add_executable(eikonal eikonal.cpp)
target_link_libraries(eikonal RungeKuttaRayBendingLib png gsl)
