// TODO:
// 1. Abstract runner and input plugins by creating an overaching Event plugin, which will have register/unregister functionality. Runner
//    and input will then be able to load that Event.dll plugin and acquire an interface to it in order to utilize said methods.
// 2. Get entire project to compile/work in linux.
// 3. Python interface (seeing if we can get rid of std::function in plugin runner/input interfaces).
// 4. Code to handle situations where we try to call start/stop multiple times on a plugin.
// 5. is_async should be publically accessible via python.
// 6. Keep testing different loading configurations.
// 7. Add more thorough comments everywhere.
// 8. Eventstream system - plugins like runner and input can subscribe to this Eventstream, and from there other plugins can
//     access Evenstream to choose what plugin they'd like to push functions to. Eventstream basically becomes a middle-man
//     between plugins that have functions to update and plugins that do the actual updating. Need a generic way to pass payloads
//     to runner/input/etc. (since the RunnerDesc update functions take a double, InputDesc update functions take a custom Event struct, etc.)

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>
#include <fstream>
#include <string>

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "inputImpl.h"

#if defined(EVENT_KEYBOARD_I) || defined(EVENT_MOUSE_I) || defined(EVENT_RUNNER_I)
#include "event.h"
#endif
#if defined(DIRECT_RUNNER_I)
#include "pluginManager.h"
#include "runner.h"
#endif

#if defined(EVENT_KEYBOARD_I) || defined(EVENT_MOUSE_I) || defined(EVENT_RUNNER_I)
EventStream<double>* es;
size_t g_id1, g_id2, g_id3;
#endif

#ifdef DIRECT_RUNNER_I
PluginManager* pm;
Runner* runner;
RunnerDesc rDesc;
#endif

#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
std::vector<InputDesc> Input::inputDescriptors;
#endif

bool breakLoop = false;
bool g_block = false;
bool isAsync;

HANDLE hStdin;
DWORD fdwSaveOldMode;
DWORD cNumRead, fdwMode, i;
INPUT_RECORD irInBuf[128];
COORD coord;

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

void KeyEventProc(KEY_EVENT_RECORD ker, InputData &inputData)
{
    char keyChar = ker.uChar.AsciiChar;
    std::string keyString(1, keyChar);
    inputData.key = new char[keyString.size()];
    strcpy_s(inputData.key, sizeof inputData.key, keyString.c_str());
    inputData.keyDown = ker.bKeyDown;
    
    if (inputData.keyDown)
    {
        std::cout << "Key event: key pressed, " << keyString << std::endl;
    }
    else
    {
        std::cout << "Key event: key released, " << keyString << std::endl;
    }
}

void MouseEventProc(MOUSE_EVENT_RECORD mer, InputData &inputData)
{
    #ifndef MOUSE_HWHEELED
        #define MOUSE_HWHEELED 0x0008
    #endif
    switch (mer.dwEventFlags)
    {
        case 0:
            if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
                std::cout << "Mouse event: left button press: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
                inputData.leftMouseButtonPressed = true;
            }
            else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                std::cout << "Mouse event: right button press: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
                inputData.rightMouseButtonPressed = true;
            }
            else
            {
                std::cout << "Mouse event: button press: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
                inputData.mouseButtonPressed = true;
            }
            break;
        case DOUBLE_CLICK:
            std::cout << "Mouse event: double click: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.doubleClick = true;
            break;
        case MOUSE_HWHEELED:
            std::cout << "Mouse event: horizontal mouse wheel: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.horizontalMouseWheel = true;
            break;
        case MOUSE_MOVED:
            std::cout << "Mouse event: mouse moved: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.mouseMoved = true;
            break;
        case MOUSE_WHEELED:
            std::cout << "Mouse event: vertical mouse wheel: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.verticalMouseWheel = true;
            break;
        default:
            std::cout << "Mouse event: unknown: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.unknownMouseEvent = true;
            break;
    }
}

void ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr, InputData &inputData)
{
    std::cout << "Resize event" << std::endl;
    std::cout << "Console screen buffer is " << wbsr.dwSize.X << " columns by" << wbsr.dwSize.Y << " rows" << std::endl;
}

void parseConfigFile(std::string filePath)
{
    std::string configFilePath = filePath;
    std::ifstream cFile(configFilePath);
    if (cFile.is_open())
    {
        std::string line;
        while (getline(cFile, line))
        {
            line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
            if (line[0] == '#' || line.empty())
            {
                continue;
            }

            size_t delimiterPos = line.find("=");
            std::string type = line.substr(0, delimiterPos);
            if (type == "is_async")
            {
                std::string _so;
                for (size_t i = delimiterPos + 1; i <= line.length(); ++i)
                {
                    if (line[i] == ',' || i == line.length())
                    {
                        if (_so == "false")
                        {
                            isAsync = false;
                        }
                        else
                        {
                            isAsync = true;
                            if (_so != "true")
                            {
                                std::cout << "Unable to read is_async flag; default to set is_async to true" << std::endl;
                            }
                        }
                        _so.clear();
                    }
                    else
                    {
                        _so.push_back(line[i]);
                    }
                }
            }
        }
    }

    else
    {
        std::cerr << "Couldn't open config file module for reading.\n";
    }
}

void internalIteration(double dt)
{
    // Wait for the events. 
    if (!ReadConsoleInput(
        hStdin,      // input buffer handle 
        irInBuf,     // buffer to read into 
        128,         // size of read buffer 
        &cNumRead))  // number of records read 
        ErrorExit("ReadConsoleInput");

    // Dispatch the events to the appropriate handler.
    for (i = 0; i < cNumRead; i++)
    {
        InputData inputData;
        switch (irInBuf[i].EventType)
        {
            case KEY_EVENT: // keyboard input
                std::sort(Input::inputDescriptors.begin(), Input::inputDescriptors.end(), keyboard_priority_queue());
                KeyEventProc(irInBuf[i].Event.KeyEvent, inputData);
                // TODO: Create a keyboard input event system, process handles (functions) subscribed to keyboard event here as well
                for (InputDesc desc : Input::inputDescriptors)
                {
                    if (desc.cKeyboardUpdate != nullptr && desc.pyKeyboardUpdate == nullptr)
                    {
                        desc.cKeyboardUpdate(inputData);
                    }
                    else if (desc.cKeyboardUpdate == nullptr && desc.pyKeyboardUpdate != nullptr)
                    {
                        desc.pyKeyboardUpdate(inputData);
                    }
                }
                break;

            case MOUSE_EVENT: // mouse input
                std::sort(Input::inputDescriptors.begin(), Input::inputDescriptors.end(), mouse_priority_queue());
                MouseEventProc(irInBuf[i].Event.MouseEvent, inputData);
                // TODO: Create a mouse input event system, process handles (functions) subscribed to keyboard event here as well
                for (InputDesc desc : Input::inputDescriptors)
                {
                    if (desc.cMouseUpdate != nullptr && desc.pyMouseUpdate == nullptr)
                    {
                        desc.cMouseUpdate(inputData);
                    }

                    else if (desc.cMouseUpdate == nullptr && desc.pyMouseUpdate != nullptr)
                    {
                        desc.pyMouseUpdate(inputData);
                    }
                }
                break;

            case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing 
                ResizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent, inputData);
                break;

            case FOCUS_EVENT: // disregard focus events 

            case MENU_EVENT: // disregard menu events 
                break;

            default:
                ErrorExit("Unknown event type");
                break;
        }
    }
}

// Define what processes the plug-in runs once it is loaded into a program by the plugin manager.
void InputImpl::initialize(size_t identifier)
{
    std::cout << "InputImpl::initialize" << std::endl;

    // Get the standard input handle. 
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");

    // Save the current input mode, to be restored on exit. 
    if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
        ErrorExit("GetConsoleMode");

    // Enable the window and mouse input events.
    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_INSERT_MODE | ENABLE_EXTENDED_FLAGS;
    if (!SetConsoleMode(hStdin, fdwMode))
        ErrorExit("SetConsoleMode");

    // Read local config file to determine sync status.
    const char* exeDir = reinterpret_cast<const char*>(identifier);
    std::string configPath = std::string(exeDir) + "..\\plugins\\input\\start_option.cfg";
    parseConfigFile(configPath);

#if defined(EVENT_KEYBOARD_I) || defined(EVENT_MOUSE_I) || defined(EVENT_RUNNER_I)
    es = EventStream<double>::Instance();
#endif
#ifdef DIRECT_RUNNER_I
    pm = PluginManager::Instance(identifier);
#endif
}

// Define what processes the plug-in runs immediately before it is unloaded the plugin manager.
void InputImpl::release()
{
    std::cout << "InputImpl::release" << std::endl;
#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
    inputDescriptors.clear();
#endif

    if (!isAsync)
    {
#if defined(EVENT_KEYBOARD_I) || defined(EVENT_MOUSE_I) || defined(EVENT_RUNNER_I)
        es = nullptr;
#endif
#ifdef DIRECT_RUNNER_I
        pm->Unload("runner");
#endif
    }

    SetConsoleMode(hStdin, fdwSaveOldMode);
}

void InputImpl::start()
{
    std::cout << "InputImpl::start" << std::endl;

    if (isAsync)
    {
        thread = std::thread(&InputImpl::start2, InputImpl());
    }
    else
    {
        start2();
    }
}

// Sets a parameter to break the update loop if asynchronous, or to pop off from runner update loop if synchronous.
void InputImpl::stop()
{
    std::cout << "InputImpl::stop" << std::endl;
    if (isAsync)
    {
        breakLoop = true;
        //thread.detach(); // TODO: Not sure if this is right...
        thread.join();
    }

    else
    {
#ifdef EVENT_RUNNER_I
        es->unsubscribe("runner", g_id1);
#endif
#ifdef DIRECT_RUNNER_I
        runner->pop(rDesc);
#endif
    }
}

void InputImpl::start2()
{
    std::cout << "InputImpl::start2" << std::endl;
    if (isAsync)
    {
        while (!breakLoop)
        {
            if (g_block) continue;

            internalIteration(0);
        }
    }

    else
    {
#ifdef EVENT_RUNNER_I
        g_id1 = es->subscribe("runner", &(::internalIteration));
#endif
#ifdef DIRECT_RUNNER_I
        runner = (Runner*)pm->Load("runner");
        rDesc.priority = 1;
        rDesc.name = "InputImpl::internalIteration";
        rDesc.cUpdate = &(::internalIteration);
        rDesc.pyUpdate = nullptr;
        
        runner->push(rDesc);
#endif
    }
}

// Registers the descriptors of functions that have been newly assigned for updating in the main loop.
#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
void InputImpl::push(const InputDesc& desc)
{
    std::cout << "InputImpl::push" << std::endl;
    g_block = true;
    size_t cnt = 0;
    for (unsigned int i = 0; i < Input::inputDescriptors.size(); ++i)
    {
        if (strcmp(Input::inputDescriptors[i].name, desc.name) == 0 && (Input::inputDescriptors[i].keyboardUpdatePriority == desc.keyboardUpdatePriority || Input::inputDescriptors[i].mouseUpdatePriority == desc.mouseUpdatePriority) && Input::inputDescriptors[i].cKeyboardUpdate == desc.cKeyboardUpdate && Input::inputDescriptors[i].cMouseUpdate == desc.cMouseUpdate)
        {
            return;
        }
        else
        {
            ++cnt;
        }
    }

    if (cnt == Input::inputDescriptors.size()) Input::inputDescriptors.push_back(desc);
    g_block = false;
}
#endif

// Removes function descriptors that have been designated for unsubscription from the main update cycle.
#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
bool InputImpl::pop(const InputDesc& desc)
{
    std::cout << "InputImpl::pop" << std::endl;
    g_block = true;
    size_t count = Input::inputDescriptors.size();
    std::vector<InputDesc>::iterator it = Input::inputDescriptors.begin();
    while (it != Input::inputDescriptors.end())
    {
        if (strcmp(it->name, desc.name) == 0 && (it->keyboardUpdatePriority == desc.keyboardUpdatePriority || it->mouseUpdatePriority == desc.mouseUpdatePriority) && it->cKeyboardUpdate == desc.cKeyboardUpdate && it->cMouseUpdate == desc.cMouseUpdate)
        {
            it = Input::inputDescriptors.erase(it);
        }
        else
        {
            ++it;
        }
    }
    g_block = false;
    return Input::inputDescriptors.size() != count;
}
#endif

// Define functions with C symbols (create/destroy Input instance)
InputImpl* g_input = nullptr;

extern "C" INPUT InputImpl* Create()
{
    if (g_input != nullptr)
    {
        return g_input;
    }

    else
    {
        g_input = new InputImpl;
        return g_input;
    }
}
extern "C" INPUT void Destroy()
{
    assert(g_input);
    delete g_input;
    g_input = nullptr;
}
