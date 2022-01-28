// Container is an internal plug-in that stores a few data containers (hence the name)
// with information about the external plug-ins in our system, such as how many references
// to a single plug-in exist, the handles to those plug-ins, etc. 
// This data is shared across all loaded plug-ins, and is accessible with the below functions.
// Writing to these data members is made thread-safe with mutex locking.

#include "stdafx.h" // this header needs to come first
#include <mutex>
#include "containerImpl.h"

#pragma data_seg("SHARED")
ContainerImpl* g_pmc = nullptr;
std::map<std::string, int> g_ref;
std::map<std::string, void*> g_plugins;
#ifdef _WIN32
std::map<std::string, HMODULE> g_handles;
#elif __linux__
std::map<std::string, void*> g_handles;
#endif
std::map<std::string, void*> g_events;
std::map<std::string, size_t> g_eventsRef;
#pragma data_seg()

std::mutex m;

// reference counter
std::map<std::string, int> &ContainerImpl::getRefCount()
{
    return g_ref;
}

void ContainerImpl::addRefCount(std::string pluginName)
{
    m.lock();
    if (g_ref.find(pluginName) != g_ref.end())
    {
        g_ref.at(pluginName) += 1;
    }
    else
    {
        g_ref.insert(std::pair<std::string, int>(pluginName, 1));
    }
    m.unlock();
}

void ContainerImpl::subtractRefCount(std::string pluginName)
{
    m.lock();
    if (g_ref.find(pluginName) != g_ref.end())
    {
        g_ref.at(pluginName) -= 1;
    }
    m.unlock();
}

void ContainerImpl::eraseRefCount(std::string pluginName)
{
    m.lock();
    g_ref.erase(pluginName);
    m.unlock();
}

// plug-in pointers
std::map<std::string, void*> &ContainerImpl::getPlugins()
{
    return g_plugins;
}

void ContainerImpl::addPlugin(std::string pluginPath, void* ptr_plugin)
{
    m.lock();
    g_plugins[pluginPath] = ptr_plugin;
    m.unlock();
}

void ContainerImpl::erasePlugin(std::string pluginPath)
{
    m.lock();
    //delete g_plugins[pluginPath];
    g_plugins.erase(pluginPath);
    m.unlock();
}

// plug-in handles
#ifdef _WIN32
std::map<std::string, HMODULE> &ContainerImpl::getHandles()
{
    return g_handles;
}
void ContainerImpl::addHandle(std::string pluginPath, HMODULE handle)
{
    m.lock();
    g_handles[pluginPath] = handle;
    m.unlock();
}
void ContainerImpl::eraseHandle(std::map<std::string, HMODULE>::iterator it)
{
    m.lock();
    g_handles.erase(it);
    m.unlock();
}
#elif __linux__
std::map<std::string, void*> &ContainerImpl::getHandles()
{
    return g_handles;
}
void ContainerImpl::addHandle(std::string pluginPath, void* handle)
{
    m.lock();
    g_handles[pluginPath] = handle;
    m.unlock();
}
void ContainerImpl::eraseHandle(std::map<std::string, void*>::iterator it)
{
    m.lock();
    g_handles.erase(it);
    m.unlock();
}
#endif

std::map<std::string, size_t> &ContainerImpl::getEventRefCount()
{
    return g_eventsRef;
}

void ContainerImpl::addEventRefCount(std::string name)
{
    m.lock();
    if (g_eventsRef.find(name) != g_eventsRef.end())
    {
        g_eventsRef.at(name) += 1;
    }
    else
    {
        g_eventsRef.insert(std::pair<std::string, int>(name, 1));
    }
    m.unlock();
}

void ContainerImpl::subtractEventRefCount(std::string name)
{
    m.lock();
    if (g_eventsRef.find(name) != g_eventsRef.end())
    {
        g_eventsRef.at(name) -= 1;
    }
    m.unlock();
}

void ContainerImpl::eraseEventRefCount(std::string name)
{
    m.lock();
    g_eventsRef.erase(name);
    m.unlock();
}

std::map<std::string, void*>& ContainerImpl::getEvents()
{
    return g_events;
}

void ContainerImpl::addEvent(std::string name, void* ptr_event)
{
    m.lock();
    g_events[name] = ptr_event;
    m.unlock();
}

void ContainerImpl::eraseEvent(std::string name)
{
    m.lock();
    //delete g_events[name];
    g_events.erase(name);
    m.unlock();
}

// create a container instance
extern "C" CONTAINER ContainerImpl* Create()
{
    if (g_pmc != nullptr)
    {
        return g_pmc;
    }

    else
    {
        g_pmc = new ContainerImpl;
        return g_pmc;
    }
}

#pragma comment(linker, "/section:SHARED,RWS")
