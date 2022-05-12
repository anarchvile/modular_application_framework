// The following ifdef block is the standard way of creating macros which make exporting 
// from a plugin (.dll or .so) simpler. All files within this plugin are compiled 
// with the COMMON_EXPORTS symbol defined on the command line (since the COMMON_EXPORTS 
// macro is defined in the preprocessor settings for this project only). This symbol should 
// not be defined on any project that uses this plugin, so that any other project whose 
// source files include this file see COMMON_EXPORTS functions as being imported from the
// plugin, whereas this plugin sees symbols defined with this macro as being exported.

#pragma once
#ifdef _WIN32 // Microsoft compiler
    #ifdef COMMON_EXPORTS
        #define COMMON_API __declspec(dllexport)
    #else
        #define COMMON_API
    #endif
#elif __linux__  // GNU compiler
    #define COMMON_API
#endif
