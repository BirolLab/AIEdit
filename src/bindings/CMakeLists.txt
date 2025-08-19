set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
find_library(TORCH_PYTHON_LIBRARY torch_python PATH "${TORCH_INSTALL_PREFIX}/lib")
find_package(Threads REQUIRED)
find_package(Torch REQUIRED)
find_library(BTLLIB btllib REQUIRED)

file(GLOB bindings_cpps "*.cpp")

pybind11_add_module(core MODULE ${bindings_cpps})

target_link_libraries(
    core
    PRIVATE core_objs
            pybind11::module
            Threads::Threads
            ${TORCH_LIBRARIES}
            ${TORCH_PYTHON_LIBRARY}
            ${BTLLIB}
)

set(aiedit_path ${PROJECT_SOURCE_DIR}/aiedit)
set_target_properties(core PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${aiedit_path})

set(_PY_SITE ${Python_SITEARCH})
if(NOT _PY_SITE)
    set(_PY_SITE ${Python_SITELIB})
endif()
if(NOT _PY_SITE)
    message(FATAL_ERROR "Could not determine Python site-packages.")
endif()

install(
    TARGETS core
    LIBRARY DESTINATION ${_PY_SITE}/aiedit
    RUNTIME DESTINATION ${_PY_SITE}/aiedit
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/aiedit/
    DESTINATION ${_PY_SITE}/aiedit
    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "*.pt"
)

find_program(PYBIND11_STUBGEN NAMES pybind11-stubgen)
if(PYBIND11_STUBGEN)
    add_custom_command(
        TARGET core
        DEPENDS core POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${aiedit_path} ${PYBIND11_STUBGEN} --output-dir
                ${aiedit_path} core
        COMMENT "Generating Python stubs"
    )
endif()
