// Note: For now, there is no dependency tracking between plugins, so they'll be loaded in the order defined
// by the config files. Any plugins that attempt to register to runner before runner is loaded will safely fail
// in doing so.

#ifndef MAIN_H
#define MAIN_H

#include "pch.h"

#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
#elif __linux
    #include <unistd.h>
    #include <linux/limits.h>
#endif

#include <functional>
#include <algorithm>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <sys/stat.h>
#include "ghc/filesystem.hpp"

#include "helloWorld.h"
#include "goodbyeWorld.h"
#include "concurrentLoading.h"
#include "runner.h"
#include "pluginManager.h"

#include <pybind11/embed.h>

namespace
{
    PluginManager* g_PlgsMan;
    std::string g_AppDir, g_PlgsDir;
    std::string g_StartupFile;
    std::vector<std::string> g_PlgNames;
    std::vector<std::string> g_PlgPaths;
    std::vector<Plugin*> g_PlgPtrs;

    #ifdef _WIN32
    char delimiter = '\\';
    #elif __linux__
    char delimiter = '/';
    #endif
}

// set the .exe path to navigate to the configuration file
// and the plugins folder (where all plugins should live)

void getPaths(const char* appDir = nullptr)
{
    std::string exeDir;
    if (appDir == nullptr)
    {
        // obtain the .exe absolute path
        #ifdef _WIN32
        WCHAR wPath[MAX_PATH];
        GetModuleFileNameW(NULL, wPath, MAX_PATH);
        char cPath[MAX_PATH];
        char DefChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, wPath, -1, cPath, MAX_PATH, &DefChar, NULL);
        exeDir = cPath;
        #elif __linux__
        char cPath[PATH_MAX];
        size_t count = readlink("/proc/self/exe", cPath, PATH_MAX);
        exeDir = cPath;
        #endif

        // use the executable's path to navigate to the
        // plugin folder and the configuration file
        size_t cnt = exeDir.length() - 1;
        while (exeDir[cnt] != delimiter)
        {
            --cnt;
            exeDir.pop_back();
        }
    }

    else
    {
        exeDir = std::string(appDir);
    }

    g_AppDir = exeDir;

    g_PlgsDir = exeDir + ".." + delimiter + "plugins" + delimiter;

    g_StartupFile = exeDir + ".." + delimiter + "startup";
}

void addPathsToSys(pybind11::module_ sys)
{
    std::string includeDir = g_AppDir + ".." + delimiter + "include" + delimiter;
    auto _o = sys.attr("path").attr("append")(includeDir);

    for (const auto& dirEntry1 : ghc::filesystem::directory_iterator(g_PlgsDir))
    {
        for (const auto& dirEntry2 : ghc::filesystem::directory_iterator(dirEntry1.path().string()))
        {
            if (dirEntry2.path().filename().string() == "bindings" || dirEntry2.path().filename().string() == "scripts")
            {
                pybind11::object sysPathAppendObj = sys.attr("path").attr("append")(dirEntry2.path().string());
            }
        }
    }
}

// parse the configuration (or .py) file to determine what plugins should be
// registered in the update loop of the application during startup
void parseConfigFile(std::string filePath)
{
    std::string configFilePath = filePath + ".cfg";
    std::string pyFilePath = filePath + ".py";
    std::ifstream cFile(configFilePath);
    std::ifstream pFile(pyFilePath);
    if (cFile.is_open())
    {
        std::string line;
        while (getline(cFile, line))
        {
            line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
            if (line[0] == '#' || line.empty())
            {
                continue;
            }

            size_t delimiterPos = line.find("=");
            std::string type = line.substr(0, delimiterPos);
            if (type == "load_plugins")
            {
                std::string plgName;
                for (size_t i = delimiterPos + 1; i <= line.length(); ++i)
                {
                    if (line[i] == ',' || i == line.length())
                    {
                        g_PlgNames.push_back(plgName);
                        plgName.clear();
                    }
                    else
                    {
                        plgName.push_back(line[i]);
                    }
                }
            }

            else if (type == "config_dirs")
            {
                std::string _configPath;
                for (size_t i = delimiterPos + 1; i <= line.length(); ++i)
                {
                    if (line[i] == ',' || i == line.length())
                    {
                        _configPath = _configPath + delimiter + "startup" + delimiter + "startup";
                        parseConfigFile(_configPath);
                        _configPath.clear();
                    }
                    else
                    {
                        _configPath.push_back(line[i]);
                    }
                }
            }
        }
    }

    else if (pFile.is_open())
    {
        std::string _s = delimiter + "startup.py";
        size_t pos = pyFilePath.find(_s);
        if (pos != std::string::npos)
        {
            // If found then erase it from string
            pyFilePath.erase(pos, _s.length());
        }

        pybind11::module_ sys = pybind11::module_::import("sys");
        pybind11::object sysPathAppendObj = sys.attr("path").attr("append")(pyFilePath);
        pybind11::module_ pyStartup = pybind11::module_::import("startup");
    }

    else
    {
        std::cerr << "Couldn't open config file/python module for reading.\n";
    }
}

void initialize(pybind11::module_ sys)
{
    g_PlgsMan = PluginManager::Instance(reinterpret_cast<size_t>(g_AppDir.c_str()));

    // The Runner plug-in implements a game engine-style update loop.
    // Other plug-ins can subscribe their functions to the Runner.
    // The Runner will call them at every tick.

    // Load all plugins listed in config file.
    for (unsigned int i = 0; i < g_PlgNames.size(); ++i)
    {
        std::string pyPluginDir = g_PlgsDir + g_PlgNames[i] + delimiter + "scripts";
        std::string pyPluginPath = pyPluginDir + delimiter + g_PlgNames[i] + ".py";
        // If there exists a valid .py plugin, we assume that all initialization/updating/releasing behavior is handled
        // by that python plugin, so we delegate those tasks to it and call its relevant functions. If no such python
        // file exists, then try to load the C++ plugin instead.
        struct stat buffer;
        if (stat(pyPluginPath.c_str(), &buffer) == 0)
        {
            std::string _plgName = g_PlgNames[i];
            pybind11::object sysPathAppendObj = sys.attr("path").attr("append")(pyPluginDir);
            pybind11::module_ pyPlugin = pybind11::module_::import(_plgName.c_str());
            _plgName[0] = std::toupper(_plgName[0]);
            pybind11::object pyObject = pyPlugin.attr(_plgName.c_str());
            auto _o1 = pyObject.attr("__init__")(pyObject);
            auto _o2 = pyObject.attr("initialize")(pyObject, reinterpret_cast<size_t>(g_AppDir.c_str()));
            g_PlgPtrs.push_back(nullptr);
        }

        else
        {
            g_PlgPtrs.push_back(g_PlgsMan->Load(g_PlgNames[i].c_str()));
        }
    }
}

// global release (unload) function for all plug-ins
void release(pybind11::module_ sys)
{
    for (unsigned int i = 0; i < g_PlgNames.size(); ++i)
    {
        std::string pyPluginDir = g_PlgsDir + g_PlgNames[i] + delimiter + "scripts";
        std::string pyPluginPath = pyPluginDir + delimiter + g_PlgNames[i] + ".py";

        struct stat buffer;
        if (stat(pyPluginPath.c_str(), &buffer) == 0)
        {
            std::string _plgName = g_PlgNames[i];
            pybind11::object sysPathAppendObj = sys.attr("path").attr("append")(pyPluginDir);
            pybind11::module_ pyPlugin = pybind11::module_::import(_plgName.c_str());
            _plgName[0] = std::toupper(_plgName[0]);
            pybind11::object pyObject = pyPlugin.attr(_plgName.c_str());
            auto _o = pyObject.attr("release")(pyObject);
        }

        else
        {
            g_PlgsMan->Unload(g_PlgNames[i].c_str());
        }
    }
}

// global update function for all loaded plug-ins
void start(pybind11::module_ sys)
{
    for (unsigned int i = 0; i < g_PlgNames.size(); ++i)
    {
        std::string pyPluginDir = g_PlgsDir + g_PlgNames[i] + delimiter + "scripts";
        std::string pyPluginPath = pyPluginDir + delimiter + g_PlgNames[i] + ".py";
        // If we're loading python plugins, then assume all start functions will be called via python.
        // Otherwise call the C++ equivalent start functions for each plugin.
        struct stat buffer;
        if (stat(pyPluginPath.c_str(), &buffer) == 0)
        {
            std::string _plgName = g_PlgNames[i];
            pybind11::object sysPathAppendObj = sys.attr("path").attr("append")(pyPluginDir);
            pybind11::module_ pyPlugin = pybind11::module_::import(_plgName.c_str());
            _plgName[0] = std::toupper(_plgName[0]);
            pybind11::object pyObject = pyPlugin.attr(_plgName.c_str());
            auto _o = pyObject.attr("start")(pyObject);
        }
        else
        {
            g_PlgPtrs[i]->start();
        }
    }
}

void stop(pybind11::module_ sys)
{
    for (unsigned int i = 0; i < g_PlgNames.size(); ++i)
    {
        std::string pyPluginDir = g_PlgsDir + g_PlgNames[i] + delimiter + "scripts";
        std::string pyPluginPath = pyPluginDir + delimiter + g_PlgNames[i] + ".py";
        // If we're loading python plugins, then assume all start functions will be called via python.
        // Otherwise call the C++ equivalent start functions for each plugin.
        struct stat buffer;
        if (stat(pyPluginPath.c_str(), &buffer) == 0)
        {
            std::string _plgName = g_PlgNames[i];
            pybind11::object sysPathAppendObj = sys.attr("path").attr("append")(pyPluginDir);
            pybind11::module_ pyPlugin = pybind11::module_::import(_plgName.c_str());
            _plgName[0] = std::toupper(_plgName[0]);
            pybind11::object pyObject = pyPlugin.attr(_plgName.c_str());
            auto _o = pyObject.attr("stop")(pyObject);
        }
        else
        {
            g_PlgPtrs[i]->stop();
        }
    }
}

// initialize at startup whatever plugins the configuration file specified
void loadPlugins(pybind11::module_ sys)
{
    std::cout << "loadPlugins has been called" << std::endl;
    initialize(sys);
}

void unloadPlugins(pybind11::module_ sys)
{
    std::cout << "unloadPlugins has been called" << std::endl;
    release(sys);
}

#endif // MAIN_H
