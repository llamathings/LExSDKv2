/**
 * @file        LESDK/Common/Core.hpp
 * @brief       This file implements partial SFXName, BioWare's optimization of FName.
 */

#pragma once

#include <compare>

#include "LESDK/Common/Core.hpp"
#include "LESDK/Common/FString.hpp"


#pragma pack(push, 1)

struct SFXPackedIndex {
    DWORD Offset : 20;
    DWORD Length : 9;
    DWORD Flags : 3;

    static constexpr DWORD k_flagSuppressed = 0x01;
    static constexpr DWORD k_flagUnicode = 0x02;
    static constexpr DWORD k_flagConst = 0x04;
};

struct SFXNameEntry {
    SFXPackedIndex Index;
    SFXNameEntry* HashNext;
    union {
        char AnsiName[1];
        WCHAR WideName[1];
    };

    bool IsSuppressed() const noexcept { return (Index.Flags & SFXPackedIndex::k_flagSuppressed) != 0; }
    bool IsUnicode() const noexcept { return (Index.Flags & SFXPackedIndex::k_flagUnicode) != 0; }
    bool IsConst() const noexcept { return (Index.Flags & SFXPackedIndex::k_flagConst) != 0; }

    bool HasNextInPool() const noexcept;
    SFXNameEntry const* NextInPool() const noexcept;
};

struct SFXName {
    // Member variables.

    DWORD Offset : 29;
    DWORD Chunk : 3;
    INT Number;

    // Global variables.

    using tInitMethod = void(SFXName* Self, WCHAR const* InName, INT InNumber, UBOOL bCreateIfNotFound, UBOOL bSplitName);

    static tInitMethod* GInitMethod;
    static SFXNameEntry const** GBioNamePools;

    // Formatting flags.

    enum FormatMode : int {
        // Format only string contents.
        k_formatBasic = 1,
        // Format both string contents and number.
        k_formatExtended = 2,
        // If number is zero, format only string contents, otherwise also format (number - 1).
        k_formatInstanced = 3,
    };

    // Public interface.

    SFXName() = default;
    SFXName(char const* Lookup, int Instance, bool bSplit = false);
    SFXName(wchar_t const* Lookup, int Instance, bool bSplit = false);

    bool operator==(SFXName const& Other) const noexcept {
        auto const Left = *(SIZE_T*)this;
        auto const Right = *(SIZE_T*)&Other;
        return Left == Right;
    }

    auto operator<=>(SFXName const& Other) const noexcept {
        auto const Left = *(SIZE_T*)this;
        auto const Right = *(SIZE_T*)&Other;
        return Left <=> Right;
    }

    SFXNameEntry* GetEntry() noexcept;
    SFXNameEntry const* GetEntry() const noexcept;
    char const* GetName() const noexcept;
    FString Instanced() const noexcept;
    SIZE_T GetLength() const noexcept;

    template<bool WithRAII>
    void AppendToString(FStringBase<WithRAII>& OutString, FormatMode Mode) const;
    inline FString ToString(FormatMode Mode = k_formatInstanced) const;

    /** Hash function for associative containers. */
    friend inline DWORD GetTypeHash(SFXName const Value) noexcept {
        return static_cast<DWORD>(*reinterpret_cast<QWORD const*>(&Value));
    }

private:

    static bool TryFind(char const* Lookup, INT Instance, SFXName* OutName);
};

static_assert(sizeof(SFXPackedIndex) == 4);
static_assert(sizeof(SFXName) == 8);


template<bool WithRAII>
void SFXName::AppendToString(FStringBase<WithRAII>& OutString, FormatMode const Mode) const {
    SFXNameEntry const* const Entry = GetEntry();

    OutString.Reserve(Entry->Index.Length + (Mode > k_formatBasic ? 5 : 0));

    if (!Entry->IsUnicode()) {
        OutString.AppendAnsi(Entry->AnsiName);
    } else {
        OutString.Append(Entry->WideName);
    }

    if (Mode == k_formatExtended) {
        OutString.AppendFormat(L"_%d", Number);
    } else if (Mode == k_formatInstanced && Number > 0) {
        OutString.AppendFormat(L"_%d", Number - 1);
    }
}

inline FString SFXName::Instanced() const noexcept {
    return ToString(SFXName::FormatMode::k_formatInstanced);
}

inline FString SFXName::ToString(FormatMode const Mode) const {
    FString OutString{};
    AppendToString(OutString, Mode);
    return OutString;
}

// ! std::hash specialization 
// ========================================

template<>
struct std::hash<SFXName> {
    std::size_t operator()(SFXName const& name) const noexcept {
        return GetTypeHash(name);
    }
};


#pragma pack(pop)
