enable_testing()

find_package(Catch2 3)
find_package(Torch REQUIRED)

if(Catch2_FOUND)
    include(CTest)
    include(Catch)
    file(GLOB_RECURSE tests_cpps "*.cpp")
    add_executable(cpp_tests ${tests_cpps})
    target_link_libraries(
        cpp_tests PRIVATE core_objs Catch2::Catch2WithMain ${TORCH_LIBRARIES}
                          ${TORCH_PYTHON_LIBRARY}
    )
    catch_discover_tests(cpp_tests)
else()
    message(STATUS "Catch2 not found, skipping C++ tests")
endif()

find_package(
    Python3
    COMPONENTS Interpreter
    REQUIRED
)

file(GLOB PYTHON_TESTS "test_*.py")

foreach(py_test ${PYTHON_TESTS})
    message(STATUS "Found Python tests: ${py_test}")
    get_filename_component(test_name ${py_test} NAME_WE)
    add_test(NAME ${test_name} COMMAND ${Python3_EXECUTABLE} -m unittest tests.${test_name})
    set_tests_properties(
        ${test_name}
        PROPERTIES ENVIRONMENT
                   "PYTHONPATH=${PROJECT_SOURCE_DIR}"
                   WORKING_DIRECTORY
                   ${PROJECT_SOURCE_DIR}
                   LABELS
                   "python"
    )
endforeach()
