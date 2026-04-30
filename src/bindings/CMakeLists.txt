set(PYBIND11_FINDPYTHON ON)
find_package(pybind11 CONFIG REQUIRED)
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
            ${BTLLIB}
)

set(aiedit_path ${PROJECT_SOURCE_DIR}/aiedit)

install(
    TARGETS core
    LIBRARY DESTINATION aiedit
    RUNTIME DESTINATION aiedit
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/aiedit/
    DESTINATION aiedit
    FILES_MATCHING
    PATTERN "*.py"
    PATTERN "*.pt"
)

find_program(PYBIND11_STUBGEN NAMES pybind11-stubgen)
if(PYBIND11_STUBGEN)
    add_custom_command(
        TARGET core
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=$<TARGET_FILE_DIR:core> 
                ${PYBIND11_STUBGEN} --output-dir ${CMAKE_SOURCE_DIR}/aiedit core
        COMMENT "Generating Python stubs"
    )
endif()
