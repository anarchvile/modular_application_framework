#include "main.h"
#define PY_SSIZE_T_CLEAN

// Minimal implementation.
// Main's function job is to load all of the necessary plug-ins at start time,
// continuously update currently-loaded plug-ins (which includes checking for
// user input and passing that data to the plug-ins that use it), and loading
// new/unloading currently-loaded plug-ins.

// symlink:    C:\>mklink /D "c:\_my_projects\modular_application_framework\_build\x64\Release\plugins\helloWorld\scripts" "c:\_my_projects\modular_application_framework\examplePlugins\helloWorld\scripts"

int main(int argc, char* argv[])
{
    pybind11::scoped_interpreter guard{}; // start the interpreter and keep it alive
    pybind11::module_ sys = pybind11::module_::import("sys");

    getPaths();
    addPathsToSys(sys);
    parseConfigFile(g_StartupFile);
    loadPlugins(sys);
    start(sys);
    // Only need this if the config file is loading C++ plugins.
    //std::this_thread::sleep_for(std::chrono::milliseconds(4500));
    stop(sys);
    unloadPlugins(sys);

    return 0;
}
