/**
 * @file        LESDK/Headers.hpp
 * @brief       This file is the primary public header for the SDKs.
                It checks some core definitions and includes generated headers as required.
 */

#pragma once


// The SDK_TARGET_LE? is defined in the versioned project configuration
// so that shared SDK sources may differentiate between what their actual
// game is. So, first thing, we check that there's only one such definition.

#if defined(SDK_TARGET_LE1)
    #define SDK_TARGET 1
    #define SDK_TARGET_NAME_A    "LE1"
    #define SDK_TARGET_NAME_W   L"LE1"
    #if defined(SDK_TARGET_LE2) || defined(SDK_TARGET_LE3)
        #error Conflicting SDK_TARGET definitions.
    #endif
#endif

#if defined(SDK_TARGET_LE2)
    #define SDK_TARGET 2
    #define SDK_TARGET_NAME_A    "LE2"
    #define SDK_TARGET_NAME_W   L"LE2"
    #if defined(SDK_TARGET_LE1) || defined(SDK_TARGET_LE3)
        #error Conflicting SDK_TARGET definitions.
    #endif
#endif

#if defined(SDK_TARGET_LE3)
    #define SDK_TARGET 3
    #define SDK_TARGET_NAME_A    "LE3"
    #define SDK_TARGET_NAME_W   L"LE3"
    #if defined(SDK_TARGET_LE1) || defined(SDK_TARGET_LE2)
        #error Conflicting SDK_TARGET definitions.
    #endif
#endif

#if !defined(SDK_TARGET_LE1) && !defined(SDK_TARGET_LE2) && !defined(SDK_TARGET_LE3)
    #error No SDK_TARGET definition set.
#endif


// Include common type implementations like array, names, etc.

#include "LESDK/Common/Common.hpp"


// With all the preparations done, conditionally include generated SDK headers.
// If rebuilding the SDKs, the included files should *usually* be left untouched,
// but do check the original generated <SdkHeaders.h> for each target just in case.

#if defined(SDK_TARGET_LE1)
    #include "LESDK/Includes.LE1.hpp"
#endif

#if defined(SDK_TARGET_LE2)
    #include "LESDK/Includes.LE2.hpp"
#endif

#if defined(SDK_TARGET_LE3)
    #include "LESDK/Includes.LE3.hpp"
#endif
