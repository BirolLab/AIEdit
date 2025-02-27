cmake_minimum_required(VERSION 3.18 FATAL_ERROR)
project(aiedit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
include_directories("${pybind11_INCLUDE_DIRS}")

find_package(Torch REQUIRED)
include_directories("${TORCH_INCLUDE_DIRS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

find_library(BTLLIB btllib REQUIRED)
find_library(TORCH_PYTHON_LIBRARY torch_python PATH "${TORCH_INSTALL_PREFIX}/lib")
link_libraries(
    pthread
    omp
    ${BTLLIB}
    ${TORCH_LIBRARIES}
    ${TORCH_PYTHON_LIBRARY}
)

include_directories(src)

pybind11_add_module(
    aiedit
    src/bind.cpp
    src/modules/edit_region_finder.cpp
    src/modules/edit.cpp
    src/modules/environment.cpp
    src/modules/kmer_model.cpp
    src/modules/region_editor.cpp
    src/modules/signature.cpp
)
