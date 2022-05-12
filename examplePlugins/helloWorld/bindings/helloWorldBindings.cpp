// Python bindings for the helloWorld plugin.

#pragma warning(disable:4522)
#include <pybind11/pybind11.h>

#include "helloWorld.h"
#include "pluginManager.h"
#include "input.h"

namespace
{
    PluginManager* g_PlgsMan;
    std::string g_AppDir;
    HelloWorld* helloWorldPtr;
    HelloWorldInterface helloWorldInterface;

    void load(size_t identifier)
    {
        g_PlgsMan = PluginManager::Instance(identifier);
        helloWorldPtr = (HelloWorld*)g_PlgsMan->Load("helloWorld");
        helloWorldInterface = helloWorldPtr->getInterface();
    }

    void unload()
    {
        g_PlgsMan->Unload("helloWorld");
        g_PlgsMan->requestDelete();
    }

    void printHelloWorld(InputData id)
    {
        std::cout << "Print: " << helloWorldInterface.helloWorldFunc() << ", InputData" << std::endl;
    }

    void printHelloWorld(double dt)
    {
        std::cout << "Print: " << helloWorldInterface.helloWorldFunc() << ", " << dt << std::endl;
    }

    std::string returnHelloWorld()
    {
        return ("Return " + std::string(helloWorldInterface.helloWorldFunc()));
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
            [](double dt)
            {
            printHelloWorld(dt);
            },
            "Prints a hello world statement");
        m.def("print_hello_world",
            [](InputData id)
            {
            printHelloWorld(id);
            },
            "Prints a hello world statement");
        m.def("return_hello_world",
            []()
            {
            return returnHelloWorld();
            },
            "Returns a hello world statement");
        m.def("unload", &unload, "Unload HelloWorld plugin via Python");
        m.def("start", &start, "Start HelloWorld plugin by pushing a local function to runner for updating");
        m.def("stop", &stop, "Stop HelloWorld plugin from updating by popping off a local function from runner");
    }
}