#include <cstdio>
#include <cstring>
#include <malloc.h>
#include <Windows.h>

#include "LESDK/Common/Common.hpp"
#include "LESDK/Headers.hpp"
#include "LESDK/Init.hpp"


// ! Build target marker.
// ========================================

#define STR_(x) #x
#define STR(x) STR_(x)

// This only exists so that we can manually check that built .lib files have correct targets.

#define _LESDK_MARKER "LESDK_MARKER_" STR(SDK_TARGET)
volatile char const* LESDK_MARKER = _LESDK_MARKER;

#undef STR_
#undef STR


// ! Internal functions for Core.hpp.
// ========================================

void Detail::DefaultWarn(char const* const Message) {
    std::fprintf(stderr, "LESDK WARNING: %s\n", Message);
}


// ! Unreal Engine's global allocator.
// ========================================

void FMallocLike::GetPS3AllocationInfo(void* A, void* B, void* C) {
    (void)A; (void)B; (void)C;
}

void FMallocLike::CheckMemoryFragmentation(void* Ar) {
    (void)Ar;
}

UBOOL FMallocLike::TrimMemory(SIZE_T A, UBOOL B) {
    (void)A; (void)B;
    return FALSE;
}

UINT FMallocLike::BioHeapCheck() {
    int const rc = _heapchk();

    switch (rc) {
    case _HEAPBADBEGIN:
        LESDK_WARN("HEAPCHK: _HEAPBADBEGIN");
        break;
    case _HEAPBADNODE:
        LESDK_WARN("HEAPCHK: _HEAPBADNODE");
        break;
    case _HEAPBADPTR:
        LESDK_WARN("HEAPCHK: _HEAPBADPTR");
        break;
    case _HEAPEMPTY:
        LESDK_WARN("HEAPCHK: _HEAPEMPTY");
        break;
    }

    return TRUE;
}


void* sdkMalloc(DWORD const Count, DWORD const Alignment) {
    LESDK_CHECK(Count != 0, "");
    LESDK_CHECK(Alignment % 8 == 0, "");
    LESDK_CHECK(GMalloc != nullptr, "");
    LESDK_CHECK(*GMalloc != nullptr, "");
    return (*GMalloc)->Malloc(Count, Alignment);
}

void* sdkRealloc(void* const Orig, DWORD const Count, DWORD const Alignment) {
    LESDK_CHECK(Orig != nullptr, "");
    LESDK_CHECK(Count != 0, "");
    LESDK_CHECK(Alignment % 8 == 0, "");
    LESDK_CHECK(GMalloc != nullptr, "");
    LESDK_CHECK(*GMalloc != nullptr, "");
    return (*GMalloc)->Realloc(Orig, Count, Alignment);
}

void sdkFree(void* const Orig) {
    LESDK_CHECK(Orig != nullptr, "");
    LESDK_CHECK(GMalloc != nullptr, "");
    LESDK_CHECK(*GMalloc != nullptr, "");
    (*GMalloc)->Free(Orig);
}


// ! String transcoding.
// ========================================

UINT LESDK::GetAnsiLengthWide(WCHAR const* const InWideStr, UINT const InWideLength) {
    auto const Length = ::WideCharToMultiByte(CP_ACP, 0u, InWideStr, static_cast<int>(InWideLength), nullptr, 0, nullptr, nullptr);
    LESDK_CHECK(Length != 0, "");
    return static_cast<UINT>(Length);
}

UINT LESDK::GetUtf8LengthWide(WCHAR const* const InWideStr, UINT const InWideLength) {
    auto const Length = ::WideCharToMultiByte(CP_UTF8, 0u, InWideStr, static_cast<int>(InWideLength), nullptr, 0, nullptr, nullptr);
    LESDK_CHECK(Length != 0, "");
    return static_cast<UINT>(Length);
}

UINT LESDK::GetWideLengthAnsi(char const* const InAnsiStr, UINT const InAnsiLength) {
    auto const Length = ::MultiByteToWideChar(CP_ACP, 0u, InAnsiStr, static_cast<int>(InAnsiLength), nullptr, 0);
    LESDK_CHECK(Length != 0, "");
    return static_cast<UINT>(Length);
}

UINT LESDK::GetWideLengthUtf8(char const* const InUtf8Str, UINT const InUtf8Length) {
    auto const Length = ::MultiByteToWideChar(CP_UTF8, 0u, InUtf8Str, static_cast<int>(InUtf8Length), nullptr, 0);
    LESDK_CHECK(Length != 0, "");
    return static_cast<UINT>(Length);
}

bool LESDK::EncodeAnsiFromWide(WCHAR const* const InWideStr, UINT const InWideLength,
    char* const OutAnsiStr, UINT const OutAnsiLength, DWORD* const pOutError)
{
    auto const Check = ::WideCharToMultiByte(CP_ACP, 0u,
        InWideStr, static_cast<int>(InWideLength),
        OutAnsiStr, static_cast<int>(OutAnsiLength),
        nullptr, nullptr);

    if (static_cast<UINT>(Check) != OutAnsiLength) {
        if (pOutError != nullptr) {
            *pOutError = ::GetLastError();
        }
        return false;
    }
    return true;
}

bool LESDK::EncodeUtf8FromWide(WCHAR const* const InWideStr, UINT const InWideLength,
    char* const OutUtf8Str, UINT const OutUtf8Length, DWORD* const pOutError)
{
    auto const Check = ::WideCharToMultiByte(CP_UTF8, 0u,
        InWideStr, static_cast<int>(InWideLength),
        OutUtf8Str, static_cast<int>(OutUtf8Length),
        nullptr, nullptr);

    if (static_cast<UINT>(Check) != OutUtf8Length) {
        if (pOutError != nullptr) {
            *pOutError = ::GetLastError();
        }
        return false;
    }
    return true;
}

bool LESDK::EncodeWideFromAnsi(char const* const InAnsiStr, UINT const InAnsiLength,
    WCHAR* const OutWideStr, UINT const OutWideLength, DWORD* const pOutError)
{
    auto const Check = ::MultiByteToWideChar(CP_ACP, 0u,
        InAnsiStr, static_cast<int>(InAnsiLength),
        OutWideStr, static_cast<int>(OutWideLength));

    if (static_cast<UINT>(Check) != OutWideLength) {
        if (pOutError != nullptr) {
            *pOutError = ::GetLastError();
        }
        return false;
    }
    return true;
}

bool LESDK::EncodeWideFromUtf8(char const* const InUtf8Str, UINT const InUtf8Length,
    WCHAR* const OutWideStr, UINT const OutWideLength, DWORD* const pOutError)
{
    auto const Check = ::MultiByteToWideChar(CP_UTF8, 0u,
        InUtf8Str, static_cast<int>(InUtf8Length),
        OutWideStr, static_cast<int>(OutWideLength));

    if (static_cast<UINT>(Check) != OutWideLength) {
        if (pOutError != nullptr) {
            *pOutError = ::GetLastError();
        }
        return false;
    }
    return true;
}


// ! CRC32 hashes.
// ========================================

// Adapted from: https://github.com/SirCxyrtyx/ASI_LEC_Loader/blob/114c65ab734a4267f1670ae794873d5708f0b20b/LEC_NativeTest/LEC_NativeTest.cpp#L69.

static DWORD GCRCTable[] = {
    0, 79764919, 159529838, 222504665, 319059676, 398814059, 445009330, 507990021, 638119352, 583659535, 797628118,
    726387553, 890018660, 835552979, 1015980042, 944750013, 1276238704, 1221641927, 1167319070, 1095957929,
    1595256236, 1540665371, 1452775106, 1381403509, 1780037320, 1859660671, 1671105958, 1733955601, 2031960084,
    2111593891, 1889500026, 1952343757, 2552477408, 2632100695, 2443283854, 2506133561, 2334638140, 2414271883,
    2191915858, 2254759653, 3190512472, 3135915759, 3081330742, 3009969537, 2905550212, 2850959411, 2762807018,
    2691435357, 3560074640, 3505614887, 3719321342, 3648080713, 3342211916, 3287746299, 3467911202, 3396681109,
    4063920168, 4143685023, 4223187782, 4286162673, 3779000052, 3858754371, 3904687514, 3967668269, 881225847,
    809987520, 1023691545, 969234094, 662832811, 591600412, 771767749, 717299826, 311336399, 374308984, 453813921,
    533576470, 25881363, 88864420, 134795389, 214552010, 2023205639, 2086057648, 1897238633, 1976864222, 1804852699,
    1867694188, 1645340341, 1724971778, 1587496639, 1516133128, 1461550545, 1406951526, 1302016099, 1230646740,
    1142491917, 1087903418, 2896545431, 2825181984, 2770861561, 2716262478, 3215044683, 3143675388, 3055782693,
    3001194130, 2326604591, 2389456536, 2200899649, 2280525302, 2578013683, 2640855108, 2418763421, 2498394922,
    3769900519, 3832873040, 3912640137, 3992402750, 4088425275, 4151408268, 4197601365, 4277358050, 3334271071,
    3263032808, 3476998961, 3422541446, 3585640067, 3514407732, 3694837229, 3640369242, 1762451694, 1842216281,
    1619975040, 1682949687, 2047383090, 2127137669, 1938468188, 2001449195, 1325665622, 1271206113, 1183200824,
    1111960463, 1543535498, 1489069629, 1434599652, 1363369299, 622672798, 568075817, 748617968, 677256519,
    907627842, 853037301, 1067152940, 995781531, 51762726, 131386257, 177728840, 240578815, 269590778, 349224269,
    429104020, 491947555, 4046411278, 4126034873, 4172115296, 4234965207, 3794477266, 3874110821, 3953728444,
    4016571915, 3609705398, 3555108353, 3735388376, 3664026991, 3290680682, 3236090077, 3449943556, 3378572211,
    3174993278, 3120533705, 3032266256, 2961025959, 2923101090, 2868635157, 2813903052, 2742672763, 2604032198,
    2683796849, 2461293480, 2524268063, 2284983834, 2364738477, 2175806836, 2238787779, 1569362073, 1498123566,
    1409854455, 1355396672, 1317987909, 1246755826, 1192025387, 1137557660, 2072149281, 2135122070, 1912620623,
    1992383480, 1753615357, 1816598090, 1627664531, 1707420964, 295390185, 358241886, 404320391, 483945776,
    43990325, 106832002, 186451547, 266083308, 932423249, 861060070, 1041341759, 986742920, 613929101, 542559546,
    756411363, 701822548, 3316196985, 3244833742, 3425377559, 3370778784, 3601682597, 3530312978, 3744426955,
    3689838204, 3819031489, 3881883254, 3928223919, 4007849240, 4037393693, 4100235434, 4180117107, 4259748804,
    2310601993, 2373574846, 2151335527, 2231098320, 2596047829, 2659030626, 2470359227, 2550115596, 2947551409,
    2876312838, 2788305887, 2733848168, 3165939309, 3094707162, 3040238851, 2985771188
};

DWORD LESDK::WideStringHashCI(WCHAR const* Str) noexcept {
    LESDK_CHECK(Str != nullptr, "");
    DWORD Result = 0u;

    while (*Str) {
        WCHAR const Char = static_cast<WCHAR>(std::toupper(static_cast<int>(*Str++)));
        Result = ((Result >> 8) & 0x00FFFFFF) ^ GCRCTable[(Result ^ (Char)) & 0x000000FF];
        Result = ((Result >> 8) & 0x00FFFFFF) ^ GCRCTable[(Result ^ (Char >> 8)) & 0x000000FF];
    }

    return Result;
}

DWORD LESDK::MemCrc32(void* const InData, int const Length, DWORD Crc) {
    BYTE* Data = (BYTE*)InData;
    Crc = ~Crc;
    for (INT i = 0; i < Length; i++) {
        Crc = (Crc << 8) ^ GCRCTable[(Crc >> 24) ^ Data[i]];
    }
    return ~Crc;
}

DWORD LESDK::MemCrc32(void const* const InData, int const Length, DWORD Crc) {
    BYTE* Data = (BYTE*)InData;
    Crc = ~Crc;
    for (INT i = 0; i < Length; i++) {
        Crc = (Crc << 8) ^ GCRCTable[(Crc >> 24) ^ Data[i]];
    }
    return ~Crc;
}

DWORD LESDK::MemCrc32(std::span<BYTE> const InData, DWORD const Crc) {
    return MemCrc32(InData.data(), int(InData.size()), Crc);
}

DWORD LESDK::MemCrc32(std::span<BYTE const> const InData, DWORD const Crc) {
    return MemCrc32(InData.data(), int(InData.size()), Crc);
}


// ! SFXNameEntry & SFXName implementation.
// ========================================

bool SFXNameEntry::HasNextInPool() const noexcept {
    return Index.Length != 0;
}

SFXNameEntry const* SFXNameEntry::NextInPool() const noexcept {
    auto const* const ThisAddr = reinterpret_cast<BYTE const*>(this);
    // Unicode handling here is a best-guess effort, no idea if it's actually correct.
    auto const EntryLength = static_cast<SIZE_T>(Index.Length);
    auto const SkipSize = static_cast<SIZE_T>(13) + (!IsUnicode() ? EntryLength : (EntryLength * 2));
    return reinterpret_cast<SFXNameEntry const*>(ThisAddr + SkipSize);
}


SFXName::tInitMethod* SFXName::GInitMethod = nullptr;
SFXNameEntry const** SFXName::GBioNamePools = nullptr;

SFXName::SFXName(char const* const Lookup, int const Instance, bool const bSplit) {
    //bool const bTryFindSuccess = TryFind(Lookup, Instance, this);
    //LESDK_CHECK(bTryFindSuccess, "failed to look up a name");

    // See the wchar_t accepting constructor for rationale.
    *reinterpret_cast<SIZE_T*>(this) = static_cast<SIZE_T>(-1);

    FString Widestr{};
    Widestr.AppendAnsi(Lookup);

    LESDK_CHECK(GInitMethod != nullptr, "SFXName::Init pointer must be initialized first");
    GInitMethod(this, *Widestr, Instance, TRUE, bSplit);

    [[maybe_unused]] volatile char const* CheckedReturn = GetName();
}

SFXName::SFXName(wchar_t const* const Lookup, int const Instance, bool const bSplit) {
    // Fill all bits with garbage that will crash later if not initialized.
    *reinterpret_cast<SIZE_T*>(this) = static_cast<SIZE_T>(-1);

    LESDK_CHECK(GInitMethod != nullptr, "SFXName::Init pointer must be initialized first");
    GInitMethod(this, Lookup, Instance, TRUE, bSplit);

    // Crash hard if we have initialized incorrectly.
    [[maybe_unused]] volatile char const* CheckedReturn = GetName();
}

bool SFXName::TryFind(char const* const Lookup, INT const Instance, SFXName* const OutName) {
    for (SFXNameEntry const** Pool = GBioNamePools; *Pool != nullptr; Pool++) {
        for (SFXNameEntry const* Entry = *Pool; Entry->HasNextInPool(); Entry = Entry->NextInPool()) {
            if (0 == std::strcmp(Lookup, Entry->AnsiName)) {
                SFXName Name{};

                Name.Offset = (DWORD)((unsigned long long)Entry - (unsigned long long) * Pool);
                Name.Chunk = (DWORD)((unsigned long long)Pool - (unsigned long long)GBioNamePools);
                Name.Number = Instance;

                *OutName = Name;
                return true;
            }
        }
    }

    return false;
}

SFXNameEntry* SFXName::GetEntry() noexcept {
    auto const* const Pool = reinterpret_cast<BYTE const*>(GBioNamePools[Chunk]);
    return reinterpret_cast<SFXNameEntry*>(const_cast<BYTE*>(Pool) + Offset);
}

SFXNameEntry const* SFXName::GetEntry() const noexcept {
    auto const* const Pool = reinterpret_cast<BYTE const*>(GBioNamePools[Chunk]);
    return reinterpret_cast<SFXNameEntry const*>(Pool + Offset);
}

char const* SFXName::GetName() const noexcept {
    SFXNameEntry const* const Entry = GetEntry();
    LESDK_CHECK(Entry && !Entry->IsUnicode(), "can't get this name value");
    return Entry->AnsiName;
}

SIZE_T SFXName::GetLength() const noexcept {
    SFXNameEntry const* const Entry = GetEntry();
    return static_cast<SIZE_T>(Entry->Index.Length);
}


// ! Core virtual machine stuff.
// ========================================

void FFrame::Step(UObject* const Context, void* const Result) {
    LESDK_CHECK(GNatives != nullptr, "");
    int const Byte = *Code++;
    (*GNatives[Byte])(Context, this, Result);
}


// ! Container hash functions.
// ========================================

DWORD GetTypeHash(INT8 const Value) noexcept { return static_cast<DWORD>(Value);  }
DWORD GetTypeHash(UINT8 const Value) noexcept { return static_cast<DWORD>(Value); }
DWORD GetTypeHash(INT16 const Value) noexcept { return static_cast<DWORD>(Value); }
DWORD GetTypeHash(UINT16 const Value) noexcept { return static_cast<DWORD>(Value); }
DWORD GetTypeHash(INT const Value) noexcept { return static_cast<DWORD>(Value); }
DWORD GetTypeHash(UINT const Value) noexcept { return static_cast<DWORD>(Value);  }

DWORD GetTypeHash(void* const Value) noexcept {
    return static_cast<DWORD>(reinterpret_cast<UINT64>(Value));
}

DWORD GetTypeHash(void const* const Value) noexcept {
    return static_cast<DWORD>(reinterpret_cast<UINT64>(Value));
}

DWORD GetTypeHash(FGuid& Value) noexcept {
    return ::LESDK::MemCrc32(&Value, sizeof(FGuid));
}

DWORD GetTypeHash(FGuid const& Value) noexcept {
    return ::LESDK::MemCrc32(&Value, sizeof(FGuid));
}

DWORD GetTypeHash(WCHAR* const Value) noexcept {
    return ::LESDK::WideStringHashCI(const_cast<WCHAR const*>(Value));
}

DWORD GetTypeHash(WCHAR const* const Value) noexcept {
    return ::LESDK::WideStringHashCI(Value);
}


DWORD GetTypeHash(FStringView& Value) noexcept {
    return ::LESDK::WideStringHashCI(Value.Chars());
}

DWORD GetTypeHash(FStringView const& Value) noexcept {
    return ::LESDK::WideStringHashCI(Value.Chars());
}

DWORD GetTypeHash(FString& Value) noexcept {
    return ::LESDK::WideStringHashCI(Value.Chars());
}

DWORD GetTypeHash(FString const& Value) noexcept {
    return ::LESDK::WideStringHashCI(Value.Chars());
}


// ! Console utils.
// ========================================

void LESDK::InitializeConsole() {
    ::AllocConsole();

    ::freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    ::freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);

    HANDLE const Console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo;
    GetConsoleScreenBufferInfo(Console, &lpConsoleScreenBufferInfo);
    SetConsoleScreenBufferSize(Console, { lpConsoleScreenBufferInfo.dwSize.X, 30000 });
}

void LESDK::TerminateConsole() {
    ::FreeConsole();
}


// ! Non-member global variables.
// ========================================

FMallocLike**           GMalloc = nullptr;
tNative**               GNatives = nullptr;

UEngine**               GEngine = nullptr;
USystem**               GSys = nullptr;
UWorld**                GWorld = nullptr;

void**                  GError = nullptr;
