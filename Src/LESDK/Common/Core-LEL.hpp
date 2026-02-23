/**
 * @file        LESDK/Common/Core-LEL.hpp
 * @brief       This file includes SDK-specific things for Launcher ASIs. This file is standalone
                to avoid polluting the actual game files with ifdefs.
 */

#pragma once

// ! Basic assertion macros (can be overriden by consumers).
// ========================================

#if !defined(LESDK_CHECK_OVERRIDE) || LESDK_CHECK_OVERRIDE == 0

#ifndef NDEBUG
    // #include <cassert>
#define LESDK_CHECK(cond, msg) assert((cond) && msg)
#else
#define LESDK_CHECK(cond, msg) (void)(cond)
#endif

#endif

// Additional macros
#define LEASI_RVA(RVA) ::LESDK::Address::FromOffset(RVA)
