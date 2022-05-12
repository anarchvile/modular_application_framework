// This is a very simple example plug-in that is meant to both illustrate the process behind
// correctly structuring a plug-in, and highlighting how a plug-in could be expanded to perform
// more interesting tasks.

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include "helloWorldImpl.h"

#if defined(EVENT_RUNNER_HW) || defined(EVENT_KEYBOARD_INPUT_HW) || defined(EVENT_MOUSE_INPUT_HW)
#include "event.h"
#endif

#if defined(DIRECT_RUNNER_HW) || defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
#include "pluginManager.h"
#ifdef DIRECT_RUNNER_HW
#include "runner.h"
#endif
#endif

#if defined(EVENT_KEYBOARD_INPUT_HW) || defined(EVENT_MOUSE_INPUT_HW) || defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
#include "input.h"
#endif

// Note that es1 is an EventStream of type double, which means that all Events managed by this EventStream will contain
// handlers whose arguments are a single variable of type double (e.g. a runner event, whose handles will take a single 
// delta-time double).
#ifdef EVENT_RUNNER_HW
EventStream<double>* es1;
std::vector<size_t> g_ids;
#endif
// Note that es2 is an EventStream of type InputData, which means that all Events managed by this EventStream will contain
// handlers whose arguments are a single variable of type InputData (e.g. an input_keyboard event, whose handles will take
// a single InputData instance).
#if defined(EVENT_KEYBOARD_INPUT_HW) || defined(EVENT_MOUSE_INPUT_HW)
EventStream<InputData>* es2;
#endif
#ifdef EVENT_KEYBOARD_INPUT_HW
std::vector<size_t> g_id_k;
#endif
#ifdef EVENT_MOUSE_INPUT_HW
std::vector<size_t> g_id_m;
#endif

#if defined(DIRECT_RUNNER_HW) || defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
PluginManager* pm;
#ifdef DIRECT_RUNNER_HW
Runner* runner = nullptr;
RunnerDesc runnerDesc;
#endif
#if defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
Input* input = nullptr;
InputDesc inputDescKey1, inputDescKey2, inputDescMouse1, inputDescMouse2;
#endif
#endif

size_t g_counter = 0;
HelloWorldInterface hwInterface;

#if defined(EVENT_RUNNER_HW) || defined(DIRECT_RUNNER_HW)
// A locally-defined function we wish to register to the runner for updating on the tick. In this 
// case our function simply calls the class's member different update functions, but this is not 
// required and could be easily changed to something else.
void runnerUpdate(double dt)
{
    HelloWorldImpl hwi;
    hwi.preupdate(dt);
    hwi.update(dt);
    hwi.postupdate(dt);
}
#endif

#if defined(EVENT_KEYBOARD_INPUT_HW) || defined(EVENT_MOUSE_INPUT_HW) || defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
// Some locally-defined functions we wish to register to the input plugin so that they are called whenever
// a key is pressed/mouse is moved.
void keyUpdate1(InputData in_data)
{
    std::cout << "Function #1 in helloWorldImpl.cpp that gets registerd to the input plugin for updating whenever a " 
        << "key is pressed/released!" << std::endl;
}

void keyUpdate2(InputData in_data)
{
    std::cout << "Function #2 in helloWorldImpl.cpp that gets registerd to the input plugin for updating whenever a "
        << "key is pressed/released!" << std::endl;
}

void mouseUpdate1(InputData in_data)
{
    std::cout << "Function #1 in helloWorldImpl.cpp that gets registerd to the input plugin for updating whenever the "
        << "mouse is moved!" << std::endl;
}

void mouseUpdate2(InputData in_data)
{
    std::cout << "Function #2 in helloWorldImpl.cpp that gets registerd to the input plugin for updating whenever the "
        << "mouse is moved!" << std::endl;
}
#endif

// A simple function we wish to later pass by reference to our helloWorld interface.
const char* helloWorldFunc()
{
    return "A helloWorldFunc function defined in helloWorldImpl.cpp";
}

// Define plug-in behavior immediately after being loaded. Simulate complex computations upon loading by
// delaying the program by 3 seconds. Loads a combination of runner/input plugins and EventStreams depending
// on the defined macros. Pass by reference both our printHelloWorld() function to the public interface.
void HelloWorldImpl::initialize(size_t identifier)
{
    std::cout << "HelloWorldImpl::initialize: " << identifier << std::endl;

    // Create an instance of a double-type EventStream if we'll be subscribing methods to runner's event.
#ifdef EVENT_RUNNER_HW
    es1 = EventStream<double>::Instance(identifier);
#endif
    // Create an instance of an InputData-type EventStream if we'll be subscribing methods to input's event.
#if defined (EVENT_KEYBOARD_INPUT_HW) || defined(EVENT_MOUSE_INPUT_HW)
    es2 = EventStream<InputData>::Instance(identifier);
#endif
    // Get a PluginManager instance if we'll be directly loading the runner and/or input plugin.
#if defined(DIRECT_RUNNER_HW) || defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
    pm = PluginManager::Instance(identifier);
#endif
    std::cout << "Simulate complex computation during HelloWorld's initialization" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));

    hwInterface.helloWorldFunc = &(::helloWorldFunc);
}

// Define plug-in behavior immediately before being unloaded. Simulate complex computations prior to unloading by
// delaying the program by 3 seconds. Unload any plugins that might've been loaded upon initialization.
void HelloWorldImpl::release()
{
    std::cout << "HelloWorldImpl::release" << std::endl;
    std::cout << "Simulate complex computation during HelloWorld's release" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
#ifdef EVENT_RUNNER_HW
    es1->requestDelete();
    es1 = nullptr;
#endif
#if defined (EVENT_KEYBOARD_INPUT_HW) || defined(EVENT_MOUSE_INPUT_HW)
    es2->requestDelete();
    es2 = nullptr;
#endif
#ifdef DIRECT_RUNNER_HW
    pm->Unload("runner");
    runner = nullptr;
#endif
#if defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
    pm->Unload("input");
    input = nullptr;
#endif

#if defined (DIRECT_RUNNER_HW) || defined(DIRECT_KEYBOARD_INPUT_HW) || defined(DIRECT_MOUSE_INPUT_HW)
    pm->requestDelete();
#endif
}

// Starts HelloWorld plugin by pushing and/or subscribing functions to a variety of plugins and events.
void HelloWorldImpl::start()
{
    std::cout << "HelloWorldImpl::start" << std::endl;
    // Subscribe runnerUpdate to the runner Event using es1 (the double-type EventStream).
#ifdef EVENT_RUNNER_HW
    g_ids = es1->subscribe("runner", &(::runnerUpdate));
#endif
    // Load the Runner plugin using the PluginManager, directly create a RunnerDesc for runnerUpdate, and push it to the
    // Runner for updating.
#ifdef DIRECT_RUNNER_HW
    if (runner == nullptr)
    {
        runner = (Runner*)pm->Load("runner");
    }
    runnerDesc.priority = 1;
    runnerDesc.name = "HelloWorldImpl::runnerUpdate";
    runnerDesc.cUpdate = &(::runnerUpdate);
    runnerDesc.pyUpdate = nullptr;
    runner->push(runnerDesc);
#endif
    // Subscribe keyUpdate1 and keyUpdate2 to the input_keyboard Event using es2 (the InputData-type EventStream).
#ifdef EVENT_KEYBOARD_INPUT_HW
    g_id_k = es2->subscribe("input_keyboard", &(::keyUpdate1), &(::keyUpdate2));
#endif
    // Load the Input plugin using the PluginManager, directly create two InputDesc for keyUpdate1 and keyUpdate2,
    // and push them to Input so that they get called each time a keystroke is registered.
#ifdef DIRECT_KEYBOARD_INPUT_HW
    if (input == nullptr)
    {
        input = (Input*)pm->Load("input");
    }
    inputDescKey1.keyboardUpdatePriority = 1;
    inputDescKey1.name = "HelloWorldImpl::keyUpdate1";
#ifdef _WIN32
    inputDescKey1.cKeyboardUpdate = &(::keyUpdate1);
    inputDescKey1.cMouseUpdate = nullptr;
    inputDescKey1.pyKeyboardUpdate = nullptr;
    inputDescKey1.pyMouseUpdate = nullptr;
#elif __linux__
    inputDescKey1.keyboardUpdate = &(::keyUpdate1);
    inputDescKey1.mouseUpdate = nullptr;
#endif
    input->push(inputDescKey1);

    inputDescKey2.keyboardUpdatePriority = 2;
    inputDescKey2.name = "HelloWorldImpl::keyUpdate2";
#ifdef _WIN32
    inputDescKey2.cKeyboardUpdate = &(::keyUpdate2);
    inputDescKey2.cMouseUpdate = nullptr;
    inputDescKey2.pyKeyboardUpdate = nullptr;
    inputDescKey2.pyMouseUpdate = nullptr;
#elif __linux__
    inputDescKey2.keyboardUpdate = &(::keyUpdate2);
    inputDescKey2.mouseUpdate = nullptr;
#endif
    input->push(inputDescKey2);
#endif

    // Subscribe mouseUpdate1 and mouseUpdate2 to the input_mouse Event using es2 (the InputData-type EventStream).
#ifdef EVENT_MOUSE_INPUT_HW
    g_id_m = es2->subscribe("input_mouse", &(::mouseUpdate1), &(::mouseUpdate2));
#endif
    // Load the Input plugin using the PluginManager, directly create two InputDesc for mouseUpdate1 and mouseUpdate2,
    // and push them to Input so that they get called each time a mouse movement is registered.
#ifdef DIRECT_MOUSE_INPUT_HW
    if (input == nullptr)
    {
        input = (Input*)pm->Load("input");
    }
    inputDescMouse1.mouseUpdatePriority = 1;
    inputDescMouse1.name = "HelloWorldImpl::mouseUpdate1";
#ifdef _WIN32
    inputDescMouse1.cMouseUpdate = &(::mouseUpdate1);
    inputDescMouse1.cKeyboardUpdate = nullptr;
    inputDescMouse1.pyKeyboardUpdate = nullptr;
    inputDescMouse1.pyMouseUpdate = nullptr;
#elif __linux__
    inputDescMouse1.cKeyboardUpdate = nullptr;
    inputDescMouse1.cMouseUpdate = &(::mouseUpdate1);
#endif
    input->push(inputDescMouse1);

    inputDescMouse2.mouseUpdatePriority = 2;
    inputDescMouse2.name = "HelloWorldImpl::mouseUpdate1";
#ifdef _WIN32
    inputDescMouse2.cMouseUpdate = &(::mouseUpdate2);
    inputDescMouse2.cKeyboardUpdate = nullptr;
    inputDescMouse2.pyKeyboardUpdate = nullptr;
    inputDescMouse2.pyMouseUpdate = nullptr;
#elif __linux__
    inputDescMouse2.cKeyboardUpdate = nullptr;
    inputDescMouse2.cMouseUpdate = &(::mouseUpdate2);
#endif
    input->push(inputDescMouse2);

#endif
}

// Stops the runnerUpdate, keyUpdate1, keyUpdate2, mouseUpdate1, and/or mouseUpdate2 functions from
// being called through a combination of popping off and unsubscribing these functions from the 
// corresponding plugins/events they were initially pushed/subscribed to.
void HelloWorldImpl::stop()
{
    std::cout << "HelloWorldImpl::stop" << std::endl;
#ifdef EVENT_RUNNER_HW
    es1->unsubscribe("runner", g_ids);
#endif
#ifdef DIRECT_RUNNER_HW
    runner->pop(runnerDesc);
#endif

#ifdef EVENT_KEYBOARD_INPUT_HW
    es2->unsubscribe("input_keyboard", g_id_k);
#endif
#ifdef DIRECT_KEYBOARD_INPUT_HW
    input->pop(inputDescKey1);
    input->pop(inputDescKey2);
#endif

#ifdef EVENT_MOUSE_INPUT_HW
    es2->unsubscribe("input_mouse", g_id_m);
#endif
#ifdef DIRECT_MOUSE_INPUT_HW
    input->pop(inputDescMouse1);
    input->pop(inputDescMouse2);
#endif
}

// Simple preupdate function with optional sleep call, to slow down iteration for debugging/visualization purposes.
void HelloWorldImpl::preupdate(double dt)
{
    std::cout << "HelloWorldImpl::preupdate " << dt << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(1));
}

// Simple update method that can signal to the runner to unregister itself from the update cycle after 10 iterations.
// Also has an optional sleep call to slow down iteration for debugging/visualization purposes.
void HelloWorldImpl::update(double dt)
{
    std::cout << "HelloWorldImpl::update " << dt << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(2));
#ifdef DIRECT_RUNNER_HW
    if (g_counter == 10)
    {
        runner->pop(runnerDesc);
    }

    ++g_counter;
#endif
}

// Simple postupdate function with optional sleep call, to slow down iteration for debugging/visualization purposes.
void HelloWorldImpl::postupdate(double dt)
{
    std::cout << "HelloWorldImpl::postupdate " << dt << std::endl;
    //std::this_thread::sleep_for(std::chrono::seconds(3));
}

// Returns a custom interface to this plug-in, which can then be utilized inside other plug-ins
// to call the functions attached to the interface initially. In the specific case of helloWorldImpl,
// we've attached a helloWorldFunc() method to the public interface in our initialize method, so once
// helloWorld is loaded inside another context, we could create an instance of the interface to then
// call helloWorldFunc(), even though we would not actually be inside helloWorld!
HelloWorldInterface HelloWorldImpl::getInterface()
{
    return hwInterface;
}

// Define functions with C symbols (create/destroy HelloWorldImpl instance)
HelloWorldImpl* g_helloworldimpl = nullptr;

extern "C" HELLOWORLD HelloWorldImpl* Create()
{
    assert(g_helloworldimpl == nullptr);
    g_helloworldimpl = new HelloWorldImpl;
    return g_helloworldimpl;
}
extern "C" HELLOWORLD void Destroy()
{
    assert(g_helloworldimpl);
    delete g_helloworldimpl;
    g_helloworldimpl = nullptr;
}
