#pragma warning(disable:4522)
#include <pybind11/pybind11.h>

#include "goodbyeWorld.h"
#include "runner.h"
#include "pluginManager.h"

namespace
{
    PluginManager* g_PlgsMan;
    std::string g_AppDir;
    GoodbyeWorld* goodbyeWorldPtr;
    GoodbyeWorldInterface goodbyeWorldInterface;
    Runner* runner;

    void load(size_t identifier)
    {
        g_PlgsMan = PluginManager::Instance(identifier);

        runner = (Runner*)g_PlgsMan->Load("runner");

        goodbyeWorldPtr = (GoodbyeWorld*)g_PlgsMan->Load("goodbyeWorld");
        goodbyeWorldInterface = goodbyeWorldPtr->getInterface();
    }

    void unload()
    {
        g_PlgsMan->Unload("goodbyeWorld");
        g_PlgsMan->Unload("runner");
    }

    const char* printGoodbyeWorld()
    {
        return goodbyeWorldInterface.printGoodbyeWorld();
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
            []()
            {
            return std::string(printGoodbyeWorld());
            },
            "Prints a goodbye world statement");
        m.def("unload", &unload, "Unload GoodbyeWorld plugin via Python");
        m.def("start", &start, "Start GoodbyeWorld plugin by pushing a local function to runner for updating");
        m.def("stop", &stop, "Stop GoodbyeWorld plugin from updating by popping off a local function from runner");
    }
}