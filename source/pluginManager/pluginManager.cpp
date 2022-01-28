// The plug-in manager is responsible for loading/unloading plug-ins into the program
// and performing some basic checks, such as whether a plug-in that is currently
// pending for loading is, in fact, already loaded.
// By design only one instance of a given dynamic library is allowed at any time.

#include "pluginManager.h"

PluginManager* PluginManager::m_instance = 0;
std::pair<size_t, const char*> PluginManager::m_appDir;
Container* PluginManager::m_container = 0;

PluginManager::PluginManager() 
{

}

PluginManager::~PluginManager()
{

}

void PluginManager::loadContainer()
{
    std::cout << "PluginManager::loadContainer has been called" << std::endl;
    if (m_container == nullptr)
    {
        std::cout << "m_container is a null pointer" << std::endl;
        #ifdef _WIN32
            std::string intermediateString = std::string(m_appDir.second) + "\\container.dll";
            std::wstring containerPath = std::wstring(intermediateString.begin(), intermediateString.end());
            LPCWSTR cp = containerPath.c_str();
            HMODULE containerHandle = LoadLibrary(cp);
            typedef Container*(*fnCreateContainer)();
            fnCreateContainer createContainer = (fnCreateContainer)GetProcAddress(containerHandle, "Create");
        #elif __linux__
            std::string containerPath = std::string(m_appDir.second) + "/container.so";
            void* containerHandle = dlopen(containerPath.c_str(), 3);
            typedef Container*(*fnCreateContainer)();
            fnCreateContainer createContainer = (fnCreateContainer)dlsym(containerHandle, (char*)("Create"));
        #endif
        m_container = createContainer();
    }
}

PluginManager* PluginManager::Instance(size_t identifier)
{
    m_appDir = std::pair<size_t, const char*>(identifier, reinterpret_cast<const char*>(identifier)); // dehash the size_t to get the executable's absolute directory.

    if (!PluginManager::m_instance)
    {
        PluginManager::m_instance = new PluginManager;
    }
    
    return PluginManager::m_instance;
}

// The Load method is overloaded with two definitions - the first returns the plug-in pointer,
// while the second uses an accessor.

// This first overloaded definition of the ::Load method loads the specified plug-in by name, and returns a pointer to 
// that .dll/.so. For most cases this is the preferable function to use since it saves one from having to define a null
// pointer in a separate line before passing it to ::Load (i.e. one less line of code, slightly easier readability).
Plugin* PluginManager::Load(const char* pluginName)
{
    std::cout << "PluginManager::Load has been called" << std::endl;
    // Plug-in manager may be called from multiple places in parallel.
    // To provide thread safety, only one instance may access the shared
    // internal structures at a time.
    m.lock();

    // load the shared container that stores information about loaded plug-ins
    loadContainer();

    Plugin* ptr_plugin = nullptr;

    #ifdef _WIN32
        //std::string pluginPath = std::string(m_appDir.second) + "..\\plugins\\" + pluginName + ".dll";
        std::string pluginPath = std::string(m_appDir.second) + "..\\plugins\\" + pluginName + "\\bin\\" + pluginName + ".dll";
    #elif __linux__
        //std::string pluginPath = std::string(m_appDir.second) + "../plugins/" + pluginName + ".so";
        std::string pluginPath = std::string(m_appDir.second) + "../plugins/" + pluginName + "/bin/" + pluginName + ".so";
    #endif

    std::cout << "m_appDir: " << m_appDir.second << std::endl;
    // Check the plug-in map to see if the requested plug-in has been loaded before.
    // If it has, simply return that same plug-in rather than loading a new instance
    // of the dynamic library.
    auto it = m_container->getPlugins().find(pluginPath);
    if (it == (m_container->getPlugins()).end())
    {
        // try to load the plugin library
        #ifdef _WIN32
            std::wstring stemp = std::wstring(pluginPath.begin(), pluginPath.end());
            LPCWSTR sw = stemp.c_str();
            HMODULE handle = LoadLibrary(sw);
        #elif __linux__
            void* handle = dlopen(pluginPath.c_str(), 3);
        #endif
        if (handle != nullptr)
        {
            // Define the prototype for a function that should exist in the .dll/.so
            // that is used to create and return the plug-in type in the .dll/.so.
            typedef Plugin*(*fnCreatePlugin)(const char* path);

            // Note that every plug-in will need to define a method called CreatePlugin
            // (which we gain access to in the plug-in manager via GetProcAddress), which
            // will return a pointer to the Plugin object defined in the .dll/.so file.
            #ifdef _WIN32
                fnCreatePlugin CreatePlugin = (fnCreatePlugin)GetProcAddress(handle, "Create");
            #elif __linux__
                fnCreatePlugin CreatePlugin = (fnCreatePlugin)dlsym(handle, (char*)("Create"));
            #endif

            // handle plug-in load
            if (CreatePlugin != NULL)
            {
                ptr_plugin = CreatePlugin(pluginPath.c_str());
                if (ptr_plugin != NULL)
                {
                    // add the plug-in and library to the shared containers
                    m_container->addPlugin(pluginPath, static_cast<void*>(ptr_plugin));
                    m_container->addHandle(pluginPath, handle);
                    m_container->addRefCount(std::string(pluginName));
                    ptr_plugin->initialize(m_appDir.first);
                    std::cout << pluginPath << " successfully loaded!" << std::endl;
                }
                else
                {
                    std::cerr << "ERROR: Could not load plug-in from " << pluginPath << std::endl;
                    #ifdef _WIN32
                        FreeLibrary(handle);
                    #elif __linux__
                        dlclose(handle);
                    #endif
                }
            }
            else
            {
                std::cerr << "ERROR: Could not find symbol \"Create\" in " << pluginPath << std::endl;
                #ifdef _WIN32
                    FreeLibrary(handle);
                #elif __linux__
                    dlclose(handle);
                #endif
            }
        }
        else
        {
            std::cerr << "INFO: Could not load library: " << pluginPath << std::endl;
        }
    }
    else
    {
        std::cout << "Library \"" << pluginPath << "\" already loaded." << std::endl;
        ptr_plugin = static_cast<Plugin*>(it->second);
        m_container->addRefCount(std::string(pluginName));
    }

    m.unlock();

    return ptr_plugin;
}

// This second overloaded definition of the ::Load method passes by reference a plug-in pointer in its second argument,
// which gets set to the correct memory address of the loaded plug-in after it is successfully loaded. Consider using this
// definition over the first if returning the pointer directly by value is not an option. For example, if one is attempting 
// to use the load method in multiple threads, but wishes to use the returned plug-in pointers later on in a single-threaded 
// environment, then null pointers could be defined and passed by reference to the ::Load functions in each thread, thereby 
// correctly setting the pointers and allowing for future single-thread use (check the concurrentLoading plug-in for a more
// concrete example of how this works).
void PluginManager::Load(const char* pluginName, Plugin* &ptr_plugin)
{
    // Plug-in manager may be called from multiple places in parallel.
    // To provide thread safety, only one instance may access the shared
    // internal structures at a time.
    m.lock();

    // load the shared container that stores information about loaded plug-ins
    loadContainer();

    #ifdef _WIN32
        //std::string pluginPath = std::string(m_appDir.second) + "..\\plugins\\" + pluginName + ".dll";
        std::string pluginPath = std::string(m_appDir.second) + "..\\plugins\\" + pluginName + "\\bin\\" + pluginName + ".dll";
    #elif __linux__
        //std::string pluginPath = std::string(m_appDir.second) + "../plugins/" + pluginName + ".so";
        std::string pluginPath = std::string(m_appDir.second) + "../plugins/" + pluginName + "/bin/" + pluginName + ".so";
    #endif

    // Check the plug-in map to see if the requested plug-in has been loaded before.
    // If it has, simply return that same plug-in rather than loading a new instance
    // of the dynamic library.
    auto it = m_container->getPlugins().find(pluginPath);
    if (it == (m_container->getPlugins()).end())
    {
        // Try to load the plugin library.
        #ifdef _WIN32
            std::wstring stemp = std::wstring(pluginPath.begin(), pluginPath.end());
            LPCWSTR sw = stemp.c_str();
            HMODULE handle = LoadLibrary(sw);
        #elif __linux__
            void* handle = dlopen(pluginPath.c_str(), 3);
        #endif
        if (handle != nullptr)
        {
            // Define the prototype for a function that should exist in the .dll/.so
            // that is used to create and return the plug-in type in the .dll/.so.
            typedef Plugin*(*fnCreatePlugin)(const char* path);

            // Note that every plug-in will need to define a method called CreatePlugin
            // (which we gain access to in the plug-in manager via GetProcAddress), which
            // will return a pointer to the Plugin object defined in the .dll/.so file.
            #ifdef _WIN32
                fnCreatePlugin CreatePlugin = (fnCreatePlugin)GetProcAddress(handle, "Create");
            #elif __linux__
                fnCreatePlugin CreatePlugin = (fnCreatePlugin)dlsym(handle, (char*)("Create"));
            #endif

            // handle plug-in load
            if (CreatePlugin != NULL)
            {
                ptr_plugin = CreatePlugin(pluginPath.c_str());
                if (ptr_plugin != NULL)
                {
                    // add the plug-in and library to the containers
                    m_container->addPlugin(pluginPath, static_cast<void*>(ptr_plugin));
                    m_container->addHandle(pluginPath, handle);
                    m_container->addRefCount(std::string(pluginName));
                    ptr_plugin->initialize(m_appDir.first);
                    std::cout << pluginPath << " successfully loaded!" << std::endl;
                }
                else
                {
                    std::cerr << "ERROR: Could not load plug-in from " << pluginPath << std::endl;
                    #ifdef _WIN32
                        FreeLibrary(handle);
                    #elif __linux__
                        dlclose(handle);
                    #endif
                }
            }
            else
            {
                std::cerr << "ERROR: Could not find symbol \"Create\" in " << pluginPath << std::endl;
                #ifdef _WIN32
                    FreeLibrary(handle);
                #elif __linux__
                    dlclose(handle);
                #endif
            }
        }
        else
        {
            std::cerr << "INFO: Could not load library: " << pluginPath << std::endl;
        }
    }
    else
    {
        std::cout << "Library \"" << pluginPath << "\" already loaded." << std::endl;
        ptr_plugin = static_cast<Plugin*>(it->second);
        m_container->addRefCount(std::string(pluginName));
    }

    m.unlock();
}

// To unload a plug-in, this function first checks how many references (i.e. how many separate pointers)
// to a plug-in exist. If the number of references is less than or equal to 1 (i.e. we only have - at most
// - one instance of a plug-in in our given context), then the function marches on to release and delete that
// loaded plug-in instance. If the number of references is greater than 1, however, then the unload function
// simply removes a reference from the global counter to that plug-in. The plug-in will still exist in this
// context and continue to operate as usual.
void PluginManager::Unload(const char* pluginName)
{
    // Plug-in manager may be called from multiple places in parallel.
    // To provide thread safety, only one instance may access the shared
    // internal structures at a time.
    m.lock();

    loadContainer();

    #ifdef _WIN32
        //std::string pluginPath = std::string(m_appDir.second) + "..\\plugins\\" + pluginName + ".dll";
        std::string pluginPath = std::string(m_appDir.second) + "..\\plugins\\" + pluginName + "\\bin\\" + pluginName + ".dll";
    #elif __linux__
        //std::string pluginPath = std::string(m_appDir.second) + "../plugins/" + pluginName + ".so";
        std::string pluginPath = std::string(m_appDir.second) + "../plugins/" + pluginName + "/bin/" + pluginName + ".so";
    #endif

    auto it = m_container->getHandles().find(pluginPath);
    if (it != m_container->getHandles().end())
    {
        if (m_container->getRefCount()[pluginName] <= 1)
        {
            // remove the plug-in from the shared container
            Plugin* ptr_plugin = static_cast<Plugin*>(m_container->getPlugins().at(pluginPath));
            m_container->erasePlugin(pluginPath);

            #ifdef _WIN32
                HMODULE handle = it->second;
            #elif __linux__
                void* handle = it->second;
            #endif
            typedef void(*fnDestroyPlugin)();
            // Note that every plug-in that an application will load will need to define
            // a method called DestroyPlugin (which we gain access to in the main application
            // via GetProcAddress), which should deallocate all resources taken up by that plug-in.
            #ifdef _WIN32
                fnDestroyPlugin DestroyPlugin = (fnDestroyPlugin)GetProcAddress(handle, "Destroy");
            #elif __linux__
                fnDestroyPlugin DestroyPlugin = (fnDestroyPlugin)dlsym(handle, (char*)("Destroy"));
            #endif
            if (DestroyPlugin != nullptr)
            {
                ptr_plugin->release();
                DestroyPlugin();
                ptr_plugin = nullptr;
                m_container->subtractRefCount(pluginName);
                m_container->eraseRefCount(pluginName);
                std::cout << pluginPath << " has been successfully unloaded!" << std::endl;
            }
            else
            {
                std::cerr << "ERROR: Unable to find symbol \"Destroy\" in library \"" << pluginPath << std::endl;
            }

            // unload the plug-in and remove the library from the map
            #ifdef _WIN32 
                FreeLibrary(handle);
            #elif __linux__
                dlclose(handle);
            #endif
            m_container->eraseHandle(it);
        }
        else
        {
            m_container->subtractRefCount(pluginName);
        }
    }

    else
    {
        std::cout << "WARNING: Trying to unload a plug-in that is already unloaded or has never been loaded." << std::endl;
    }

    m.unlock();
}
