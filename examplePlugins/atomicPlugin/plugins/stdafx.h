#ifndef STDAFX_H
#define STDAFX_H

#include "targetver.h"

#ifdef _WIN32 // Microsoft compiler
    #define WIN32_LEAN_AND_MEAN // exclude rarely-used stuff from Windows headers
    #include <windows.h>
#elif __linux__ // GNU compiler
    #include <dlfcn.h>
#else
    #error define your compiler
#endif

#endif // STDAFX_H
