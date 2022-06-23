cmake_minimum_required(VERSION 3.22)
project(AIEdit)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# -------------------------------- OPENMP FLAGS --------------------------------
find_package(OpenMP)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# ------------------------------- INCLUDE PATHS --------------------------------
include_directories(include)
include_directories(vendor/argparse/include)
include_directories(vendor/btllib/include)
include_directories(vendor/btllib/subprojects/sdsl-lite/include)
include_directories(vendor/btllib/subprojects/cpptoml/include)
include_directories(vendor/json/single_include)

# -------------------------------- SOURCE FILES --------------------------------
file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# -------------------------------- BUILD BTLLIB --------------------------------
if (NOT EXISTS ${PROJECT_BINARY_DIR}/btllib/lib/libbtllib.a)
    add_custom_command(
            OUTPUT ${PROJECT_BINARY_DIR}/btllib/lib/libbtllib.a
            COMMAND ${PROJECT_SOURCE_DIR}/vendor/btllib/compile --prefix ${PROJECT_BINARY_DIR}/btllib
            USES_TERMINAL
    )
    set_property(SOURCE main.cpp APPEND PROPERTY OBJECT_DEPENDS ${PROJECT_BINARY_DIR}/btllib/lib/libbtllib.a)
endif()

# ------------------------------- BUILD PROJECT --------------------------------
add_executable(AIEdit main.cpp ${SRC_FILES})
target_link_libraries(AIEdit ${PROJECT_BINARY_DIR}/btllib/lib/libbtllib.a)
