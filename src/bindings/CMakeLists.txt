set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
find_library(TORCH_PYTHON_LIBRARY torch_python PATH "${TORCH_INSTALL_PREFIX}/lib")
find_package(Threads REQUIRED)
find_package(Torch REQUIRED)
find_library(BTLLIB btllib REQUIRED)

file(GLOB bindings_cpps "*.cpp")

pybind11_add_module(core MODULE
    ${bindings_cpps}
)

target_link_libraries(core PRIVATE core_objs
    pybind11::module
    Threads::Threads
    ${TORCH_LIBRARIES}
    ${TORCH_PYTHON_LIBRARY}
    ${BTLLIB}
)

set(aiedit_path ${PROJECT_SOURCE_DIR}/aiedit)
set_target_properties(core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${aiedit_path})

add_custom_command(
    TARGET core
    DEPENDS core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${aiedit_path} pybind11-stubgen --output-dir
            ${aiedit_path} core
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${aiedit_path} black ${aiedit_path}/core.pyi
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${aiedit_path} isort ${aiedit_path}/core.pyi
    COMMENT "Generating Python stubs"
)