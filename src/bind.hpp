#pragma once

#include <functional>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define REGISTER_BINDING(CODE)                                                           \
    namespace {                                                                          \
    static const auto _ = []() {                                                         \
        PythonBindings::instance().register_binding([](pybind11::module_& m) { CODE; }); \
        return 0;                                                                        \
    }();                                                                                 \
    }

class PythonBindings
{
  public:

    static PythonBindings& instance();

    void register_binding(const std::function<void(pybind11::module_&)>& func);

    void bind_all(pybind11::module_& m);

  private:

    std::vector<std::function<void(pybind11::module_&)>> bindings;

    PythonBindings() = default;
    PythonBindings(const PythonBindings&) = delete;
    PythonBindings& operator=(const PythonBindings&) = delete;
};
