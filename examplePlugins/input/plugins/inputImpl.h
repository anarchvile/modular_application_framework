#ifndef INPUTIMPL_H
#define INPUTIMPL_H

#ifdef _WIN32
    #ifdef INPUT_EXPORTS
        #define INPUT __declspec(dllexport)
    #else
        #define INPUT __declspec(dllimport)
    #endif
#elif __linux__
    #define INPUT
#endif

#include "input.h"
#include <thread>

class InputImpl : public Input
{
    void initialize(size_t identifier);
    void release();
    void start();
    void stop();

    void start2();

#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
    void push(const InputDesc& desc);
    bool pop(const InputDesc& desc);
#endif

    std::thread thread;
};

#ifdef DIRECT_KEYBOARD_I
struct keyboard_priority_queue
{
    inline bool operator() (const InputDesc& inputDesc1, const InputDesc& inputDesc2)
    {
        return (inputDesc1.keyboardUpdatePriority < inputDesc2.keyboardUpdatePriority);
    }
};
#endif // DIRECT_KEYBOARD_I

#ifdef DIRECT_MOUSE_I
struct mouse_priority_queue
{
    inline bool operator() (const InputDesc& inputDesc1, const InputDesc& inputDesc2)
    {
        return (inputDesc1.mouseUpdatePriority < inputDesc2.mouseUpdatePriority);
    }
};
#endif // DIRECT_MOUSE_I

#endif // INPUTIMPL_H