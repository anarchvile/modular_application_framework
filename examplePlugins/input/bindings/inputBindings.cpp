#include <pybind11/functional.h>
#include "input.h"
#include "pluginManager.h"

namespace
{
    PluginManager* g_PlgsMan;
    std::string g_AppDir;
    Input* input;
    bool isAsync;

    void load(size_t identifier)
    {
        g_PlgsMan = PluginManager::Instance(identifier);
        input = (Input*)g_PlgsMan->Load("input");
    }

    void unload()
    {
        g_PlgsMan->Unload("input");
        g_PlgsMan->requestDelete();
    }

#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
    void push(const char* name, int keyboardUpdatePriority, int mouseUpdatePriority, bool isKeyboardUpdateFunc, std::function<void(InputData)>& func)
    {
        // assemble descriptor here.
        InputDesc desc;
        desc.name = name;
        desc.keyboardUpdatePriority = keyboardUpdatePriority;
        desc.mouseUpdatePriority = mouseUpdatePriority;
        desc.cKeyboardUpdate = nullptr;
        desc.cMouseUpdate = nullptr;
        if (isKeyboardUpdateFunc)
        {
            desc.pyKeyboardUpdate = func;
            desc.pyMouseUpdate = nullptr;
        }
        else
        {
            desc.pyKeyboardUpdate = nullptr;
            desc.pyMouseUpdate = func;
        }
        // call Input::push
        input->push(desc);
    }
    void pop(const char* name, int keyboardUpdatePriority, int mouseUpdatePriority, bool isKeyboardUpdateFunc, std::function<void(InputData)>& func)
    {
        // assemble descriptor here.
        InputDesc desc;
        desc.name = name;
        desc.keyboardUpdatePriority = keyboardUpdatePriority;
        desc.mouseUpdatePriority = mouseUpdatePriority;
        desc.cKeyboardUpdate = nullptr;
        desc.cMouseUpdate = nullptr;
        if (isKeyboardUpdateFunc)
        {
            desc.pyKeyboardUpdate = func;
            desc.pyMouseUpdate = nullptr;
        }
        else
        {
            desc.pyKeyboardUpdate = nullptr;
            desc.pyMouseUpdate = func;
        }
        // call Input::pop
        input->pop(desc);
    }
#endif
    void start()
    {
        input->start2();
        pybind11::gil_scoped_acquire acquire;
    }

    void stop()
    {
        input->stop();
    }
}
PYBIND11_MODULE(inputPython, m) {
    m.doc() = "pybind11 input plugin"; // optional module docstring
    pybind11::class_<InputData>(m, "InputData"); // create python bindings for InputData struct
    m.def("load", &load, "Load Input plugin");
    m.def("unload", &unload, "Unload Input plugin");
#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
    m.def("push", &push, "Register a function to be updated on via Input");
    m.def("pop", &pop, "Unregister a function from being updated via Input");
#endif
    m.def("start", &start, pybind11::call_guard<pybind11::gil_scoped_release>(), "Starts the input update cycle");
    m.def("stop", &stop, "Stops the input update cycle");
}