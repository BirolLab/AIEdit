#pragma once

#include <functional>
#include <vector>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#define REGISTER_BINDING(CODE)                                                                  \
    namespace {                                                                                 \
    static const auto _ = []() {                                                                \
        PythonBindings::instance().register_binding([](pybind11::module_& m) { CODE; }, false); \
        return 0;                                                                               \
    }();                                                                                        \
    }

#define REGISTER_DEPENDENCY(CODE)                                                              \
    namespace {                                                                                \
    static const auto _ = []() {                                                               \
        PythonBindings::instance().register_binding([](pybind11::module_& m) { CODE; }, true); \
        return 0;                                                                              \
    }();                                                                                       \
    }

class PythonBindings
{
  public:

    static PythonBindings& instance();

    void register_binding(const std::function<void(pybind11::module_&)>& func, bool is_dependency);

    void bind_all(pybind11::module_& m);

  private:

    std::vector<std::function<void(pybind11::module_&)>> bindings;

    PythonBindings() = default;
    PythonBindings(const PythonBindings&) = delete;
    PythonBindings& operator=(const PythonBindings&) = delete;
};
