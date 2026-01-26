/**
 * @file        LESDK/Common/Misc.hpp
 * @brief       This file implements some miscellaneous stuff that can't go anywhere else.
 */

#pragma once

#include "LESDK/Common/Core.hpp"
#include "LESDK/Common/SFXName.hpp"


#pragma pack(push, 4)

struct FScriptDelegate {
    UObject* Object;
    SFXName FunctionName;
};
static_assert(sizeof(FScriptDelegate) == 0x10);

struct FRepRecord final {
    UProperty* Prop;
    int Index;
};
static_assert(sizeof(FRepRecord) == 0x0C);

// ! Common native structs
struct FScriptInterface
{
    UObject* Object;
    void* Interface;
};

struct FObjectResource
{
    SFXName ObjectName;
    int	OuterIndex;
};

struct FObjectImport : public FObjectResource
{
    SFXName ClassPackage;
    SFXName ClassName;
    UObject* Object;
    class ULinkerLoad* SourceLinker;
    int SourceIndex;
};

// From v1 LE2 SDK
struct FObjectExport : public FObjectResource
{
    int ClassIndex;
    int SuperclassIndex;
    int ArchetypeIndex;
    long long ObjectFlags;
    int SerialSize;
    int DataOffset;

    int unk1;
    int unk2;

    class UObject* Object; // The loaded object
    int next;

    int ExportFlags;
    TArray<INT> GenerationsObjectCount;
    //FGuid PackageGuid;
    unsigned char PackageGuid[0x10]; // FGuid won't compile here!!!!
    int PackageFlags;
};

#pragma pack(pop)


// Flags used to control loading
enum ELoadFlags
{
    LOAD_None = 0x00000000,	// No flags
    LOAD_SeekFree = 0x00000001,	// Load the file using seekfree
    LOAD_NoWarn = 0x00000002,	// Don't warn if there's a problem
    LOAD_Throw = 0x00000008,	// Throw an exception if there's a problem
    LOAD_Verify = 0x00000010,	// Test if it exists; don't actually load the file
    LOAD_AllowDll = 0x00000020, // ???
    LOAD_DisallowFiles = 0x00000040,	// ???
    LOAD_NoVerify = 0x00000080,   // Don't verify imports exist
    LOAD_Quiet = 0x00002000,   // Don't log anything
    LOAD_FindIfFail = 0x00004000,	// If object fails to load, try to find in memory instead...?
    LOAD_MemoryReader = 0x00008000,	// Load into memory, and then load from memory instead of piece-by-piece from disk
    LOAD_RemappedPackage = 0x00010000,	// Unused in ME
    LOAD_NoRedirects = 0x00020000,	// Ignore object redirectors
};


namespace LESDK {

    void InitializeConsole();
    void TerminateConsole();

    // These functions below are templated over UObject to ensure late binding to the UObject type in question.
    // In other words, we don't need to have the concrete UObject structure by this point.

    template<class UObjectLike, bool WithRAII>
    void AppendObjectName(UObjectLike const* const InObject, FStringBase<WithRAII>& OutString,
        SFXName::FormatMode const Mode = SFXName::k_formatInstanced)
    {
        (InObject->UObject::Name).AppendToString(OutString, Mode);
    }

    template<class UObjectLike, bool WithRAII>
    void AppendObjectNameCPP(UObjectLike const* const InObject, FStringBase<WithRAII>& OutString) {
        if (InObject->IsA(UClass::StaticClass())) {
            UClass const* pClass = reinterpret_cast<UClass const*>(InObject);
            while (pClass != nullptr) {
                if (pClass == AActor::StaticClass()) {
                    OutString.Append(L'A');
                } else if (pClass == UObject::StaticClass()) {
                    OutString.Append(L'U');
                }
                pClass = reinterpret_cast<UClass const*>(pClass->SuperField);
            }
        } else {
            OutString.Append(L'F');
        }
        AppendObjectName(InObject, OutString, SFXName::k_formatBasic);
    }

    template<class UObjectLike, bool WithRAII>
    void AppendObjectNameFull(UObjectLike const* const InObject, FStringBase<WithRAII>& OutString,
        SFXName::FormatMode const Mode = SFXName::k_formatInstanced)
    {
        if (InObject->Class != nullptr) {
            AppendObjectName(InObject->Class, OutString, Mode);
            OutString.Append(L" ");

            if (InObject->Outer != nullptr) {
                if (InObject->Outer->Outer != nullptr) {
                    if (InObject->Outer->Outer->Outer != nullptr) {
                        //degenerate case, delegate to a recursive function
                        AppendFullPathRecursive(InObject->Outer->Outer->Outer, OutString, Mode);
                        OutString.Append(L".");
                    }
                    AppendObjectName(InObject->Outer->Outer, OutString, Mode);
                    OutString.Append(L".");
                }

                AppendObjectName(InObject->Outer, OutString, Mode);
                OutString.Append(L".");
            }

            AppendObjectName(InObject, OutString, Mode);
            return;
        }

        OutString.Append(L"(null)");
    }

    template<class UObjectLike, bool WithRAII>
    void AppendObjectFullPath(UObjectLike const* const InObject, FStringBase<WithRAII>& OutString,
        SFXName::FormatMode const Mode = SFXName::k_formatInstanced)
    {
        if (InObject != nullptr) {

            if (InObject->Outer != nullptr) {
                if (InObject->Outer->Outer != nullptr) {
                    if (InObject->Outer->Outer->Outer != nullptr) {
                        //degenerate case, delegate to a recursive function
                        AppendFullPathRecursive(InObject->Outer->Outer->Outer, OutString, Mode);
                        OutString.Append(L".");
                    }
                    AppendObjectName(InObject->Outer->Outer, OutString, Mode);
                    OutString.Append(L".");
                }
                AppendObjectName(InObject->Outer, OutString, Mode);
                OutString.Append(L".");
            }

            AppendObjectName(InObject, OutString, Mode);
            return;
        }

        OutString.Append(L"(null)");
    }

    template<class UObjectLike, bool WithRAII>
    void AppendFullPathRecursive(UObjectLike const* const InObject, FStringBase<WithRAII>& OutString,
        SFXName::FormatMode const Mode = SFXName::k_formatInstanced)
    {
            if (InObject->Outer != nullptr) {
                AppendFullPathRecursive(InObject->Outer, OutString, Mode);
                OutString.Append(L".");
            }

            AppendObjectName(InObject, OutString, Mode);
    }

}
