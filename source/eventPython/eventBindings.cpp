#include <pybind11/functional.h>
#include "event.h"

namespace
{
    void eventTestFunc()
    {
        std::cout << "a test event function using eventPythonBindings" << std::endl;
    }
}

PYBIND11_MODULE(eventPython, m) 
{
    m.doc() = "pybind11 runner plugin"; // optional module docstring
    m.def("eventTestFunc", &eventTestFunc, "Test event function");
}