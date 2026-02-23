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

//should be FOutputDevice**, but that is not defined yet
extern void**           GError;


// ! Macro for using the location property of AActor, FTPOV, maybe some others?

#if defined(SDK_TARGET_LE1) || defined(SDK_TARGET_LE2)

#define LOCATION Location

#endif

#if defined(SDK_TARGET_LE3)

#define LOCATION location

#endif

// Enums

// Enum Core.Object.EObjectFlags
enum EObjectFlags : uint64_t
{
    InSingularFunc = 0x0000000000000002, // In a singular function.
    StateChanged = 0x0000000000000004,   // Object did a state change.
    DebugPostLoad = 0x0000000000000008,   // For debugging Serialize calls.
    DebugSerialize = 0x0000000000000010,   // For debugging Serialize calls.
    DebugFinishDestroy = 0x0000000000000020,   // For debugging FinishDestroy calls.
    EdSelected = 0x0000000000000040,
    ZombieComponent = 0x0000000000000080,
    Protected = 0x0000000000000100, // Property can only be accessed by owning class or subclasses
    ClassDefaultObject = 0x0000000000000200, // this object is its class's default object
    ArchetypeObject = 0x0000000000000400, // this object is a template for another object - treat like a class default object
    ForceTagExp = 0x0000000000000800, //Force this object into the export table when saving
    TokenStreamAssembled = 0x0000000000001000,
    MisAlignedObject = 0x0000000000002000, // Object has desynced from c++ class (native classes, editor only)
    RootSet = 0x0000000000004000, // This object should not be garbage collected
    BeginDestroyed = 0x0000000000008000, // BeginDestroy has been called
    FinishDestroyed = 0x0000000000010000, // FinishDestroy has been called
    DebugBeginDestroyed = 0x0000000000020000, // If object is considered part of the root set (?)
    MarkedByCooker = 0x0000000000040000,
    LocalizedResource = 0x0000000000080000, // Resource object is localized
    InitializedProps = 0x0000000000100000, // have properties been initialized?
    PendingFieldPatches = 0x0000000000200000, // ScriptPatch system (not used in ME)
    IsCrossLevelReferenced = 0x0000000000400000, // This object has been pointed to by a cross-level reference, and therefore requires additional cleanup upon deletion

    Saved = 0x0000000080000000,
    Transactional = 0x0000000100000000,   // Object is transactional.
    Unreachable = 0x0000000200000000,   // Object is not reachable on the object graph.            
    Public = 0x0000000400000000, // Object is visible outside its package.
    TagImp = 0x0000000800000000, // Temporary import tag in load/save.
    TagExp = 0x0000001000000000, // Temporary export tag in load/save.
    Obsolete = 0x0000002000000000,   // Object marked as obsolete and should be replaced.
    TagGarbage = 0x0000004000000000, // Check during garbage collection.
    DisregardForGC = 0x0000008000000000,// Object is considered static // REPLACED Final = 0x0000008000000000,	// Object is not visible outside of class.
    PerObjectLocalized = 0x0000010000000000, // Object is localized by instance name, not by class.
    NeedLoad = 0x0000020000000000,   // During loading, indicates object needs loading.
    AsyncLoading = 0x0000040000000000, // Object is being async loaded
    NeedPostLoadSubobjects = 0x0000080000000000, // During load, Subobjects also need instanced
    Suppress = 0x0000100000000000,   //warning: Mirrored in UnName.h. Suppressed log name.
    InEndState = 0x0000200000000000,   // Within an EndState call.
    Transient = 0x0000400000000000,  // Don't save object.
    Cooked = 0x0000800000000000, // Content was cooked
    LoadForClient = 0x0001000000000000,  // In-file load for client.
    LoadForServer = 0x0002000000000000,  // In-file load for client.
    LoadForEdit = 0x0004000000000000,    // In-file load for client.
    Standalone = 0x0008000000000000,   // Keep object around for editing even if unreferenced.
    NotForClient = 0x0010000000000000,   // Don't load this object for the game client.
    NotForServer = 0x0020000000000000,   // Don't load this object for the game server.
    NotForEdit = 0x0040000000000000, // Don't load this object for the editor.
    // There is nothing in this slot.
    NeedPostLoad = 0x0100000000000000,   // Object needs to be postloaded.
    HasStack = 0x0200000000000000,   // Has execution stack.
    Native = 0x0400000000000000,   // Native (UClass only).
    Marked = 0x0800000000000000,   // Marked (for debugging).
    ErrorShutdown = 0x1000000000000000, // ShutdownAfterError called.
    PendingKill = 0x2000000000000000 // Object is pending destruction
};

// Define the bitwise OR operator for Flags
inline EObjectFlags operator|(EObjectFlags lhs, EObjectFlags rhs) {
    return static_cast<EObjectFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

// Define the bitwise OR assignment operator for Flags
inline EObjectFlags& operator|=(EObjectFlags& lhs, EObjectFlags rhs) {
    lhs = static_cast<EObjectFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
    return lhs;
}

// Define the bitwise AND operator for Flags
inline EObjectFlags operator&(EObjectFlags lhs, EObjectFlags rhs) {
    return static_cast<EObjectFlags>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

// Define the bitwise AND assignment operator for Flags
inline EObjectFlags& operator&=(EObjectFlags& lhs, EObjectFlags rhs) {
    lhs = static_cast<EObjectFlags>(static_cast<int>(lhs) & static_cast<int>(rhs));
    return lhs;
}

// If you need others define them here...



// Address macros
#define LEASI_RVA(RVA) ::LESDK::Address::FromOffset(RVA)
