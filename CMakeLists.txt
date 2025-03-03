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
    src/core/buffer2d.cpp
    src/core/edit_region_finder.cpp
    src/core/edit.cpp
    src/core/editor.cpp
    src/core/kmer_model.cpp
    src/core/model_interface.cpp
)

file(GLOB PY_FILES "src/core/*.py")

# Add a custom command to copy each .py file to the output directory
foreach(PY_FILE ${PY_FILES})
    add_custom_command(
        TARGET aiedit
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PY_FILE} ${CMAKE_BINARY_DIR}
        COMMENT "Copying ${PY_FILE} to ${CMAKE_BINARY_DIR}"
    )
endforeach()

add_custom_command(
    TARGET aiedit
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_BINARY_DIR} pybind11-stubgen --output-dir
            ${CMAKE_BINARY_DIR} aiedit DEPENDS aiedit
    COMMENT "Generating Python stubs"
)
