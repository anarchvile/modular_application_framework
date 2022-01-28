#pragma warning(disable:4522)
#include <pybind11/pybind11.h>

#include "helloWorld.h"
#include "runner.h"
#include "pluginManager.h"

namespace
{
    PluginManager* g_PlgsMan;
    std::string g_AppDir;
    HelloWorld* helloWorldPtr;
    HelloWorldInterface helloWorldInterface;
    Runner* runner;

    void load(size_t identifier)
    {
        g_PlgsMan = PluginManager::Instance(identifier);

        runner = (Runner*)g_PlgsMan->Load("runner");

        helloWorldPtr = (HelloWorld*)g_PlgsMan->Load("helloWorld");
        helloWorldInterface = helloWorldPtr->getInterface();
    }

    void unload()
    {
        g_PlgsMan->Unload("helloWorld");
        g_PlgsMan->Unload("runner");
    }

    const char* printHelloWorld()
    {
        return helloWorldInterface.printHelloWorld();
    }

    void start()
    {
        helloWorldPtr->start();
    }

    void stop()
    {
        helloWorldPtr->stop();
    }

    PYBIND11_MODULE(helloWorldPython, m) {
        m.doc() = "pybind11 helloWorld plugin"; // optional module docstring
        m.def("load", &load, "Load HelloWorld plugin via Python");
        m.def("print_hello_world",
            []()
            {
            return std::string(printHelloWorld());
            },
            "Prints a hello world statement");
        m.def("unload", &unload, "Unload HelloWorld plugin via Python");
        m.def("start", &start, "Start HelloWorld plugin by pushing a local function to runner for updating");
        m.def("stop", &stop, "Stop HelloWorld plugin from updating by popping off a local function from runner");
    }
}