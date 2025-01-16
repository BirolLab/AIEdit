cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(aiedit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_package(Torch REQUIRED)
include_directories("${TORCH_INCLUDE_DIRS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

find_library(
    BTLLIB
    btllib
    REQUIRED
)
find_library(
    TORCH_PYTHON_LIBRARY
    torch_python
    PATH
    "${TORCH_INSTALL_PREFIX}/lib"
)
link_libraries(
    pthread
    omp
    ${BTLLIB}
    ${TORCH_LIBRARIES}
    ${TORCH_PYTHON_LIBRARY}
)

include_directories("${PROJECT_SOURCE_DIR}/src")

file(
    GLOB_RECURSE
    aiedit_sources
    ${CMAKE_SOURCE_DIR}/src/*/*.cpp
)
foreach(FILE ${aiedit_sources})
    message(STATUS "Found main source file: ${FILE}")
endforeach()
add_library(aiedit_objects OBJECT ${aiedit_sources})

add_executable(aiedit src/main.cpp $<TARGET_OBJECTS:aiedit_objects>)


# Tests

# find_package(Catch2)
# 
# if(${Catch2_FOUND})
#     file(
#         GLOB
#         test_files
#         "tests/test_*.cpp"
#     )
#     foreach(FILE ${test_files})
#         message(STATUS "Found test source file: ${FILE}")
#     endforeach()
#     add_executable(aiedit_tests ${test_files} $<TARGET_OBJECTS:aiedit_objects>)
#     target_include_directories(aiedit_tests PRIVATE "${PROJECT_SOURCE_DIR}/src")
#     target_link_libraries(aiedit_tests PRIVATE Catch2::Catch2WithMain)
#     include(Catch)
#     catch_discover_tests(aiedit_tests)
# else(${Catch2_FOUND})
#     message(WARNING "Compiling tests requires Catch2 to be installed")
# endif(${Catch2_FOUND})
