#ifndef EVENT_H
#define EVENT_H

#include <atomic>
#include <iostream>
#include <functional>
#include <vector>
#include <mutex>
#include <future>
#include <map>
#include <string>
#include <cstdarg>
#include <type_traits>

#include "container.h"
#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#else
#error define your compiler
#endif

// The EventStream class is responsible for managing all Events sharing the same number of arguments/argument types in this 
// application. It is designed as a singleton so that a single instance stores all Events and their corresponding handles, thus 
// allowing it to be easily shared/distributed across multiple plugins. If we create an EventStream instance of type double, for
// example (i.e. EventStream<double>), then that EventStream will be responsible for managing Event<double> objects, and by 
// extension handles whose only argument is a type-double variable.
template <typename... Args> class EventStream
{
private:
    static Container* m_container;
    std::recursive_mutex m_lock;

    // Use a function signature to obtain the EventStream's templated specialization type.
    static std::string esType(std::string funcSig)
    {
        std::string type;
        bool addChar = false;
        int cnt = 0;
        for (int i = 0; i < funcSig.size(); ++i)
        {
            #ifdef _WIN32
            if (funcSig[i] == '>')
            #elif __linux__
            if (funcSig[i] == '}')
            #endif
            {
                --cnt;
                if (cnt == 0)
                {
                    addChar = false;
                    break;
                }
            }

            if (addChar)
            {
                type.push_back(funcSig[i]);
            }

            #ifdef _WIN32
            if (funcSig[i] == '<')
            #elif __linux__
            if (funcSig[i] == '{')
            #endif
            {
                if (!addChar)
                {
                    addChar = true;
                }
                ++cnt;
            }
        }

        return type;
    }

    // Load the container plugin, which contains data shared across all loaded plugins (including plugin pointers,
    // events, etc.).
    static void loadContainer(size_t identifier)
    {
        if (m_container == nullptr)
        {
            std::cout << "m_container is a null pointer" << std::endl;
            const char* appDir = reinterpret_cast<const char*>(identifier);
            #ifdef _WIN32
            std::string intermediateString = std::string(appDir) + "\\container.dll";
            //#ifdef _DEBUG
            //std::string intermediateString = "c:\\_projects\\modular_application_framework\\_build\\x64\\Debug\\bin\\container.dll";
            //#else
            //std::string intermediateString = "c:\\_projects\\modular_application_framework\\_build\\x64\\Release\\bin\\container.dll";
            //#endif
            std::wstring containerPath = std::wstring(intermediateString.begin(), intermediateString.end());
            LPCWSTR cp = containerPath.c_str();
            HMODULE containerHandle = LoadLibrary(cp);
            typedef Container* (*fnCreateContainer)();
            fnCreateContainer createContainer = (fnCreateContainer)GetProcAddress(containerHandle, "Create");
            #elif __linux__
            //std::string containerPath = "/mnt/hgfs/C/_projects/modular_application_framework/_build_linux/bin/container.so";
            std::string containerPath = std::string(appDir) + "/container.so";
            void* containerHandle = dlopen(containerPath.c_str(), 3);
            typedef Container* (*fnCreateContainer)();
            fnCreateContainer createContainer = (fnCreateContainer)dlsym(containerHandle, (char*)("Create"));
            #endif
            m_container = createContainer();
        }
    }

    EventStream() 
    {
        #ifdef _WIN32
        std::cout << "EventStream() was called on" << __FUNCSIG__ << std::endl;
        #elif __linux__
        std::cout << "EventStream() was called on " << __PRETTY_FUNCTION__ << std::endl;
        #endif
    };

    ~EventStream() 
    {
        #ifdef _WIN32
        std::cout << "~EventStream() was called on" << __FUNCSIG__ << std::endl;
        #elif __linux__
        std::cout << "~EventStream() was called on " << __PRETTY_FUNCTION__ << std::endl;
        #endif
    }

    // EventHandler is a wrapper class for functions we wish to subscribe to a specific event.
    // Each EventHandler basically stores the actual function and an associated id.
    //template <typename... Args> class EventHandler
    template <typename... Args2> class EventHandler
    {
    public:
        explicit EventHandler(const std::function<void(Args2...)>& handlerFunc)
            : m_handlerFunc(handlerFunc)
        {
            m_handlerId = ++m_handlerIdCounter;
        }

        // Copy constructor.
        EventHandler(const EventHandler<Args2...>& src)
            : m_handlerFunc(src.m_handlerFunc), m_handlerId(src.m_handlerId)
        {}

        // Move constructor.
        EventHandler(EventHandler<Args2...>&& src)
            : m_handlerFunc(std::move(src.m_handlerFunc)), m_handlerId(src.m_handlerId)
        {}

        size_t id() const
        {
            return m_handlerId;
        }

        // Function call operator.
        void operator()(Args2... params) const
        {
            if (m_handlerFunc)
            {
                m_handlerFunc(params...);
            }
        }

        // Event handler comparison operator.
        bool operator==(const EventHandler<Args2...>& other) const
        {
            return m_handlerId == other.m_handlerId;
        }

        // Copy assignment operator.
        EventHandler<Args2...>& operator=(const EventHandler<Args2...>& src)
        {
            if (&src == this) return *this;
            m_handlerFunc = src.m_handlerFunc;
            m_handlerId = src.m_handlerId;

            return *this;
        }

        // Move assignment operator.
        EventHandler<Args2...>& operator=(EventHandler<Args2...>&& src)
        {
            std::swap(m_handlerFunc, src.m_handlerFunc);
            m_handlerId = src.m_handlerId;

            return *this;
        }

    private:
        size_t m_handlerId;
        std::function<void(Args2...)> m_handlerFunc;
        static inline std::atomic_uint m_handlerIdCounter = 0;
    };

    // The Event class forms the meat-and-potatoes of the event system. Every instantiated Event object can be specialized to accept 
    // specific user inputs. EventHandlers that share the same argument types as an Event can be subscribed to that Event. Events hold
    // a list of EventHandlers that can be called in a variety of ways, so every time an Event is triggered/ran it actually ends up calling
    // the corresponding subscribed handles. Similarly, EventHandlers can be unsubscribed from an Event so that they are no longer called
    // whenever an Event is ran.
    template <typename... Args2> class Event
    {
    public:
        // Default constructor.
        Event()
        {
            std::cout << "Event has been default-constructed!" << std::endl;
        }

        // Copy constructor.
        Event(const Event<Args2...>& src)
        {
            std::lock_guard<std::recursive_mutex> lock(src.m_handlersLocker);

            m_handlers = src.m_handlers;
        }

        // Move constructor.
        Event(Event<Args2...>&& src)
        {
            std::lock_guard<std::recursive_mutex> lock(src.m_handlersLocker);

            m_handlers = std::move(src.m_handlers);
        }

        // Add an EventHandler to the current Event. Return a size_t id that uniquely identifies the handler.
        size_t add(const EventHandler<Args2...>& handler)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before subscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isSubscriptionComplete = false;
            m_handlers.push_back(handler);
            isSubscriptionComplete = true;
            return handler.id();
        }

        // Add a vector of EventHandlers to the current Event. Return a vector of size_t ids that uniquely identify all subscribed handlers.
        std::vector<size_t> add(const std::vector<EventHandler<Args2...>>& handlers)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before subscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isSubscriptionComplete = false;
            std::vector<size_t> _ids; _ids.reserve(handlers.size());
            for (int i = 0; i < handlers.size(); ++i)
            {
                m_handlers.push_back(handlers[i]);
                _ids.push_back(handlers[i].id());
            }
            isSubscriptionComplete = true;
            return _ids;
        }

        // Add an std::function (which gets converted into an EventHandler) to the current Event. Return a size_t id that uniquely 
        // identifies the handler.
        size_t add(const std::function<void(Args2...)>& handler)
        {
            //std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            return add(EventHandler<Args2...>(handler));
        }

        // Add a vector of std::functions (each of which gets converted into an EventHandler) to the current Event. Return a vector of size_t ids 
        // that uniquely identify all subscribed handlers.
        std::vector<size_t> add(const std::vector<std::function<void(Args2...)>>& handlers)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before subscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isSubscriptionComplete = false;
            std::vector<size_t> _ids; _ids.reserve(handlers.size());
            for (int i = 0; i < handlers.size(); ++i)
            {
                EventHandler<Args2...> _eh(handlers[i]);
                m_handlers.push_back(_eh);
                _ids.push_back(_eh.id());
            }
            isSubscriptionComplete = true;
            return _ids;
        }

        // Remove an EventHandler from the Event by searching for the handle specifically.
        void remove(const EventHandler<Args2...>& handler)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before unsubscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isUnsubscriptionComplete = false;
            auto it = std::find(m_handlers.begin(), m_handlers.end(), handler);
            if (it != m_handlers.end())
            {
                m_handlers.erase(it);
            }
            isUnsubscriptionComplete = true;
        }

        // Remove all EventHandlers in the input vector by searching for the handle specifically.
        void remove(const std::vector<EventHandler<Args2...>>& handlers)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before unsubscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isUnsubscriptionComplete = false;
            for (int i = 0; i < handlers.size(); ++i)
            {
                auto it = std::find(m_handlers.begin(), m_handlers.end(), handlers[i]);
                if (it != m_handlers.end())
                {
                    m_handlers.erase(it);
                }
            }
            isUnsubscriptionComplete = true;
        }

        // Remove an EventHandler from the Event by searching for the handle's id.
        void remove_id(const size_t& handlerId)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before unsubscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isUnsubscriptionComplete = false;
            auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [handlerId](const EventHandler<Args2...>& handler) { return handler.id() == handlerId; });
            if (it != m_handlers.end())
            {
                m_handlers.erase(it);
            }
            isUnsubscriptionComplete = true;
        }

        // Remove all EventHandlers in the input argument list from the Event by searching for each handle's id.
        void remove_id(const std::vector<size_t>& handlerIds)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before unsubscribing any new functions.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isUnsubscriptionComplete = false;
            for (int i = 0; i < handlerIds.size(); ++i)
            {
                size_t _id = handlerIds[i];
                auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [_id](const EventHandler<Args2...>& handler) { return handler.id() == _id; });
                if (it != m_handlers.end())
                {
                    m_handlers.erase(it);
                }
            }
            isUnsubscriptionComplete = true;
        }

        // Sequentially/synchronously call each EventHandler in this Event.
        void call(Args2... params)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before calling any functions again.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isExecutionComplete = false;
            std::vector<EventHandler<Args2...>> handlersCopy = getHandlersCopy();
            callImpl(handlersCopy, params...);
            isExecutionComplete = true;
        }

        // Allows one to run this Event in multiple threads.
        std::future<void> callAsyncBlocking(Args2... params)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before calling any functions again.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            return std::async(std::launch::async, [this](Args2... asyncParams)
            { call(asyncParams...); }, params...);
        }

        // Run each EventHandler subscribed to this Event in a separate thread.
        void callAsync(Args2... params)
        {
            // Wait for current execution cycle and any other subscription/unsubscription services to finish before calling any functions again.
            while (!isExecutionComplete || !isSubscriptionComplete || !isUnsubscriptionComplete) continue;
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            isExecutionComplete = false;
            callAsyncImpl(m_handlers, params...);
            isExecutionComplete = true;
        }

        // Returns a copy of the Event's std::vector of EventHandlers. 
        std::vector<EventHandler<Args2...>> getHandlersCopy() const
        {
            // TODO: This line seemed to be the issue when it came to input halting from time-to-time.
            //std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);

            // Since the function return value is by copy, 
            // before the function returns (and destruct the lock_guard object),
            // it creates a copy of the m_handlers container.
            return m_handlers;
        }

        // Copy assignment operator.
        Event<Args2...>& operator=(const Event<Args2...>& src)
        {
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);
            std::lock_guard<std::recursive_mutex> lock2(src.m_handlersLocker);

            m_handlers = src.m_handlers;

            return *this;
        }

        // Move assignment operator.
        Event<Args2...>& operator=(Event<Args2...>&& src)
        {
            std::lock_guard<std::recursive_mutex> lock(m_handlersLocker);
            std::lock_guard<std::recursive_mutex> lock2(src.m_handlersLocker);

            std::swap(m_handlers, src.m_handlers);

            return *this;
        }

        // Atomic flag used to determine if all EventHandlers subscribed to an Event have finished executing.
        std::atomic<bool> isExecutionComplete = true;
        // Atomic flag used to determine if the subscription of EventHandlers has completed.
        std::atomic<bool> isSubscriptionComplete = true;
        // Atomic flag used to determine if the unsubscription of EventHandlers has completed.
        std::atomic<bool> isUnsubscriptionComplete = true;

    private:
        std::recursive_mutex m_handlersLocker;
        std::vector<EventHandler<Args2...>> m_handlers;

        // Helper function for call(Args... params). Simply loops through all handles in the Event
        // and calls them in the order that they are stored.
        void callImpl(const std::vector<EventHandler<Args2...>>& handlers, Args2... params) const
        {
            //std::cout << "Num Handles: " << handlers.size() << std::endl;
            for (const auto& handler : handlers)
            {
                handler(params...);
            }
        }

        // Helper function for callAsync(Args... params). Spawns entirely-separate threads for each subscribed handle to 
        // be executed in. The method then waits for all threads to finish executing before joining them back together.
        void callAsyncImpl(const std::vector<EventHandler<Args2...>>& handlers, Args2... params)
        {
            std::vector<std::thread> threads;
            std::vector<bool> threadStatus;
            
            threads.reserve(handlers.size());
            threadStatus.reserve(handlers.size());

            int cnt = 0;
            for (const auto& handler : handlers)
            {
                threadStatus.push_back(false);
                // Run some task on a new thread. Make sure to set the done flag to true when finished.
                threads.push_back(std::thread([&threadStatus, cnt, handler, params...]{ handler(params...); threadStatus[cnt] = true; }));
                ++cnt;
            }

            while (cnt > 0)
            {
                for (size_t i = 0; i < threadStatus.size(); ++i)
                {
                    if (threadStatus[i])
                    {
                        --cnt;
                    }
                }
            }

            for (size_t i = 0; i < threads.size(); ++i)
            {
                threads[i].join();
            }

            threadStatus.clear();
            threads.clear();
        }
    };

public:
    void requestDelete()
    {
        // Use the function signature of requestDelete() to obtain the EventStream's templated specialization type.
        #ifdef _WIN32
        std::string type = esType(__FUNCSIG__);
        #elif __linux__
        std::string type = esType(__PRETTY_FUNCTION__);
        #endif

        std::cout << "_______________EventStream<" << type << "> requestDelete has been called_______________" << std::endl;
        
        if (m_container->getEventStreamRefCount().find(type) != m_container->getEventStreamRefCount().end())
        {
            std::cout << "EventStream<" << type << "> count BEFORE: " << m_container->getEventStreamRefCount().at(type) << std::endl;
        }
        else
        {
            std::cout << "EventStream<" << type << "> count BEFORE: " << 0 << std::endl;
        }

        // If the EventStream type is not even present in the container, do nothing.
        if (m_container->getEventStreamRefCount().find(type) != m_container->getEventStreamRefCount().end())
        {
            // If only one reference exists for the EventStream we wish to delete, then destruct that EventStream. Otherwise
            // simply decrement its reference counter.
            if (m_container->getEventStreamRefCount().at(type) > 1)
            {
                m_container->subtractEventStreamRefCount(type);
            }
            else
            {
                delete static_cast<EventStream<Args...>*>(m_container->getEventStreams().at(type));
                m_container->eraseEventStream(type);
                m_container->eraseEventStreamRefCount(type);
            }
        }

        if (m_container->getEventStreamRefCount().find(type) != m_container->getEventStreamRefCount().end())
        {
            std::cout << "EventStream<" << type << "> count AFTER: " << m_container->getEventStreamRefCount().at(type) << std::endl;
        }
        else
        {
            std::cout << "EventStream<" << type << "> count AFTER: " << 0 << std::endl;
        }
    }

    // Note that EventStream is a singleton. The size_t identifier is a hashed representation of the app's executable
    // directory.
    static EventStream* Instance(size_t identifier)
    {
        loadContainer(identifier);

        // Use the function signature of Instance() to obtain the EventStream's templated specialization type.
        #ifdef _WIN32
        std::string type = esType(__FUNCSIG__);
        #elif __linux__
        std::string type = esType(__PRETTY_FUNCTION__);
        #endif

        // If the template specialization of EventStream we wish to create doesn't already exist, create a new instance of
        // said EventStream and store it in the container. Otherwise simply increment the EventStream reference counter.
        if (m_container->getEventStreams().find(type) == m_container->getEventStreams().end())
        {
            std::cout << "______________A new EventStream<" << type << "> has been created______________" << std::endl;
            EventStream<Args...>* es = new EventStream;
            m_container->addEventStream(type, static_cast<void*>(es));
            m_container->addEventStreamRefCount(type);
            return es;
        }
        else
        {
            std::cout << "______________EventStream<" << type << "> already exists______________" << std::endl;
            m_container->addEventStreamRefCount(type);
            return static_cast<EventStream<Args...>*>(m_container->getEventStreams()[type]);
        }
    }

    // Create a new Event.
    void create(std::string eventName)
    {
        // Note that if the event we try to define already exists in the global container, simply increment its reference count.
        if (m_container->getEvents().count(eventName) > 0)
        {
            m_container->addEventRefCount(eventName);
        }
        else
        {
            Event<Args...>* newEvent = new Event<Args...>;
            m_container->addEvent(eventName, static_cast<void*>(newEvent));
            m_container->addEventRefCount(eventName);
        }
    }

    // Destroy the Event specified by its name.
    void destroy(std::string eventName)
    {
        std::cout << "We've called into es->destroy(" + eventName + ")" << std::endl;
        std::lock_guard<std::recursive_mutex> lock(m_lock);

        size_t cnt = m_container->getEvents().count(eventName);
        std::cout << "cnt = " << cnt << std::endl;
        if (cnt == 0)
        {
            std::cout << "Event with name " << eventName << " could not be found, and thus cannot be destroyed." << std::endl;
            return;
        }

        // If multiple references to a specific event exist, simply decrement the overall count.
        if (cnt > 1)
        {
            m_container->subtractEventRefCount(eventName);
        }
        else
        {
            Event<Args...>* eventPtr = static_cast<Event<Args...>*>(m_container->getEvents().at(eventName));
            if (eventPtr != nullptr)
            {
                // Wait for the Event to finish executing any handlers in the middle of computation before deleting.
                while (!eventPtr->isExecutionComplete)
                {
                    continue;
                }
                delete eventPtr;
                m_container->eraseEvent(eventName);
                m_container->eraseEventRefCount(eventName);
                eventPtr = nullptr;
            }
            else
            {
                std::cout << "eventPtr is null" << std::endl;
            }
        }
    }

    // Subscribe multiple methods simultaneously to a named Event using a vector of std::functions. Returns a vector of unique ids that 
    // map to the handler functions we subscribed.
    std::vector<size_t> subscribe(std::string eventName, const std::vector<std::function<void(Args...)>>& handlerFuncs)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            return static_cast<Event<Args...>*>(m_container->getEvents()[eventName])->add(handlerFuncs);
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to perform subscription." << std::endl;
            return std::vector<size_t>();
        }
    }

    // Subscribe multiple methods simultaneously to a named Event using a vector of function pointers. Returns a vector of unique ids that
    // map to the handler functions we subscribed.
    std::vector<size_t> subscribe(std::string eventName, const std::vector<void(*)(Args...)>& handlerFuncs)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            std::vector<std::function<void(Args...)>> _v; _v.reserve(handlerFuncs.size());
            for (int i = 0; i < handlerFuncs.size(); ++i)
            {
                std::function<void(Args...)> _f = handlerFuncs[i];
                _v.push_back(_f);
            }
            return subscribe(eventName, _v);
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to perform subscription." << std::endl;
            return std::vector<size_t>();
        }
    }

    // Subscribe multiple methods simultaneously to a named Event using variadic argument. Returns a  vector of unique ids that map to the 
    // handler functions we subscribed.
    template<typename T, typename... Args2>
    std::vector<size_t> subscribe(std::string eventName, T firstHandlerFunc, Args2... handlerFuncs)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            typedef void(*funcType)(Args...);
            // Use std::conjunction to check if the templated typenames correspond to valid function types.
            if ((std::conjunction_v<std::is_same<std::function<void(Args...)>, T>> && std::conjunction_v<std::is_same<std::function<void(Args...)>, Args2>...>)
                || (std::conjunction_v<std::is_same<funcType, T>> && std::conjunction_v<std::is_same<funcType, Args2>...>))
            {
                T handlerFuncsArr[sizeof...(handlerFuncs) + 1] = { firstHandlerFunc, handlerFuncs... };
                int cnt = sizeof(handlerFuncsArr) / sizeof(handlerFuncsArr[0]);

                std::vector<T> handlerFuncsVector(handlerFuncsArr, handlerFuncsArr + cnt);

                return subscribe(eventName, handlerFuncsVector);
            }

            else
            {
                std::cout << "ERROR: Invalid argument type for subscription; unable to perform subscription." << std::endl;
            }
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to perform subscription." << std::endl;
            return std::vector<size_t>();
        }
    }

    // Unsubscribe multiple functions simultaneously from an Event using a list of unique ids that map
    // to each handler.
    void unsubscribe(std::string eventName, const std::vector<size_t>& handlerIds)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            static_cast<Event<Args...>*>(m_container->getEvents()[eventName])->remove_id(handlerIds);
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to perform unsubscription." << std::endl;
        }
    }

    // Unsubscribe multiple functions simultaneously from an Event using a variadic input of unique ids that map
    // to each handler.
    template<typename Arg1, typename... Args2>
    void unsubscribe(std::string eventName, Arg1 firstId, Args2... handlerIds)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            if ((std::conjunction_v<std::is_same<size_t, Arg1>> && std::conjunction_v<std::is_same<size_t, Args2>...>)
                || (std::conjunction_v<std::is_same<int, Arg1>> && std::conjunction_v<std::is_same<int, Args2>...>)
                || (std::conjunction_v<std::is_same<unsigned int, Arg1>> && std::conjunction_v<std::is_same<unsigned int, Args2>...>))
            {
                Arg1 handlerIdsArr[sizeof...(handlerIds) + 1] = { firstId, handlerIds... };
                int cnt = sizeof(handlerIdsArr) / sizeof(handlerIdsArr[0]);
                std::vector<size_t> handlerIdsVector(handlerIdsArr, handlerIdsArr + cnt);

                return unsubscribe(eventName, handlerIdsVector);
            }

            else
            {
                std::cout << "ERROR: Invalid argument type for unsubscription; unable to perform unsubscription." << std::endl;
            }
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to perform unsubscription." << std::endl;
        }
    }

    // Sequentially call each EventHandler in a name-specified Event.
    void call(std::string eventName, Args... params)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            static_cast<Event<Args...>*>(m_container->getEvents()[eventName])->call(params...);
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to call." << std::endl;
        }
    }

    // Allows one to run the same name-specified Event in multiple threads.
    std::future<void> callAsyncBlocking(std::string eventName, Args... params)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            static_cast<Event<Args...>*>(m_container->getEvents()[eventName])->callAsyncBlocking(params...);
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to callAsyncBlocking." << std::endl;
            return std::future<void>();
        }
    }

    // Run each EventHandler for a name-specified Event in a separate thread.
    void callAsync(std::string eventName, Args... params)
    {
        if (m_container->getEvents().count(eventName) > 0)
        {
            static_cast<Event<Args...>*>(m_container->getEvents()[eventName])->callAsync(params...);
        }

        else
        {
            std::cout << "No Event named " << eventName << " exists; unable to callAsync." << std::endl;
        }
    }
};

template <typename... Args> Container* EventStream<Args...>::m_container = 0;

#endif // EVENT_H