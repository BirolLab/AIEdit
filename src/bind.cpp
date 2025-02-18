#include <pybind11/pybind11.h>

void bind_edit(pybind11::module_&);
void bind_editor(pybind11::module_&);
void bind_environment(pybind11::module_&);
void bind_kmer_model(pybind11::module_&);
void bind_signature(pybind11::module_&);
void bind_state(pybind11::module_&);

PYBIND11_MODULE(aiedit, m)
{
    bind_edit(m);
    bind_editor(m);
    bind_environment(m);
    bind_kmer_model(m);
    bind_signature(m);
    bind_state(m);
}