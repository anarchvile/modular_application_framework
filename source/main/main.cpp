#include "main.h"
#define PY_SSIZE_T_CLEAN

#include <iostream>
#include <math.h>

// Minimal implementation.
// Main's function's job is to load all of the necessary plug-ins at start time,
// continuously update currently-loaded plug-ins (which includes checking for
// user input and passing that data to the plug-ins that use it), and loading
// new/unloading currently-loaded plug-ins.

int main(int argc, char* argv[])
{
    pybind11::scoped_interpreter guard{}; // start the interpreter and keep it alive
    pybind11::module_ sys = pybind11::module_::import("sys");

    getPaths();
    addPathsToSys(sys);
    parseConfigFile(g_StartupFile);
    loadPlugins(sys);
    start(sys);
    stop(sys);
    unloadPlugins(sys);

    return 0;
}
