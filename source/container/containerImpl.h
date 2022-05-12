#ifndef CONTAINERIMPL_H
#define CONTAINERIMPL_H

#include <map>
#include "container.h"

#ifdef _WIN32
    #include <Windows.h>
    #ifdef CONTAINER_EXPORTS
        #define CONTAINER __declspec(dllexport)
    #else
        #define CONTAINER __declspec(dllimport)
    #endif
#elif __linux__
    #define CONTAINER
#endif

struct ContainerImpl : Container
{
public:
    size_t getExeDir();
    void setExeDir(size_t identifier);

    size_t& getPlgManRefCount();
    void addPlgManRefCount();
    void subtractPlgManRefCount();
    void* getPlgMan();
    void setPlgMan(void* plgManPtr);

    std::map<std::string, size_t> &getPluginRefCount();
    void addPluginRefCount(std::string pluginName);
    void subtractPluginRefCount(std::string pluginName);
    void erasePluginRefCount(std::string pluginName);
    std::map<std::string, void*> &getPlugins();
    void addPlugin(std::string pluginPath, void* ptr_plugin);
    void erasePlugin(std::string pluginPath);

    #ifdef _WIN32
    std::map<std::string, HMODULE> &getHandles();
    void addHandle(std::string pluginPath, HMODULE handle);
    void eraseHandle(std::map<std::string, HMODULE>::iterator it);
    #elif __linux__
    std::map<std::string, void*> &getHandles();
    void addHandle(std::string pluginPath, void* handle);
    void eraseHandle(std::map<std::string, void*>::iterator it);
    #endif

    std::map<std::string, size_t>& getEventRefCount();
    void addEventRefCount(std::string name);
    void subtractEventRefCount(std::string name);
    void eraseEventRefCount(std::string name);
    std::map<std::string, void*>& getEvents();
    void addEvent(std::string name, void* ptr_event);
    void eraseEvent(std::string name);

    std::map<std::string, size_t>& getEventStreamRefCount();
    void addEventStreamRefCount(std::string type);
    void subtractEventStreamRefCount(std::string type);
    void eraseEventStreamRefCount(std::string type);
    std::map<std::string, void*>& getEventStreams();
    void addEventStream(std::string type, void* ptr_eventStream);
    void eraseEventStream(std::string type);
};

extern "C" CONTAINER ContainerImpl* Create();

#endif // CONTAINERIMPL_H
