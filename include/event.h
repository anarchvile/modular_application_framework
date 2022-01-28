#ifndef EVENT_H
#define EVENT_H

// TODO: 
// 1. Subscribe 3 different goodbyeWorld::update functions to a runner event, use callAsync
//    to see if they run in parallel - They do not. Need an option to let events spawn their own threads to run
//    eventHandlers (i.e. functions subscribed to that event) with. Create a new callAsync function. Have an 
//    internal list of threads that get assigned to each handle when callAsync is first called (if said threads
//    have not already been assigned to). Then, have a while loop running that checks if each thread is completed.
//    When all threads are done with their computations, terminate them and return from callAsync. If, however, callAsync
//    is invoked while some threads are still running, have an internal buffer store the payloads for each function handle
//    that is yet to finish. When the threads finish their current iteration, grab the next payload in the queue and use it
//    to recall the function handle in its respective thread. For all other threads that did finish their work, simply recreate/
//    rerun them since callAsync was called again (which is the default behavior anyhow, so we don't need to do anything else
//    to obtain this functionality). - DONE
// 1.1. Check if callAsync works in multiple threads. In runner, create 2 threads to run 2 different callAsync calls, see if
//      it works correctly. It seems to work... - DONE
// 2. Mutex locking for EventStream::destroy - DONE
// 3. Keep the RunnerDes/InputDesc methods for subscribing to those plugins. Keep the code in the example
//    plugins that show how subscribing functions directly to those plugins works. - DONE.
// 4??. pythonExamplePlugin crashing/hanging when runner calls callAsync from multiple threads, need to check/fix. - Note that we 
//    can't run std::threads in here if plan on calling this start2() function froum Python via python bindings. If we wish to have 
//    multithreading on the python end, we need to implement that functionality natively using python, not through bindings/C++. - DONE.
// 5. Add EventStream logic to the rest of the example plugins (similar to how goodbyeWorld is now)
// 6. Add event logic to inputImpl, similar to the runner (i.e. create an event called "input" to which other functions can subscribe to,
//    test by having atomicPlugin subscribe functions to both event "runner" and event "input" to make sure they're doing the right thing,
//    check that both runner and input run on their own threads, see if callAsync works in each case - basically each event thread will
//    spawn another thread to run the atomicPlugin functions in - "nested" threads).
// 7. Python bindings for event.
// 8. As of right now we're not actually calling event.call or event.callAsync via the python bindings. Change/add to them and see if
//    we can successfully use the event system through the bindings.

#include <atomic>
#include <iostream>
#include <functional>
#include <vector>
#include <mutex>
#include <future>
#include <map>
#include <string>

#include "container.h"

#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <dlfcn.h>
#else
#error define your compiler
#endif

template <typename... Args> class EventHandler
{
public:
    explicit EventHandler(const std::function<void(Args...)>& handlerFunc)
        : m_handlerFunc(handlerFunc)
    {
        m_handlerId = ++m_handlerIdCounter;
    }

    // copy constructor
    EventHandler(const EventHandler<Args...>& src)
        : m_handlerFunc(src.m_handlerFunc), m_handlerId(src.m_handlerId)
    {

    }

    // move constructor
    EventHandler(EventHandler<Args...>&& src)
        : m_handlerFunc(std::move(src.m_handlerFunc)), m_handlerId(src.m_handlerId)
    {

    }

    size_t id() const
    {
        return m_handlerId;
    }

    // function call operator
    void operator()(Args... params) const
    {
        if (m_handlerFunc)
        {
            m_handlerFunc(params...);
        }
    }

    // event handler comparison operator
    bool operator==(const EventHandler<Args...>& other) const
    {
        return m_handlerId == other.m_handlerId;
    }

    // copy assignment operator
    EventHandler<Args...>& operator=(const EventHandler<Args...>& src)
    {
        if (&src == this) return *this;
        m_handlerFunc = src.m_handlerFunc;
        m_handlerId = src.m_handlerId;

        return *this;
    }

    // move assignment operator
    EventHandler<Args...>& operator=(EventHandler<Args...>&& src)
    {
        std::swap(m_handlerFunc, src.m_handlerFunc);
        m_handlerId = src.m_handlerId;

        return *this;
    }

private:
    size_t m_handlerId;
    std::function<void(Args...)> m_handlerFunc;
    static std::atomic_uint m_handlerIdCounter;
};

template <typename... Args> std::atomic_uint EventHandler<Args...>::m_handlerIdCounter(0);

template <typename... Args> class Event
{
public:
    Event() {}; // default constructor

    // copy constructor
    Event(const Event<Args...>& src)
    {
        std::lock_guard<std::mutex> lock(src.m_handlersLocker);

        m_handlers = src.m_handlers;
    }

    // move constructor
    Event(Event<Args...>&& src)
    {
        std::lock_guard<std::mutex> lock(src.m_handlersLocker);

        m_handlers = std::move(src.m_handlers);
    }

    size_t add(const EventHandler<Args...>& handler)
    {
        std::lock_guard<std::mutex> lock(m_handlersLocker);

        m_handlers.push_back(handler);
        return handler.id();
    }

    size_t add(const std::function<void(Args...)>& handler)
    {
        return add(EventHandler<Args...>(handler));
    }

    bool remove(const EventHandler<Args...>& handler)
    {
        std::lock_guard<std::mutex> lock(m_handlersLocker);

        auto it = std::find(m_handlers.begin(), m_handlers.end(), handler);
        if (it != m_handlers.end())
        {
            m_handlers.erase(it);
            return true;
        }

        return false;
    }

    bool remove_id(const size_t& handlerId)
    {
        std::lock_guard<std::mutex> lock(m_handlersLocker);

        auto it = std::find_if(m_handlers.begin(), m_handlers.end(), [handlerId](const EventHandler<Args...>& handler) { return handler.id() == handlerId; });
        if (it != m_handlers.end())
        {
            m_handlers.erase(it);
            return true;
        }

        return false;
    }

    // Sequentially call each event handler in an event.
    void call(Args... params)
    {
        m_isDone = true;
        std::vector<EventHandler<Args...>> handlersCopy = getHandlersCopy();
        callImpl(handlersCopy, params...);
    }

    void callImpl(const std::vector<EventHandler<Args...>>& handlers, Args... params) const
    {
        for (const auto& handler : handlers)
        {
            handler(params...);
        }
    }

    // Allows one to run the same event in multiple threads.
    std::future<void> callAsyncBlocking(Args... params) const
    {
        return std::async(std::launch::async, [this](Args... asyncParams)
        { call(asyncParams...); }, params...);
    }

    // Run each event handler in a separate thread.
    void callAsync(Args... params)
    {
        m_isDone = false;
        callAsyncImpl(m_handlers, params...);
        m_isDone = true;
    }

    void callAsyncImpl(const std::vector<EventHandler<Args...>>& handlers, Args... params)
    {
        std::vector<std::thread> threads;
        std::vector<bool> threadStatus;

        threads.reserve(handlers.size());
        threadStatus.reserve(handlers.size());

        size_t cnt = 0;
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

    std::vector<EventHandler<Args...>> getHandlersCopy() const
    {
        std::lock_guard<std::mutex> lock(m_handlersLocker);

        // Since the function return value is by copy, 
        // before the function returns (and destruct the lock_guard object),
        // it creates a copy of the m_handlers container.
        return m_handlers;
    }

    // copy assignment operator
    Event<Args...>& operator=(const Event<Args...>& src)
    {
        std::lock_guard<std::mutex> lock(m_handlersLocker);
        std::lock_guard<std::mutex> lock2(src.m_handlersLocker);

        m_handlers = src.m_handlers;

        return *this;
    }

    // move assignment operator
    Event<Args...>& operator=(Event<Args...>&& src)
    {
        std::lock_guard<std::mutex> lock(m_handlersLocker);
        std::lock_guard<std::mutex> lock2(src.m_handlersLocker);

        std::swap(m_handlers, src.m_handlers);

        return *this;
    }

private:
    mutable std::mutex m_handlersLocker;
    std::vector<EventHandler<Args...>> m_handlers;
    std::atomic<bool> m_isDone;
    template <typename... Args> friend class EventStream;
};

template <typename... Args> class EventStream
{
private:
    static EventStream* m_instance;
    static Container* m_container;
    mutable std::mutex m_lock;

    static void loadContainer()
    {
        if (m_container == nullptr)
        {
            std::cout << "m_container is a null pointer" << std::endl;
            #ifdef _WIN32
            //std::string intermediateString = std::string(m_appDir.second) + "\\container.dll";
            std::string intermediateString = "c:\\_projects\\modular_application_framework\\_build\\x64\\Release\\bin\\container.dll";
            std::wstring containerPath = std::wstring(intermediateString.begin(), intermediateString.end());
            LPCWSTR cp = containerPath.c_str();
            HMODULE containerHandle = LoadLibrary(cp);
            typedef Container* (*fnCreateContainer)();
            fnCreateContainer createContainer = (fnCreateContainer)GetProcAddress(containerHandle, "Create");
            #elif __linux__
            std::string containerPath = std::string(m_appDir.second) + "/container.so";
            void* containerHandle = dlopen(containerPath.c_str(), 3);
            typedef Container* (*fnCreateContainer)();
            fnCreateContainer createContainer = (fnCreateContainer)dlsym(containerHandle, (char*)("Create"));
            #endif
            m_container = createContainer();
        }
    }

    EventStream() {}

public:
    ~EventStream() 
    {
        delete m_instance;
        m_instance = nullptr;
        delete m_container;
        m_container = nullptr;
    }

    static EventStream* Instance()
    {
        loadContainer();

        if (!m_instance)
        {
            m_instance = new EventStream;
        }

        return m_instance;
    }

    Event<Args...>* create(std::string name)
    {
        if (m_container->getEvents().count(name) > 0)
        {
            m_container->addEventRefCount(name);
            return static_cast<Event<Args...>*>(m_container->getEvents()[name]);
        }
        else
        {
            Event<Args...>* newEvent = new Event<Args...>;
            m_container->addEvent(name, static_cast<void*>(newEvent));
            m_container->addEventRefCount(name);
            return newEvent;
        }
    }

    void destroy(std::string name, Event<Args...>*& eventPtr = nullptr)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        if (m_container->getEvents().count(name) > 1)
        {
            m_container->subtractEventRefCount(name);
        }
        else if (eventPtr != nullptr)
        {
            while (!eventPtr->m_isDone)
            {
                continue;
            }
            delete eventPtr;
            //delete m_container->getEvents()[name];
            //m_container->getEvents()[name] = nullptr;
            m_container->eraseEvent(name);
            m_container->eraseEventRefCount(name);
            eventPtr = nullptr;
        }
    }

    size_t subscribe(std::string name, const std::function<void(Args...)>& handlerFunc)
    {
        if (m_container->getEvents().count(name) > 0)
        {
            return static_cast<Event<Args...>*>(m_container->getEvents()[name])->add(handlerFunc);
        }

        else
        {
            std::cout << "No Event named " << name << " exists; unable to perform subscription." << std::endl;
        }
    }

    void unsubscribe(std::string name, size_t handlerId)
    {
        if (m_container->getEvents().count(name) > 0)
        {
            static_cast<Event<Args...>*>(m_container->getEvents()[name])->remove_id(handlerId);
        }

        else
        {
            std::cout << "No Event named " << name << " exists; unable to perform unsubscription." << std::endl;
        }
    }

    void numEvents()
    {
        std::cout << "Num Event Refs = " + std::to_string(m_container->getEventRefCount().size()) << std::endl;
        std::cout << "Num Events = " + std::to_string(m_container->getEvents().size()) << std::endl;
    }
};

template <typename... Args> EventStream<Args...>* EventStream<Args...>::m_instance = 0;
template <typename... Args> Container* EventStream<Args...>::m_container = 0;

#endif // EVENT_H