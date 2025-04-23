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
    core
    src/bind.cpp
    src/bindings/buffer2d.cpp
    src/bindings/edit_region_finder.cpp
    src/bindings/edit.cpp
    src/bindings/editor.cpp
    src/bindings/kmer_model.cpp
    src/bindings/model_interface.cpp
    src/bindings/seed_generator.cpp
    src/bindings/utils.cpp
    src/core/buffer2d.cpp
    src/core/edit_region_finder.cpp
    src/core/edit.cpp
    src/core/editor.cpp
    src/core/kmer_model.cpp
    src/core/model_interface.cpp
    src/core/seed_generator.cpp
    src/core/utils.cpp
)

set(aiedit_path ${PROJECT_SOURCE_DIR}/aiedit)
set_target_properties(core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${aiedit_path})

add_custom_command(
    TARGET core
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${aiedit_path} pybind11-stubgen --output-dir
            ${aiedit_path} core DEPENDS core
    COMMENT "Generating Python stubs"
)
