/**
 * @file        LESDK/Common/Core.hpp
 * @brief       This file includes core scalar typedefs, forward declarations, and utility macros.
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

#if !defined(LESDK_WARN_OVERRIDE) || LESDK_WARN_OVERRIDE == 0

namespace Detail { void DefaultWarn(char const* Message); }
#define LESDK_WARN(msg) ::Detail::DefaultWarn(msg)

#endif


// ! Basic type definitions.
// ========================================

typedef signed char             SBYTE;
typedef signed short            SWORD;
typedef signed int              SDWORD;
typedef unsigned long long      QWORD;
typedef signed long long        SQWORD;

typedef size_t                  SIZE_T;


static_assert(sizeof(BYTE) == 1 && sizeof(SBYTE) == 1);
static_assert(sizeof(WORD) == 2 && sizeof(SWORD) == 2);
static_assert(sizeof(DWORD) == 4 && sizeof(SDWORD) == 4);
static_assert(sizeof(QWORD) == 8 && sizeof(SQWORD) == 8);


typedef unsigned int UBOOL;


// ! Implementation stuff for containers.
// ========================================

#define CONTAINER_TYPEDEFS(Elem, Size, Diff)                        \
    public:                                                         \
        using value_type = Elem;                                    \
        using size_type = Size;                                     \
        using difference_type = Diff;                               \
        using reference = value_type&;                              \
        using const_reference = value_type const&;                  \
        using pointer = value_type*;                                \
        using const_pointer = value_type const*;                    \
        using iterator = value_type*;                               \
        using const_iterator = value_type const*;


// ! Forward declarations.
// ========================================

struct FFrame;
struct FStateFrame;

class UObject;
class UField;
class UEnum;
class UConst;
class UStruct;
class UFunction;
class UState;
class UClass;
class UProperty;
class UEngine;
class UWorld;
class USystem;


// ! Unreal Engine's global allocator.
// ========================================

#if defined(SDK_TARGET_LE1)

class FMallocLike {
public:

    virtual UBOOL Exec(WCHAR const* Cmd, void* Ar) = 0;                         // vtable offset = 0x00
    virtual void* Malloc(DWORD Count, DWORD Alignment) = 0;                     // vtable offset = 0x08
    virtual void* Realloc(void* Orig, DWORD Count, DWORD Alignment) = 0;        // vtable offset = 0x10
    virtual UINT Unknown0x18() { return 0; };                                   // vtable offset = 0x18
    virtual void Free(void* Orig) = 0;                                          // vtable offset = 0x20
    virtual UINT Unknown0x28() { return 0; }                                    // vtable offset = 0x28
    virtual void Unknown0x30() { }                                              // vtable offset = 0x30
    virtual UINT Unknown0x38() { return 0; }                                    // vtable offset = 0x38
    virtual void Unknown0x40() { }                                              // vtable offset = 0x40
    virtual void Unknown0x48() { }                                              // vtable offset = 0x48
    virtual void Unknown0x50() { }                                              // vtable offset = 0x50
    virtual void Unknown0x58() { }                                              // vtable offset = 0x58
    virtual void Tick(FLOAT Delta) = 0;                                         // vtable offset = 0x60
    virtual UBOOL IsThreadSafe() = 0;                                           // vtable offset = 0x68
    virtual void GetAllocationInfo(SIZE_T* Virt, SIZE_T* Phys) = 0;             // vtable offset = 0x70
    virtual void GetPS3AllocationInfo(void* A, void* B, void* C);               // vtable offset = 0x78
    virtual void CheckMemoryFragmentation(void* Ar);                            // vtable offset = 0x80
    virtual UBOOL TrimMemory(SIZE_T A, UBOOL B);                                // vtable offset = 0x88
    virtual void Unknown0x90() { }                                              // vtable offset = 0x90
    virtual void Unknown0x98() { }                                              // vtable offset = 0x98
    virtual UINT Unknown0xA0() { return (UINT)-1; }                             // vtable offset = 0xA0
    virtual UINT BioHeapCheck();                                                // vtable offset = 0xA8

};

#endif

#if defined(SDK_TARGET_LE2) || defined(SDK_TARGET_LE3)

class FMallocLike {
public:

    virtual UBOOL Exec(WCHAR const* Cmd, void* Ar) = 0;                         // vtable offset = 0x00
    virtual void* Malloc(DWORD Count, DWORD Alignment) = 0;                     // vtable offset = 0x08
    virtual void* Realloc(void* Orig, DWORD Count, DWORD Alignment) = 0;        // vtable offset = 0x10
    virtual void Free(void* Orig) = 0;                                          // vtable offset = 0x18
    virtual UINT Unknown0x20() { return 0; };                                   // vtable offset = 0x20
    virtual void Unknown0x28() { }                                              // vtable offset = 0x28
    virtual UINT Unknown0x30() { return 0; };                                   // vtable offset = 0x30
    virtual void Unknown0x38() { }                                              // vtable offset = 0x38
    virtual void Unknown0x40() { }                                              // vtable offset = 0x40
    virtual void Unknown0x48() { }                                              // vtable offset = 0x48
    virtual void Unknown0x50() { }                                              // vtable offset = 0x50
    virtual void Tick(FLOAT Delta) = 0;                                         // vtable offset = 0x58
    virtual UBOOL IsThreadSafe() = 0;                                           // vtable offset = 0x60
    virtual void GetAllocationInfo(SIZE_T* Virt, SIZE_T* Phys) = 0;             // vtable offset = 0x68
    virtual void GetPS3AllocationInfo(void* A, void* B, void* C);               // vtable offset = 0x70
    virtual void CheckMemoryFragmentation(void* Ar);                            // vtable offset = 0x78
    virtual UINT TrimMemory(SIZE_T A, UBOOL B);                                 // vtable offset = 0x80
    virtual void Unknown0x88() { }                                              // vtable offset = 0x88
    virtual void Unknown0x90() { }                                              // vtable offset = 0x90
    virtual UINT Unknown0x98() { return (UINT)-1; }                             // vtable offset = 0x98
    virtual UINT BioHeapCheck();                                                // vtable offset = 0xA0

};

#endif

// No idea what those "Unknown" methods are...
// Maybe some kind of a security feature that blows up vtable for giggles?


static constexpr unsigned int UN_DEFAULT_ALIGNMENT = sizeof(void*) * 2;
static_assert(UN_DEFAULT_ALIGNMENT == 16);


void* sdkMalloc(DWORD Count, DWORD Alignment = UN_DEFAULT_ALIGNMENT);
void* sdkRealloc(void* Orig, DWORD Count, DWORD Alignment = UN_DEFAULT_ALIGNMENT);
void  sdkFree(void* Orig);

template<typename T>
T* sdkMallocTyped(DWORD const Num, DWORD const Alignment = UN_DEFAULT_ALIGNMENT) {
    return reinterpret_cast<T*>(sdkMalloc(static_cast<DWORD>(Num * sizeof(T)), Alignment));
}

template<typename T>
T* sdkReallocTyped(T* const Orig, DWORD const Num, DWORD const Alignment = UN_DEFAULT_ALIGNMENT) {
    return reinterpret_cast<T*>(sdkRealloc(Orig, static_cast<DWORD>(Num * sizeof(T)), Alignment));
}

template<typename T>
void sdkFreeTyped(T* const Orig) {
    sdkFree(Orig);
}


// ! General-purpose CRC32 hash.
// ========================================

namespace LESDK {
    DWORD MemCrc32(void* InData, int Length, DWORD Crc = 0);
    DWORD MemCrc32(void const* InData, int Length, DWORD Crc = 0);
    DWORD MemCrc32(std::span<BYTE> InData, DWORD Crc = 0);
    DWORD MemCrc32(std::span<BYTE const> InData, DWORD Crc = 0);
}


// ! Non-member global variables.
// ========================================

extern FMallocLike**    GMalloc;

extern UEngine**        GEngine;
extern USystem**        GSys;
extern UWorld**         GWorld;
