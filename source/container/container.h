#ifndef CONTAINER_H
#define CONTAINER_H

#ifdef _WIN32
    #include <Windows.h>
#endif

struct Container
{
public:
    virtual std::map<std::string, int> &getRefCount() = 0;
    virtual void addRefCount(std::string pluginName) = 0;
    virtual void subtractRefCount(std::string pluginName) = 0;
    virtual void eraseRefCount(std::string pluginName) = 0;

    virtual std::map<std::string, void*> &getPlugins() = 0;
    virtual void addPlugin(std::string pluginPath, void* ptr_plugin) = 0;
    virtual void erasePlugin(std::string pluginPath) = 0;

    #ifdef _WIN32
    virtual std::map<std::string, HMODULE> &getHandles() = 0;
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
};

#endif // CONTAINER_H
