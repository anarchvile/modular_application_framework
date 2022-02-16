#include <pybind11/functional.h>
#include "event.h"

// TODO: 
// Can we have variadic argument template for the python-binded version of subscribe, call, etc.? If so, implement it.
// callAsync won't work directly in Python - need to be able to call each eventHandle individually before spawning threads
// on the python side of things to execute each handler concurrently.

namespace
{
    EventStream<double>* es = nullptr;

    void create(const char* eventName)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        es->create(eventName);
        std::cout << "Create pyEvent named " << eventName << std::endl;
    }

    void destroy(const char* eventName)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        es->destroy(eventName);
        std::cout << "Destroy pyEvent named " << eventName << std::endl;
    }

    size_t subscribe(const char* eventName, const std::function<void(double)>& handlerFunc)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        return es->subscribe(std::string(eventName), handlerFunc);
        std::cout << "Subscribe to " << eventName << std::endl;
    }

    void unsubscribe(const char* eventName, size_t id)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        es->unsubscribe(std::string(eventName), id);
        std::cout << "Unsubscribing function with id " << id << " from " << eventName << std::endl;
    }

    void call(const char* eventName, double params)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        es->call(eventName, params);
    }

    void callAsyncBlocking(const char* eventName, double params)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        std::future<void> f = es->callAsyncBlocking(eventName, params);
    }

    void callAsync(const char* eventName, double params)
    {
        if (es == nullptr)
        {
            es = EventStream<double>::Instance();
        }

        es->callAsync(eventName, params);
    }
}

PYBIND11_MODULE(eventPython, m)
{
    m.doc() = "pybind11 runner plugin"; // optional module docstring
    m.def("create", &create, "Create a new named Event.");
    m.def("destroy", &destroy, "Destroy a name-specified Event.");
    m.def("subscribe", &subscribe, "Subscribe a handler (function) to a named Event.");
    m.def("unsubscribe", &unsubscribe, "Unsubscribe a handler (function) from a named Event.");
    m.def("call", &call, "Sequentially call each EventHandler in an Event.");
    m.def("callAsyncBlocking", &callAsyncBlocking, "Call the same Event in multiple threads.");
    m.def("callAsync", &callAsync, "Run the Event's EventHandlers in their own, separate threads.");
}