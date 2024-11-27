cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(aiedit)
set(CMAKE_CXX_STANDARD 17)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
include_directories("${pybind11_INCLUDE_DIRS}")

find_package(Torch REQUIRED)
include_directories("${TORCH_INCLUDE_DIRS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

find_library(BTLLIB btllib)
find_library(TORCH_PYTHON_LIBRARY torch_python PATH "${TORCH_INSTALL_PREFIX}/lib")
link_libraries(pthread omp ${BTLLIB} ${TORCH_LIBRARIES} ${TORCH_PYTHON_LIBRARY})

add_executable(aiedit src/main.cpp)
pybind11_add_module(aiedit_torch_extensions src/torch_extensions.cpp)