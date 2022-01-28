#include <pybind11/functional.h>
#include "runner.h"
#include "pluginManager.h"

namespace
{
    PluginManager* g_PlgsMan;
    std::string g_AppDir;
    Runner* runner;

    void load(size_t identifier)
    {
        g_PlgsMan = PluginManager::Instance(identifier);
        runner = (Runner*)g_PlgsMan->Load("runner");
    }

    void unload()
    {
        g_PlgsMan->Unload("runner");
    }

    void push(const char* name, int priority, std::function<void(double)>& func)
    {
        // assemble descriptor here.
        RunnerDesc desc;
        desc.name = name;
        desc.priority = priority;
        desc.cUpdate = nullptr;
        desc.pyUpdate = func;
        // call Runner::push
        runner->push(desc);
    }

    void pop(const char* name, int priority, std::function<void(double)>& func)
    {
        // assemble descriptor here.
        RunnerDesc desc;
        desc.name = name;
        desc.priority = priority;
        desc.cUpdate = nullptr;
        desc.pyUpdate = func;
        // call Runner::pop
        runner->pop(desc);
    }

    void start()
    {
        runner->start2();
        pybind11::gil_scoped_acquire acquire;
    }

    void stop()
    {
        runner->stop2();
    }
}
PYBIND11_MODULE(runnerPython, m) {
    m.doc() = "pybind11 runner plugin"; // optional module docstring
    m.def("load", &load, "Load Runner plugin");
    m.def("unload", &unload, "Unload Runner plugin");
    m.def("push", &push, "Register a function to be updated on the loop by Runner");
    m.def("pop", &pop, "Unregister a function from being updated on the loop by Runner");
    m.def("start", &start, pybind11::call_guard<pybind11::gil_scoped_release>(), "Starts the runner update cycle");
    m.def("stop", &stop, "Stops the runner update cycle");
}