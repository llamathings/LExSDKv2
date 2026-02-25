/**
 * @file        LESDK/Common/FArchive.hpp
 * @brief       This file contains a partial implementation of FArchive class and subclasses.
 *
 * @author      Mgamerz
 */

#pragma once

#include "LESDK/Common/Core.hpp"
#include "LESDK/Common/TArray.hpp"

#pragma pack(push, 4)
#if defined(SDK_TARGET_LE1)
    #define AR_VER 684
    #define AR_LICENSEE_VER 171
#elif defined(SDK_TARGET_LE2)
    #define AR_VER 684
    #define AR_LICENSEE_VER 168
#elif defined(SDK_TARGET_LE3)
    #define AR_VER 685
    #define AR_LICENSEE_VER 205
#endif


// Foward declaration
struct FArchive;
class ULinker;

// This is cause inheritance without defined virtual methods doesn't work.
struct FArchive_VTable {
    void (*Destructor)(FArchive* Ar);
    void (*Serialize)(FArchive* Ar, void* V, INT Length);
    void SerializeBits(FArchive* Ar, void* V, INT LengthBits);
    void (*SerializeInt)(FArchive* Ar, DWORD& Value, DWORD Max);
    void Preload(FArchive* Ar, UObject* Object);
    void CountBytes(FArchive* Ar, SIZE_T InNum, SIZE_T InMax);
    FArchive& SerializeObject(FArchive* Ar, UObject*& Res);
    FArchive& SerializeName(FArchive* Ar, struct SFXName& N);

    FString GetArchiveName(FArchive* Ar) const;
    ULinker* GetLinker(FArchive* Ar);
    INT Tell(FArchive* Ar);
    INT TotalSize(FArchive* Ar);
    UINT AtEnd(FArchive* Ar);
    void Seek(FArchive* Ar, INT InPos);
    // Probably more.
};

struct FArchive
{
    // Built from Ghidra decompilation
    // FArchive size appears to be 0x8C
public:
    FArchive_VTable* VTable;
    int ArVer;
    int ArNetVer;
    int ArLicenseeVer;

    BOOL ArIsLoading;
    BOOL ArIsSaving;
    BOOL ArIsTransacting;
    BOOL ArIsWantBinaryPropertySerialization;
    BOOL ArIsForceUnicode;
    BOOL ArIsPersistent;
    BOOL ArForEdit;
    BOOL ArForClient;
    BOOL ArForServer;
    BOOL ArIsError;
    BOOL ArIsCriticalError;
    BOOL ArContainsCookedData;
    BOOL ArContainsCode;
    BOOL ArContainsMap;
    BOOL ArForceByteSwapping;
    int ArSerializingDefaults;

    BOOL ArIgnoreArchetypeRef;
    BOOL ArIgnoreOuterRef;
    BOOL ArIgnoreClassRef;
    BOOL ArAllowEliminatingReferences;
    BOOL ArAllowLazyLoading;
    BOOL ArIsObjectReferenceCollector;
    BOOL ArIsCountingMemory;

    DWORD ArPortFlags;
    BOOL ArShouldSkipBulkData;
    BOOL ArIsSaveGame;
    BOOL ArIsFinalPackageSave;

    // Decomp shows this might be 32 or 64 bit, unsure if a second property follows and its 2 32bits
    // It'd make sense to be a 32bit as max file size is 2GiB in LE
    __int64 ArMaxSerialSize;
    //int ArMaxSerialSize;
    BOOL ArBioWareUnknown;

};

static_assert(sizeof(FArchive) == 0x8C, "FArchive size is not 0x8C!");

struct FArchiveProxy : public FArchive {
    FArchive* InnerArchive;
};

struct FMemoryArchiveBase : public FArchive {
    // C++ is weird...
    FMemoryArchiveBase(TArray<BYTE>& InData) : FArchive(), Offset(0), Data(InData) {}

public:
    INT Offset;
    TArray<BYTE>& Data;
};

static_assert(sizeof(FMemoryArchiveBase) == 0x98, "FMemoryArchiveBase size is not 0x98!");

// Just for distinction
struct FMemoryWriter : public FMemoryArchiveBase {
public:
    FMemoryWriter(TArray<BYTE>& InData) : FMemoryArchiveBase(InData)
    {
        ArIsSaving = true;
        // For serializing properly you need these as it changes serialization logic
        ArVer = AR_VER;
        ArLicenseeVer = AR_LICENSEE_VER;
    }
};

struct FBitReader : public FArchive
{
public:
    TArray<BYTE*> Buffer; // 0x8C
    INT NumBytes; //0x9C
    INT BunchPos; // 0xAO
};

struct FBitWriter : public FArchive
{
public:
    TArray<BYTE> Buffer; // 0x8C
    INT Num; //0x9C
    INT Max; // 0xAO
};

#pragma pack(pop)
