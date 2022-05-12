#ifndef INPUT_H
#define INPUT_H

// Convenience macros to define how input will call functions that are pushed/subscribed to it. These are not necessary
// for developing custom plugins, they exist to more clearly highlight more options for structuring a plugin. Note that
// the input plugin has separate keyboard-specific and mouse-specific events, and also has the option of being called either
// on its own thread, or by pushing onto/subscribing to a Runner.
//#define EVENT_SYNC_KEYBOARD_I // Calls each handler in the input_keyboard event one-at-a-time, in sequence.
#define EVENT_ASYNC_KEYBOARD_I // Spawns new threads for each handler in input_keyboard to be executed in.
//#define EVENT_MULTI_KEYBOARD_I // Utilize the input_keyboard event in multiple different threads.
#define DIRECT_KEYBOARD_I // Call each InputDesc registered to the loaded input plugin containing a keyboard-bound function for updating.

//#define EVENT_SYNC_MOUSE_I // Calls each handler in the input_mouse event one-at-a-time, in sequence.
#define EVENT_ASYNC_MOUSE_I // Spawns new threads for each handler in input_mouse to be executed in.
//#define EVENT_MULTI_MOUSE_I // Utilize the input_mouse event in multiple different threads.
#define DIRECT_MOUSE_I // Call each InputDesc registered to the loaded input plugin containing a mouse-bound function for updating.

#define EVENT_RUNNER_I // Subscribe the input plugin to a runner event if Input is not being updated in its own thread.
//#define DIRECT_RUNNER_I // Push the input plugin to a locally-loaded Runner plugin if Input is not being updated in its own thread.

#include "plugin.h"
#include <vector>
#include <functional>

// Custom InputData struct for storing pertinent input information.
// TODO: Deal with non-ascii characters as well...
struct InputData
{
    // Keyboard input parameters.
    char* key;
    bool keyDown;

    // Mouse input parameters.
    bool leftMouseButtonPressed;
    bool leftMouseButtonReleased;
    bool rightMouseButtonPressed;
    bool rightMouseButtonReleased;
    bool mouseButtonPressed;
    bool mouseButtonReleased;
    bool doubleClick;
    bool horizontalMouseWheel;
    bool mouseMoved;
    bool verticalMouseWheel;
    bool unknownMouseEvent;
};

// InputDesc is a struct for holding methods we wish to have updated when the user performs a keystroke and/or mouse action.
// It contains the function to be updated, options for whether it should be updated when a key is pressed or a mouse is moved,
// a name, and a priority number (so that the order of method calls can be specified given a certain input).
#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
struct InputDesc
{
    int keyboardUpdatePriority;
    int mouseUpdatePriority;
    const char* name;
    #ifdef _WIN32
    void(__cdecl* cKeyboardUpdate)(InputData);
    void(__cdecl* cMouseUpdate)(InputData);
    std::function<void(InputData)> pyKeyboardUpdate;
    std::function<void(InputData)> pyMouseUpdate;
    #elif __linux__
    void(*cKeyboardUpdate)(InputData);
    void(*cMouseUpdate)(InputData);
    std::function<void(InputData)> pyKeyboardUpdate;
    std::function<void(InputData)> pyMouseUpdate;
    #endif
};
#endif

class Input : Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void start2() = 0;

#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
    virtual void push(const InputDesc& desc) = 0;
    virtual bool pop(const InputDesc& desc) = 0;

    static std::vector<InputDesc> inputDescriptors;
#endif
};

#endif //INPUT_H