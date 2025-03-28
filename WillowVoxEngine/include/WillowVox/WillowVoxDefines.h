#pragma once

#ifdef PLATFORM_WINDOWS
    #ifdef WILLOWVOX_EXPORT
        #define WILLOWVOX_API __declspec(dllexport)
    #else
        #define WILLOWVOX_API __declspec(dllimport)
    #endif
#else
    #define WILLOWVOX_API __attribute__((visibility("default")))
#endif