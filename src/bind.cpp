#include "bind.hpp"

PythonBindings& PythonBindings::instance()
{
    static PythonBindings instance;
    return instance;
}

void PythonBindings::register_binding(const std::function<void(pybind11::module_&)>& func)
{
    bindings.push_back(func);
}

void PythonBindings::bind_all(pybind11::module_& m)
{
    for (const auto& func : bindings) {
        func(m);
    }
}

PYBIND11_MODULE(core, m)
{
    PythonBindings::instance().bind_all(m);
}