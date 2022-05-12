// Container is an internal plug-in that stores a few data containers (hence the name)
// with information about the external plug-ins in our system, such as how many references
// to a single plug-in exist, the handles to those plug-ins, etc. 
// This data is shared across all loaded plug-ins, and is accessible with the below functions.
// Writing to these data members is made thread-safe with mutex locking.

#include "stdafx.h" // this header needs to come first
#include <mutex>
#include "containerImpl.h"
#include "pluginManager.h"

// Share this plugin (.dll or .so) across the entire application.
#pragma data_seg("SHARED")
size_t g_exeDir = 0;
ContainerImpl* g_pmc = nullptr;
size_t g_PlgsManRef = 0;
void* g_PlgsMan = nullptr;
std::map<std::string, size_t> g_pluginsRef;
std::map<std::string, void*> g_plugins;
#ifdef _WIN32
std::map<std::string, HMODULE> g_handles;
#elif __linux__
std::map<std::string, void*> g_handles;
#endif
std::map<std::string, size_t> g_eventsRef;
std::map<std::string, void*> g_events;
std::map<std::string, size_t> g_eventStreamsRef;
std::map<std::string, void*> g_eventStreams;
#pragma data_seg()

std::recursive_mutex m_lock;

size_t ContainerImpl::getExeDir()
{
    return g_exeDir;
}

void ContainerImpl::setExeDir(size_t identifier)
{
    g_exeDir = identifier;
}

size_t& ContainerImpl::getPlgManRefCount()
{
    return g_PlgsManRef;
}

void ContainerImpl::addPlgManRefCount()
{
    ++g_PlgsManRef;
}

void ContainerImpl::subtractPlgManRefCount()
{
    --g_PlgsManRef;
}

void* ContainerImpl::ContainerImpl::getPlgMan()
{
    return g_PlgsMan;
}

void ContainerImpl::setPlgMan(void* plgManPtr)
{
    g_PlgsMan = plgManPtr;
}

// Reference counter for plugins.
std::map<std::string, size_t> &ContainerImpl::getPluginRefCount()
{
    return g_pluginsRef;
}

void ContainerImpl::addPluginRefCount(std::string pluginName)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (g_pluginsRef.find(pluginName) != g_pluginsRef.end())
    {
        g_pluginsRef.at(pluginName) += 1;
    }
    else
    {
        g_pluginsRef.insert(std::pair<std::string, size_t>(pluginName, 1));
    }
}

void ContainerImpl::subtractPluginRefCount(std::string pluginName)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (g_pluginsRef.find(pluginName) != g_pluginsRef.end())
    {
        g_pluginsRef.at(pluginName) -= 1;
    }
}

void ContainerImpl::erasePluginRefCount(std::string pluginName)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_pluginsRef.erase(pluginName);
}

// Plug-in pointers.
std::map<std::string, void*>& ContainerImpl::getPlugins()
{
    return g_plugins;
}

void ContainerImpl::addPlugin(std::string pluginPath, void* ptr_plugin)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_plugins[pluginPath] = ptr_plugin;
}

void ContainerImpl::erasePlugin(std::string pluginPath)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_plugins.erase(pluginPath);
}

// Plug-in handles
#ifdef _WIN32
std::map<std::string, HMODULE>& ContainerImpl::getHandles()
{
    return g_handles;
}
void ContainerImpl::addHandle(std::string pluginPath, HMODULE handle)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_handles[pluginPath] = handle;
}
void ContainerImpl::eraseHandle(std::map<std::string, HMODULE>::iterator it)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    //m.lock;
    g_handles.erase(it);
    //m.unlock();
}
#elif __linux__
std::map<std::string, void*> &ContainerImpl::getHandles()
{
    return g_handles;
}
void ContainerImpl::addHandle(std::string pluginPath, void* handle)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_handles[pluginPath] = handle;
}
void ContainerImpl::eraseHandle(std::map<std::string, void*>::iterator it)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_handles.erase(it);
}
#endif

// Reference counter for events.
std::map<std::string, size_t> &ContainerImpl::getEventRefCount()
{
    return g_eventsRef;
}

void ContainerImpl::addEventRefCount(std::string name)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (g_eventsRef.find(name) != g_eventsRef.end())
    {
        ++g_eventsRef.at(name);
    }
    else
    {
        g_eventsRef.insert(std::pair<std::string, int>(name, 1));
    }
}

void ContainerImpl::subtractEventRefCount(std::string name)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (g_eventsRef.find(name) != g_eventsRef.end())
    {
        --g_eventsRef.at(name);
    }
}

void ContainerImpl::eraseEventRefCount(std::string name)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_eventsRef.erase(name);
}

// Store void pointers to Events.
std::map<std::string, void*>& ContainerImpl::getEvents()
{
    return g_events;
}

void ContainerImpl::addEvent(std::string name, void* ptr_event)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_events[name] = ptr_event;
}

void ContainerImpl::eraseEvent(std::string name)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_events.erase(name);
}

std::map<std::string, size_t>& ContainerImpl::getEventStreamRefCount()
{
    return g_eventStreamsRef;
}

void ContainerImpl::addEventStreamRefCount(std::string type)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (g_eventStreamsRef.find(type) != g_eventStreamsRef.end())
    {
        ++g_eventStreamsRef.at(type);
    }
    else
    {
        g_eventStreamsRef.insert(std::pair<std::string, int>(type, 1));
    }
}

void ContainerImpl::subtractEventStreamRefCount(std::string type)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    if (g_eventStreamsRef.find(type) != g_eventStreamsRef.end())
    {
        --g_eventStreamsRef.at(type);
    }
}

void ContainerImpl::eraseEventStreamRefCount(std::string type)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_eventStreamsRef.erase(type);
}

std::map<std::string, void*>& ContainerImpl::getEventStreams()
{
    return g_eventStreams;
}

void ContainerImpl::addEventStream(std::string type, void* ptr_eventStream)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_eventStreams[type] = ptr_eventStream;
}

void ContainerImpl::eraseEventStream(std::string type)
{
    std::lock_guard<std::recursive_mutex> lock(m_lock);
    g_eventStreams.erase(type);
}

// Create a container instance.
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
