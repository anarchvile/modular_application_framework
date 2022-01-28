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

# contents

The core software components of this project are as follows:
- plugin manager
    a singleton class that's responsible for actually loading/unloading 
    dynamic libraries within the program.
- runner
    a plug-in that allows other, external plug-ins to register member 
    functions to a global, constantly-ticking update loop (which is also 
    implemented in the runner).
- example plugins
    named hello/goodbye world. These are simple plug-in examples that
    serve to outline how plug-ins can be structured, and how they interact
    with the rest of the components in this project.
- concurrent loading of plugins example
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

This example application loads two separate plug-ins (plus a runner), helloWorld and 
goodbyeWorld, for use. helloWorld has a three-second-delay upon its initialization and 
release, to simulate complex computations that need to be taken care of during these two
processes (goodbyeWorld does not include this feature). An update loop is then ran,
updating the functions registered to the runner by each plug-in. In this case,
helloWorld and goodbyeWorld each print out some statements related to their preupdate,
update, and postupdate methods. After 10 iterations helloWorld will unsubscribe itself
from the loop, leaving only goodbyeWorld to continue updating for another 10 ticks.
After this 20 tick mark has been hit, goodbyeWorld will also unregister itself, and
all plug-ins will be released/unloaded from the program.