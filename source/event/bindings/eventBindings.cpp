// Python bindings for EventStream, which exposes C++ EventStream functionality (creating a new EventStream,
// deleting an EventStream, creating/destroying Events using an existing EventStream, subscribing a function to 
// an Event, etc.) to python plugins. Because EventStream is a templated class, we expose to python just a few of 
// its more commonly-utilized template specializations.

#include "pybind11/functional.h"
#include "pybind11/stl.h"
#include "event.h"

namespace
{
    template<typename T>
    class EventStreamPython
    {
    private:
        EventStream<T>* es = nullptr;
    public:
        EventStreamPython(const char* scriptPath) 
        {
            #ifdef _WIN32
            std::string scriptPathStr = std::string(scriptPath) + "\\..\\..\\..\\bin\\";
            #elif __linux__
            std::string scriptPathStr = std::string(scriptPath) + "/../../../bin/";
            #endif
            es = EventStream<T>::Instance(reinterpret_cast<size_t>(scriptPathStr.c_str()));
        }

        void requestDelete()
        {
            es->requestDelete();
        }

        void create(const char* eventName)
        {
            es->create(eventName);
            std::cout << "Create pyEvent named " << eventName << std::endl;
        }

        void destroy(const char* eventName)
        {
            es->destroy(eventName);
            std::cout << "Destroy pyEvent named " << eventName << std::endl;
        }

        std::vector<size_t> subscribe(const char* eventName, const std::vector<std::function<void(T)>> handlerFuncs)
        {
            std::cout << "Subscribe to " << eventName << " using std::vector<size_t>-return type multiple_list subscribe." << std::endl;
            return es->subscribe(std::string(eventName), handlerFuncs);
        }

        // Note that this function overload of subscribe doesn't return anything; especially useful for multithreading in python.
        void subscribe(std::vector<size_t>& ids, const char* eventName, const std::vector<std::function<void(T)>>& handlerFuncs)
        {
            std::cout << "Subscribe to " << eventName << " using void-return type multiple_list subscribe." << std::endl;
            ids = es->subscribe(std::string(eventName), handlerFuncs);
        }

        std::vector<size_t> subscribe(const char* eventName, const pybind11::args args)
        {
            std::vector<std::function<void(T)>> funcVec;
            for (size_t i = 0; i < args.size(); ++i)
            {
                funcVec.push_back(args[i].cast<std::function<void(T)>>());
            }
            std::cout << "Subscribe to " << eventName << " using std::vector<size_t>-return type multiple_args subscribe." << std::endl;
            return es->subscribe(std::string(eventName), funcVec);
            return std::vector<size_t>();
        }

        // Note that this function overload of subscribe doesn't return anything; especially useful for multithreading in python.
        void subscribe(std::vector<size_t>& ids, const char* eventName, const pybind11::args args)
        {
            std::vector<std::function<void(T)>> funcVec;
            for (int i = 0; i < args.size(); ++i)
            {
                funcVec.push_back(args[i].cast<std::function<void(T)>>());
            }
            std::cout << "Subscribe to " << eventName << " using void-return type multiple_args subscribe." << std::endl;
            ids = es->subscribe(std::string(eventName), funcVec);
        }

        void unsubscribe(const char* eventName, const std::vector<size_t>& ids)
        {
            es->unsubscribe(std::string(eventName), ids);
            for (int i = 0; i < ids.size(); ++i)
            {
                std::cout << "Unsubscribed function with id " << ids[i] << " from " << eventName << " using multiple_list unsubscribe" << std::endl;
            }
        }

        void unsubscribe(const char* eventName, const pybind11::args args)
        {
            std::vector<size_t> ids;
            for (int i = 0; i < args.size(); ++i)
            {
                ids.push_back(ids[i]);
            }

            es->unsubscribe(std::string(eventName), ids);
            for (int i = 0; i < ids.size(); ++i)
            {
                std::cout << "Unsubscribed function with id " << ids[i] << " from " << eventName << " using multiple_args unsubscribe" << std::endl;
            }
        }

        void call(const char* eventName, T params)
        {
            es->call(eventName, params);
        }

        void callAsyncBlocking(const char* eventName, T params)
        {
            std::future<void> f = es->callAsyncBlocking(eventName, params);
        }

        void callAsync(const char* eventName, T params)
        {
            es->callAsync(eventName, params);
        }
    };

    // Here we define a templated function that in turn creates python bindings for the EventStream class. We then
    // call this function with specific template parameters in order to expose a specialized version of EventStream
    // to python (since we can't do so using a generic template parameter like in C++).
    template<typename T>
    void declare_eventstream(pybind11::module& m, std::string typestr)
    {
        std::string _typestr = typestr;
        _typestr[0] = std::toupper(_typestr[0]);
        std::string pyclass_name = std::string("EventStreamPython") + _typestr;
        pybind11::class_<EventStreamPython<T>>(m, pyclass_name.c_str(), pybind11::buffer_protocol(), pybind11::dynamic_attr())
            .def(pybind11::init<const char*>())
            .def("request_delete", &EventStreamPython<T>::requestDelete, "Attempt to delete the EventStream")
            .def("create", &EventStreamPython<T>::create, "Create a new named Event.")
            .def("destroy", &EventStreamPython<T>::destroy, "Destroy a name-specified Event.")
            .def("subscribe", static_cast<std::vector<size_t>(EventStreamPython<T>::*)(const char*, const std::vector<std::function<void(T)>>)>(&EventStreamPython<T>::subscribe), "Subscribe multiple handlers (functions) in a vector to a named Event; std::vector<size_t> return type.")
            .def("subscribe", static_cast<void (EventStreamPython<T>::*)(std::vector<size_t>&, const char*, const std::vector<std::function<void(T)>>&)>(&EventStreamPython<T>::subscribe), "Subscribe multiple handlers (functions) in a vector to a named Event simultaneously; void return type.")
            .def("subscribe", static_cast<std::vector<size_t>(EventStreamPython<T>::*)(const char*, const pybind11::args)>(&EventStreamPython<T>::subscribe), "Subscribe multiple handlers (functions) to a named Event; std::vector<size_t> return type.")
            .def("subscribe", static_cast<void(EventStreamPython<T>::*)(std::vector<size_t>&, const char*, const pybind11::args)>(&EventStreamPython<T>::subscribe), "Subscribe multiple handlers (functions) to a named Event; void return type.")
            .def("unsubscribe", static_cast<void (EventStreamPython<T>::*)(const char*, const std::vector<size_t>&)>(&EventStreamPython<T>::unsubscribe), "Unsubscribe multiple handlers (functions) in a vector from a named Event simultaneously.")
            .def("unsubscribe", static_cast<void(EventStreamPython<T>::*)(const char*, const pybind11::args)>(&EventStreamPython<T>::unsubscribe), "Unsubscribe multiple handlers (functions) in a vector from a named Event simultaneously.")
            .def("call", &EventStreamPython<T>::call, "Sequentially call each EventHandler in an Event.")
            .def("callAsyncBlocking", &EventStreamPython<T>::callAsyncBlocking, "Call the same Event in multiple threads.")
            .def("callAsync", &EventStreamPython<T>::callAsync, "Run the Event's EventHandlers in their own, separate threads.");
    }
}

PYBIND11_MODULE(eventPython, m)
{
    m.doc() = "pybind11 event python bindings"; // optional module docstring
    
    declare_eventstream<double>(m, "double");
    declare_eventstream<std::string>(m, "std::string");
    declare_eventstream<std::vector<double>>(m, "std::vector<double>");
    declare_eventstream<std::vector<std::string>>(m, "std::vector<std::string>");
}