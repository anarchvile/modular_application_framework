# Modular plug-in based framework

Monolithic applications tend to not scale well. They complicate
the development process, especially when a team of developers work
in the same code base, and require recompilation of the entire
project for any code change or bug fix.

This project provides an alternative to the problem in the form
of a plug-in based system that allows a software application to
be assembled from individual and reusable components (dynamic
libraries).
All modules are independently written and compiled, which
simplifies the development process.
They rely on a stable ABI interface which minimizes the
inconvenience of supporting multiple operating systems and
multiple/changing API/SDK versions.
Functionality can be extended at runtime by loading the required
plug-ins.
Python bindings (via pybind11) allow for flexible interop between
Python and C++; plugins can be written in either language whilst
maintaining the ability to communicate with one another.

# contents

The core software components of this project are as follows:
- plugin manager
    a singleton class that's responsible for actually loading/unloading 
    dynamic libraries within the program.
- event system
    a thread-safe system for performing tasks when something occurs, i.e. when different Events occur (e.g. a key on a keyboard
    is pressed, an internal counter reaches some pre-specified value, etc.).
    The following terminology is used within this application:
    - EventHandler: A holder for an actual method that should be called when a corresponding notification is raised
    - Event: A holder for a number of handlers. An Event can be called for raising a notification, and in turn execute its handlers.
    - EventStream: A holder for a number of Events. EventStreams can be used to create/destroy/call on Events, subscribe/unsubscribe
      handlers to specific Events using the latter's name, etc.
- runner
    a plug-in that allows other, external plug-ins to register member 
    functions/events to a global, constantly-ticking update loop (which is also 
    implemented in the runner).
- input
    a plug-in that registers keyboard/mouse inputs. Can be utilized to bind
    specific functions/events to specific inputs, e.g. to call a "print" function
    every time the left mouse button is pressed.
- example plugins
    -helloWorld and goodbyeWorld
        These are simple plug-in examples that serve to outline 
        how C++ plug-ins can be structured, and how they interact
        with the rest of the components in this project.
    -concurrentLoading
        Shows how C++ plugins can be loaded within another plugin, and highlights
        some of the thread-safe loading/unloading features of the application.
    -atomicPlugin
        Another simple C++ plugin that illustrates how 
    -pyExPlg1 and pyExPlg2
        Two example plugins written entirely in Python that serve to
        outline how such plugins can be structured, how they can access
        the functionality of other plugins written in C++ via python bindings, etc.
- main app
    the main application/executable is where the global initialization,
    release, and update functions for all plug-ins live. If a plug-in 
    is to be used, it needs to be loaded/updated either directly inside
    the main app, or through another plug-in that has already been prepared
    for use inside main.

# compilation process

## Windows
Compile each solution file in the main and plug-in directories.

## Linux
Compile each makefile in the main and plug-in directories.

# running the application

Navigate to directory _build/${pltform}/${config}/bin and run the main executable.
It will open a new terminal where results from the executed code will be displayed.
