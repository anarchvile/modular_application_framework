#pragma warning(disable:4522)
#include <pybind11/pybind11.h>

#include "goodbyeWorld.h"
#include "pluginManager.h"
#include "input.h"

namespace
{
    PluginManager* g_PlgsMan;
    GoodbyeWorld* goodbyeWorldPtr;
    GoodbyeWorldInterface goodbyeWorldInterface;

    void load(size_t identifier)
    {
        g_PlgsMan = PluginManager::Instance(identifier);

        goodbyeWorldPtr = (GoodbyeWorld*)g_PlgsMan->Load("goodbyeWorld");
        goodbyeWorldInterface = goodbyeWorldPtr->getInterface();
    }

    void unload()
    {
        g_PlgsMan->Unload("goodbyeWorld");
        g_PlgsMan->requestDelete();
    }

    void printGoodbyeWorld(double dt)
    {
        std::cout << "Print: " << goodbyeWorldInterface.goodbyeWorldFunc() << ", " << dt << std::endl;
    }

    void printGoodbyeWorld(InputData id)
    {
        std::cout << "Print: " << goodbyeWorldInterface.goodbyeWorldFunc() << ", " << "InputData" << std::endl;
    }

    std::string returnGoodbyeWorld()
    {
        return ("Return " + std::string(goodbyeWorldInterface.goodbyeWorldFunc()));
    }

    void anotherGoodbyeWorldFunc(double dt)
    {
        goodbyeWorldInterface.anotherGoodbyeWorldFunc();
    }

    void start()
    {
        goodbyeWorldPtr->start();
    }

    void stop()
    {
        goodbyeWorldPtr->stop();
    }

    PYBIND11_MODULE(goodbyeWorldPython, m) {
        m.doc() = "pybind11 goodbyeWorld plugin"; // optional module docstring
        m.def("load", &load, "Load GoodbyeWorld plugin via Python");
        m.def("print_goodbye_world",
            [](double dt)
            {
            printGoodbyeWorld(dt);
            },
            "Prints a goodbye world statement");
        m.def("print_goodbye_world",
            [](InputData id)
            {
            printGoodbyeWorld(id);
            },
            "Prints a goodbye world statement");
        m.def("another_goodbye_world_func", &anotherGoodbyeWorldFunc, "A random goodybeWorld function");
        m.def("return_goodbye_world",
            []()
            {
            return returnGoodbyeWorld();
            },
            "Returns a goodbye world statement");
        m.def("unload", &unload, "Unload GoodbyeWorld plugin via Python");
        m.def("start", &start, "Start GoodbyeWorld plugin by pushing a local function to runner for updating");
        m.def("stop", &stop, "Stop GoodbyeWorld plugin from updating by popping off a local function from runner");
    }
}