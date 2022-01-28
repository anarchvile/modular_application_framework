#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "common.h"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <mutex>
#include <sys/types.h>
#include <sys/stat.h>

#include "container.h"
#include "plugin.h"

#ifdef _WIN32
    #include <Windows.h>
#elif __linux__
    #include <dlfcn.h>
#else
    #error define your compiler
#endif

namespace
{
    std::mutex m;
}

// The plug-in manager is responsible for loading/unloading plug-ins into the program
// and performing some basic checks, such as checking whether a plug-in that
// is currently pending for loading is, in fact, already loaded.
// By design, only one instance of a plug-in can be loaded at a time.
class PluginManager
{
private:
    static PluginManager* m_instance;
    static std::pair<size_t, const char*> m_appDir;
    static Container* m_container;

    static void loadContainer();

    PluginManager();

public:
    ~PluginManager();

    static PluginManager* Instance(size_t identifier);
    static Plugin* Load(const char* pluginName);
    static void Load(const char* pluginName, Plugin* &ptr_plugin);
    static void Unload(const char* pluginName);
};

#endif // PLUGINMANAGER_H
