#ifndef INPUT_H
#define INPUT_H

#include "plugin.h"
#include <vector>
#include <functional>

struct InputData
{
    // Keyboard input parameters
    char* key;
    bool keyDown;

    // Mouse input parameters
    bool leftMouseButtonPressed;
    bool rightMouseButtonPressed;
    bool mouseButtonPressed;
    bool doubleClick;
    bool horizontalMouseWheel;
    bool mouseMoved;
    bool verticalMouseWheel;
    bool unknownMouseEvent;
};

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
    void(*keyboardUpdate)(InputData);
    void(*mouseUpdate)(InputData);
    #endif
};

class Input : Plugin
{
public:
    virtual void initialize(size_t identifier) = 0;
    virtual void release() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void start2() = 0;
    virtual void push(const InputDesc& desc) = 0;
    virtual bool pop(const InputDesc& desc) = 0;

    static std::vector<InputDesc> inputDescriptors;
};

#endif //INPUT_H