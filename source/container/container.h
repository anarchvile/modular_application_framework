#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef _WIN32
    #include <Windows.h>
#endif
#include <vector>

struct Container
{
public:
    // Function set for getting/setting the .exe directory.
    virtual size_t getExeDir() = 0;
    virtual void setExeDir(size_t identifier) = 0;

    // Function set for getting/setting a PluginManager and accessing/altering its
    // corresponding reference count.
    virtual size_t& getPlgManRefCount() = 0;
    virtual void addPlgManRefCount() = 0;
    virtual void subtractPlgManRefCount() = 0;
    virtual void* getPlgMan() = 0;
    virtual void setPlgMan(void* plgManPtr) = 0;

    // Function set for adding/erasing plugins from the global shared data container, and
    // accessing/altering their corresponding refernce counts.
    virtual std::map<std::string, size_t>& getPluginRefCount() = 0;
    virtual void addPluginRefCount(std::string pluginName) = 0;
    virtual void subtractPluginRefCount(std::string pluginName) = 0;
    virtual void erasePluginRefCount(std::string pluginName) = 0;
    virtual std::map<std::string, void*>& getPlugins() = 0;
    virtual void addPlugin(std::string pluginPath, void* ptr_plugin) = 0;
    virtual void erasePlugin(std::string pluginPath) = 0;

    #ifdef _WIN32
    virtual std::map<std::string, HMODULE>& getHandles() = 0;
    virtual void addHandle(std::string pluginPath, HMODULE handle) = 0;
    virtual void eraseHandle(std::map<std::string, HMODULE>::iterator it) = 0;
    #elif __linux__
    virtual std::map<std::string, void*> &getHandles() = 0;
    virtual void addHandle(std::string pluginPath, void* handle) = 0;
    virtual void eraseHandle(std::map<std::string, void*>::iterator it) = 0;
    #endif

    virtual std::map<std::string, size_t>& getEventRefCount() = 0;
    virtual void addEventRefCount(std::string name) = 0;
    virtual void subtractEventRefCount(std::string name) = 0;
    virtual void eraseEventRefCount(std::string name) = 0;
    virtual std::map<std::string, void*>& getEvents() = 0;
    virtual void addEvent(std::string name, void* ptr_event) = 0;
    virtual void eraseEvent(std::string name) = 0;

    virtual std::map<std::string, size_t>& getEventStreamRefCount() = 0;
    virtual void addEventStreamRefCount(std::string type) = 0;
    virtual void subtractEventStreamRefCount(std::string type) = 0;
    virtual void eraseEventStreamRefCount(std::string type) = 0;
    virtual std::map<std::string, void*>& getEventStreams() = 0;
    virtual void addEventStream(std::string type, void* ptr_eventStream) = 0;
    virtual void eraseEventStream(std::string type) = 0;
};

#endif // CONTAINER_H
