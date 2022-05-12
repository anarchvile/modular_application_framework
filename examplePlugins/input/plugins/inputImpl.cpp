// The Input plugin is responsible for detecting user inputs (both keyboard and mouse) in a console/terminal, and allowing
// a developer to attach functions/processes to those inputs so that they get called/executed whenever some key and/or
// mouse action is taken. Similar to the Runner plugin, functions can be tied to inputs either directly through the use
// of an InputDesc that gets passed along to a locally-loaded Input plugin, or via a subscription to a corresponding input
// event. In this example we only have two such events: input_keyboard for handles meant to be called when a keystroke is
// detected, and input_mouse for handles meant to be called when the mouse moves (although we could create more specific
// events, such as one whose handles get called when the letter 'k' on a keyboard is pressed, for instance). Note that the
// Input plugin can be ran either completely asynchronously in its own thread (as specified by its start_option config file),
// or it could be attached to a Runner plugin/event for updating.

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>
#include <fstream>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#elif __linux__
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#else
#error define your compiler
#endif

#include "inputImpl.h"

#if defined(EVENT_SYNC_KEYBOARD_I) || defined(EVENT_ASYNC_KEYBOARD_I) || defined(EVENT_MULTI_KEYBOARD_I) \
|| defined(EVENT_SYNC_MOUSE_I) || defined(EVENT_ASYNC_MOUSE_I) || defined(EVENT_MULTI_MOUSE_I) \
|| defined(EVENT_RUNNER_I)
#include "event.h"
#endif
#if defined(DIRECT_RUNNER_I)
#include "pluginManager.h"
#include "runner.h"
#endif

#if defined(EVENT_SYNC_KEYBOARD_I) || defined(EVENT_ASYNC_KEYBOARD_I) || defined(EVENT_MULTI_KEYBOARD_I) \
|| defined(EVENT_SYNC_MOUSE_I) || defined(EVENT_ASYNC_MOUSE_I) || defined(EVENT_MULTI_MOUSE_I) \
|| defined(EVENT_RUNNER_I)
EventStream<double>* es;
std::vector<size_t> g_ids;
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
std::vector<std::thread> kbt, mt;

#if defined(EVENT_MULTI_KEYBOARD_I) || defined(EVENT_MULTI_MOUSE_I)
void callAsyncWrapper(std::string name)
{
    es->callAsync(name, 0);
}
#endif

#ifdef _WIN32
HANDLE hStdin;
DWORD fdwSaveOldMode;
DWORD cNumRead, fdwMode;
INPUT_RECORD irInBuf[128];
COORD coord;

void ErrorExit(LPCSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

// Records and prints out what key event in the console just took place (i.e. which key(s) was pressed and/or released).
void KeyEventProc(KEY_EVENT_RECORD ker, InputData &inputData)
{
    char keyChar = ker.uChar.AsciiChar;
    std::string keyString(1, keyChar);
    inputData.key = new char[keyString.size()];
    strcpy_s(inputData.key, sizeof(inputData.key), keyString.c_str());
    inputData.keyDown = ker.bKeyDown;
    
    if (inputData.keyDown)
    {
        std::cout << "Key pressed: " << keyString << std::endl;
    }
    else
    {
        std::cout << "Key released: " << keyString << std::endl;
    }
}

// Records and prints out what mouse event in the console just took place (i.e. mouse movement, left mouse button clicked, etc.).
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
                std::cout << "Mouse button pressed: left button: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
                inputData.leftMouseButtonPressed = true;
            }
            else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                std::cout << "Mouse button pressed: right button: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
                inputData.rightMouseButtonPressed = true;
            }
            else
            {
                std::cout << "Mouse button pressed: other button: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
                inputData.mouseButtonPressed = true;
            }
            break;
        case DOUBLE_CLICK:
            std::cout << "Mouse double click: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.doubleClick = true;
            break;
        case MOUSE_HWHEELED:
            std::cout << "Horizontal mouse wheel: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.horizontalMouseWheel = true;
            break;
        case MOUSE_MOVED:
            std::cout << "Mouse moved: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.mouseMoved = true;
            break;
        case MOUSE_WHEELED:
            std::cout << "Vertical mouse wheel: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.verticalMouseWheel = true;
            break;
        default:
            std::cout << "Unknown mouse action: (" << mer.dwMousePosition.X << ", " << mer.dwMousePosition.Y << ")" << std::endl;
            inputData.unknownMouseEvent = true;
            break;
    }
}

// Detects and output that the console has been resized.
void ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr, InputData &inputData)
{
    std::cout << "Resize event" << std::endl;
    std::cout << "Console screen buffer is " << wbsr.dwSize.X << " columns by" << wbsr.dwSize.Y << " rows" << std::endl;
}

#elif __linux__
Display* dpy;

void die(const char* s) 
{
    perror(s);
    exit(1);
}

struct termios orig_termios;

void disableRawMode() 
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    {
        die("tcsetattr");
    }
}
void enableRawMode() 
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");

    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}
#endif

// Parses the start_option config file that specifies whether or not input should operate in its own thread.
void parseStartConfigFile(std::string filePath)
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

// Responsible for reading in all console inputs and calling the appropriate event handlers/InputDescs based on what
// keystrokes and mouse actions the system recorded during the latest tick.
void internalIteration(double dt)
{
    //std::cout << "internalIteration is also going" << std::endl;
#ifdef _WIN32
    // Wait for the events. 
    if (!ReadConsoleInput(
        hStdin,      // input buffer handle 
        irInBuf,     // buffer to read into 
        128,         // size of read buffer 
        &cNumRead))  // number of records read 
        ErrorExit("ReadConsoleInput");

    // Dispatch the events to the appropriate handler.
    for (DWORD i = 0; i < cNumRead; i++)
    {
        InputData inputData;
        switch (irInBuf[i].EventType)
        {
            case KEY_EVENT: // Keyboard input.
                KeyEventProc(irInBuf[i].Event.KeyEvent, inputData);

#ifdef EVENT_SYNC_KEYBOARD_I
                es->call("input_keyboard", 0);
#elif defined(EVENT_ASYNC_KEYBOARD_I)
                es->callAsync("input_keyboard", 0);
#elif defined(EVENT_MULTI_KEYBOARD_I)
                kbt.push_back(std::thread(callAsyncWrapper, "input_keyboard"));
#endif

#ifdef DIRECT_KEYBOARD_I
                std::sort(Input::inputDescriptors.begin(), Input::inputDescriptors.end(), keyboard_priority_queue());
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
#endif
                break;

            case MOUSE_EVENT: // Mouse input.
                MouseEventProc(irInBuf[i].Event.MouseEvent, inputData);

#ifdef EVENT_SYNC_MOUSE_I
                es->call("input_mouse", 0);
#elif defined(EVENT_ASYNC_MOUSE_I)
                es->callAsync("input_mouse", 0);
#elif defined(EVENT_MULTI_MOUSE_I)
                mt.push_back(std::thread(callAsyncWrapper, "input_mouse"));

#endif

#ifdef DIRECT_MOUSE_I
                std::sort(Input::inputDescriptors.begin(), Input::inputDescriptors.end(), mouse_priority_queue());
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
#endif
                break;

            case WINDOW_BUFFER_SIZE_EVENT: // Screen buf. resizing.
                ResizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent, inputData);
                break;

            case FOCUS_EVENT: // Disregard focus events.

            case MENU_EVENT: // Disregard menu events.
                break;

            default:
                ErrorExit("Unknown event type");
                break;
        }
    }
    char* s;
#elif __linux__
    XEvent ev;
    char* s;
    InputData inputData;

    std::vector<std::string> key_name = { "left button", "middle button", "right button", };

    XNextEvent(dpy, &ev);

    if (ev.type == KeyPress || ev.type == KeyRelease)
    {
        s = XKeysymToString(XkbKeycodeToKeysym(dpy, ev.xkey.keycode, 0, ev.xkey.state & ShiftMask ? 1 : 0));
        if (s)
        {
            if (ev.type == KeyPress)
            {
                printf("Key Press: key pressed, %s\n", s);
                inputData.keyDown = true;
            }
            else if (ev.type == KeyRelease)
            {
                printf("Key event: key released, %s\n", s);
                inputData.keyDown = false;
            }

            std::string keyString(s);
            inputData.key = new char[keyString.size()];
            strcpy(inputData.key, keyString.c_str());

#ifdef EVENT_SYNC_KEYBOARD_I
            es->call("input_keyboard", 0);
#elif defined(EVENT_ASYNC_KEYBOARD_I)
            es->callAsync("input_keyboard", 0);
#elif defined(EVENT_MULTI_KEYBOARD_I)
            kbt.push_back(std::thread(callAsyncWrapper, "input_keyboard"));
#endif

#ifdef DIRECT_KEYBOARD_I
            std::sort(Input::inputDescriptors.begin(), Input::inputDescriptors.end(), keyboard_priority_queue());
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
#endif
        }
    }

    else if (ev.type == MotionNotify || ev.type == ButtonPress || ev.type == ButtonRelease)
    {
        if (ev.type == MotionNotify)
        {
            std::cout << "Mouse moved: (" << ev.xmotion.x_root << ", " << ev.xmotion.y_root << ")" << std::endl;
            inputData.mouseMoved = true;
        }
        else if (ev.type == ButtonPress)
        {
            std::cout << "Mouse button pressed: " << key_name[ev.xbutton.button - 1] << ": (" << ev.xmotion.x_root << ", " << ev.xmotion.y_root << ")" << std::endl;
            if (ev.xbutton.button - 1 == 0)
            {
                inputData.leftMouseButtonPressed = true;
            }
            else if (ev.xbutton.button - 1 == 1)
            {
                inputData.mouseButtonPressed = true;
            }
            else if (ev.xbutton.button - 1 == 2)
            {
                inputData.rightMouseButtonPressed = true;
            }
        }
        else if (ev.type == ButtonRelease)
        {
            std::cout << "Mouse button released: " << key_name[ev.xbutton.button - 1] << ": (" << ev.xmotion.x_root << ", " << ev.xmotion.y_root << ")" << std::endl;
            if (ev.xbutton.button - 1 == 0)
            {
                inputData.leftMouseButtonReleased = true;
            }
            else if (ev.xbutton.button - 1 == 1)
            {
                inputData.mouseButtonReleased = true;
            }
            else if (ev.xbutton.button - 1 == 2)
            {
                inputData.rightMouseButtonReleased = true;
            }
        }

#ifdef EVENT_SYNC_MOUSE_I
        es->call("input_mouse", 0);
#elif defined(EVENT_ASYNC_MOUSE_I)
        es->callAsync("input_mouse", 0);
#elif defined(EVENT_MULTI_MOUSE_I)
        kbt.push_back(std::thread(callAsyncWrapper, "input_mouse"));
#endif

#ifdef DIRECT_MOUSE_I
        std::sort(Input::inputDescriptors.begin(), Input::inputDescriptors.end(), mouse_priority_queue());
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
#endif
    }

    else if (ev.type == Expose)
    {
        while (XCheckTypedEvent(dpy, Expose, &ev))
        {
            continue;
        }
    }

    else if (ev.type == ConfigureNotify)
    {

    }
#endif
}

// Define what processes the plug-in runs once it is loaded into a program by the plugin manager.
void InputImpl::initialize(size_t identifier)
{
    std::cout << "InputImpl::initialize" << std::endl;

#ifdef _WIN32
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
#elif __linux__
    // Enable raw mode for terminal input.
    enableRawMode();
#endif
    // Read local config file to determine sync status.
    const char* exeDir = reinterpret_cast<const char*>(identifier);
#ifdef _WIN32
    std::string configPath = std::string(exeDir) + "..\\plugins\\input\\start_option.cfg";
#elif __linux__
    std::string configPath = std::string(exeDir) + "../plugins/input/start_option.cfg";
#endif
    parseStartConfigFile(configPath);

    // Create any necessary events and/or get a PluginManager instance.
#if defined(EVENT_SYNC_KEYBOARD_I) || defined(EVENT_ASYNC_KEYBOARD_I) || defined(EVENT_MULTI_KEYBOARD_I) \
|| defined(EVENT_SYNC_MOUSE_I) || defined(EVENT_ASYNC_MOUSE_I) || defined(EVENT_MULTI_MOUSE_I) \
|| defined(EVENT_RUNNER_I)
    es = EventStream<double>::Instance(identifier);
#if defined(EVENT_SYNC_KEYBOARD_I) || defined(EVENT_ASYNC_KEYBOARD_I) || defined(EVENT_MULTI_KEYBOARD_I)
    es->create("input_keyboard");
#endif
#if defined(EVENT_SYNC_MOUSE_I) || defined(EVENT_ASYNC_MOUSE_I) || defined(EVENT_MULTI_MOUSE_I)
    es->create("input_mouse");
#endif
#endif
#ifdef DIRECT_RUNNER_I
    pm = PluginManager::Instance(identifier);
#endif
}

// Define what processes the plug-in runs immediately before it is unloaded the plugin manager. Join all separate threads made for
// keyboard and mouse inputs if EVENT_MULTI_KEYBOARD_I and/or EVENT_MULTI_MOUSE_I are defined, destroy all input_keyboard and
// input_mouse events, clear all descriptors, and unload Runner if it was loaded at start-time.
void InputImpl::release()
{
    std::cout << "InputImpl::release" << std::endl;
#if defined(EVENT_SYNC_KEYBOARD_I) || defined(EVENT_ASYNC_KEYBOARD_I) || defined(EVENT_MULTI_KEYBOARD_I)
    std::cout << "CASE EVENT_KEYBOARD_I" << std::endl;
#ifdef EVENT_MULTI_KEYBOARD_I
    for (int i = 0; i < kbt.size(); ++i)
    {
        if (kbt[i].joinable())
        {
            kbt[i].join();
        }
    }
#endif
    es->destroy("input_keyboard");
#endif
#if defined(EVENT_SYNC_MOUSE_I) || defined(EVENT_ASYNC_MOUSE_I) || defined(EVENT_MULTI_MOUSE_I)
    std::cout << "CASE EVENT_MOUSE_I" << std::endl;
#ifdef EVENT_MULTI_MOUSE_I
    for (int i = 0; i < mt.size(); ++i)
    {
        if (mt[i].joinable())
        {
            mt[i].join();
        }
    }
#endif
    es->destroy("input_mouse");
#endif
#if defined(DIRECT_KEYBOARD_I) || defined(DIRECT_MOUSE_I)
    std::cout << "CASE DIRECT_I" << std::endl;
    inputDescriptors.clear();
#endif
    
    if (!isAsync)
    {
#ifdef DIRECT_RUNNER_I
        pm->Unload("runner");
        pm->requestDelete();
#endif
    }

#if defined(EVENT_SYNC_KEYBOARD_I) || defined(EVENT_ASYNC_KEYBOARD_I) || defined(EVENT_MULTI_KEYBOARD_I) \
|| defined(EVENT_SYNC_MOUSE_I) || defined(EVENT_ASYNC_MOUSE_I) || defined(EVENT_MULTI_MOUSE_I) \
|| defined(EVENT_RUNNER_I)
    es->requestDelete();
    es = nullptr;
#endif
#ifdef _WIN32
    SetConsoleMode(hStdin, fdwSaveOldMode);
#endif
}

// Start the input plugin, possibly in a separate thread if specified to run asynchronously by the config file.
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

// Sets a parameter to break the update loop if asynchronous, or to pop off/unsubscribe from runner update loop if synchronous.
// Also join any keyboard and/or mouse event threads if they exist.
void InputImpl::stop()
{
    std::cout << "InputImpl::stop" << std::endl;

#ifdef EVENT_MULTI_KEYBOARD_I
    for (int i = 0; i < kbt.size(); ++i)
    {
        kbt[i].join();
    }
#endif
#ifdef EVENT_MULTI_MOUSE_I
    for (int i = 0; i < mt.size(); ++i)
    {
        mt[i].join();
    }
#endif

    if (isAsync)
    {
        breakLoop = true;
        if (thread.joinable())
        {
            thread.join();
        }
    }

    else
    {
#ifdef EVENT_RUNNER_I
        es->unsubscribe("runner", g_ids);
#endif
#ifdef DIRECT_RUNNER_I
        runner->pop(rDesc);
#endif
    }

#ifdef __linux__
    std::cout << "We made it here" << std::endl;
    XUngrabKeyboard(dpy, CurrentTime);
#endif
}

// Helper function for start. Lets us more conveniently start the plugin asynchronously. Either call internalIteration
// inside a while loop that will eventually run on its own thread, or register internalIteration to a Runner.
void InputImpl::start2()
{
#ifdef __linux__
    dpy = XOpenDisplay(NULL);

    if (dpy == NULL)
    {
        exit(1);
    }

    XAllowEvents(dpy, AsyncBoth, CurrentTime);
    XGrabKeyboard(dpy, DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XGrabPointer(dpy, DefaultRootWindow(dpy), 1, PointerMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync, GrabModeAsync, None, None, CurrentTime);

#endif

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
#ifdef _WIN32
        g_ids = es->subscribe("runner", &(::internalIteration));
#elif __linux__
        // Until wrapper finishes executing, the runner thread won't be joined when runner's stop() function
        // is called, hence why wrapper is still being registered after runner's stop() is executed.
        g_ids = es->subscribe("runner", &(::internalIteration));
#endif
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

// Define functions with C symbols (create/destroy Input instance).
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
