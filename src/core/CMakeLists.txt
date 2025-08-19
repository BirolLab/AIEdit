find_package(Torch REQUIRED)

file(GLOB core_cpps "*.cpp")

add_library(
    core_objs OBJECT
    ${core_cpps}
)

target_include_directories(
    core_objs PUBLIC ${PROJECT_SOURCE_DIR}/src ${TORCH_INCLUDE_DIRS}
)

target_compile_options(core_objs PRIVATE ${TORCH_CXX_FLAGS})

set_target_properties(core_objs PROPERTIES POSITION_INDEPENDENT_CODE ON)
