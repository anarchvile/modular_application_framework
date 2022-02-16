//#include "event.h"

//template<typename... Args>
//EventStream<Args...>* EventStream<Args...>::m_instance = 0;
//template<typename... Args>
//Container* EventStream<Args...>::m_container = 0;
/*
///////////////////////////////////////////////////////////////////
// EventHandler
///////////////////////////////////////////////////////////////////
template<typename... Args>
EventHandler<Args...>::EventHandler(const std::function<void(Args...)>& handlerFunc)
    : m_handlerFunc(handlerFunc)
{
    m_handlerId = ++m_handlerIdCounter;
}

// copy constructor
template<typename... Args>
EventHandler<Args...>::EventHandler(const EventHandler<Args...>& src)
    : m_handlerFunc(src.m_handlerFunc), m_handlerId(src.m_handlerId)
{
}

// move constructor
template<typename... Args>
EventHandler<Args...>::EventHandler(EventHandler<Args...>&& src)
    : m_handlerFunc(std::move(src.m_handlerFunc)), m_handlerId(src.m_handlerId)
{
}

template<typename... Args>
size_t EventHandler<Args...>::id() const
{
    return m_handlerId;
}

// function call operator
template<typename... Args>
void EventHandler<Args...>::operator()(Args... params) const
{
    if (m_handlerFunc)
    {
        m_handlerFunc(params...);
    }
}

// event handler comparison operator
template<typename... Args>
bool EventHandler<Args...>::operator==(const EventHandler<Args...>& other) const
{
    return m_handlerId == other.m_handlerId;
}

// copy assignment operator
template<typename... Args>
EventHandler<Args...>& EventHandler<Args...>::operator=(const EventHandler<Args...>& src)
{
    m_handlerFunc = src.m_handlerFunc;
    m_handlerId = src.m_handlerId;

    return *this;
}

// move assignment operator
template<typename... Args>
EventHandler<Args...>& EventHandler<Args...>::operator=(EventHandler<Args...>&& src)
{
    std::swap(m_handlerFunc, src.m_handlerFunc);
    m_handlerId = src.m_handlerId;

    return *this;
}

///////////////////////////////////////////////////////////////////
// Event
///////////////////////////////////////////////////////////////////

// copy constructor
template <typename... Args>
Event<Args...>::Event(const Event<Args...>& src)
{
    std::lock_guard<std::mutex> lock(src.m_handlersLocker);

    m_handlers = src.m_handlers;
}

// move constructor
template <typename... Args>
Event<Args...>::Event(Event<Args...>&& src)
{
    std::lock_guard<std::mutex> lock(src.m_handlersLocker);

    m_handlers = std::move(src.m_handlers);
}

template <typename... Args>
size_t Event<Args...>::add(const EventHandler<Args...>& handler)
{
    std::lock_guard<std::mutex> lock(m_handlersLocker);

    m_handlers.push_back(handler);
    return handler.id();
}

template <typename... Args>
size_t Event<Args...>::add(const std::function<void(Args...)>& handler)
{
    return add(EventHandler<Args...>(handler));
}

template <typename... Args>
bool Event<Args...>::remove(const EventHandler<Args...>& handler)
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

template <typename... Args>
bool Event<Args...>::remove_id(const size_t& handlerId)
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

template <typename... Args>
void Event<Args...>::call(Args... params) const
{
    std::vector<EventHandler<Args...>> handlersCopy = getHandlersCopy();
    callImpl(handlersCopy, params...);
}

template <typename... Args>
void Event<Args...>::callImpl(const std::vector<EventHandler<Args...>>& handlers, Args... params) const
{
    for (const auto& handler : handlers)
    {
        handler(params...);
    }
}

template <typename... Args>
std::future<void> Event<Args...>::callAsync(Args... params) const
{
    return std::async(std::launch::async, [this](Args... asyncParams)
    { call(asyncParams...); }, params...);
}

template <typename... Args>
std::vector<EventHandler<Args...>> Event<Args...>::getHandlersCopy() const
{
    std::lock_guard<std::mutex> lock(m_handlersLocker);

    // Since the function return value is by copy, 
    // before the function returns (and destruct the lock_guard object),
    // it creates a copy of the m_handlers container.
    return m_handlers;
}

// copy assignment operator
template <typename... Args>
Event<Args...>& Event<Args...>::operator=(const Event<Args...>& src)
{
    std::lock_guard<std::mutex> lock(m_handlersLocker);
    std::lock_guard<std::mutex> lock2(src.m_handlersLocker);

    m_handlers = src.m_handlers;

    return *this;
}

// move assignment operator
template<typename... Args>
Event<Args...>& Event<Args...>::operator=(Event<Args...>&& src)
{
    std::lock_guard<std::mutex> lock(m_handlersLocker);
    std::lock_guard<std::mutex> lock2(src.m_handlersLocker);

    std::swap(m_handlers, src.m_handlers);

    return *this;
}
*/