/**
 * @file        LESDK/Common/FString.hpp
 * @brief       This file implements partial FString, Unreal's dynamic string type.
 */

#pragma once

// #include <cstdarg>
// #include <cwchar>
// #include <string>

#include <format>
#include "LESDK/Common/Core.hpp"
#include "LESDK/Common/TArray.hpp"


namespace LESDK {

    UINT GetAnsiLengthWide(WCHAR const* InWideStr, UINT InWideLength);
    UINT GetUtf8LengthWide(WCHAR const* InWideStr, UINT InWideLength);

    UINT GetWideLengthAnsi(char const* InAnsiStr, UINT InAnsiLength);
    UINT GetWideLengthUtf8(char const* InUtf8Str, UINT InUtf8Length);

    bool EncodeAnsiFromWide(WCHAR const* InWideStr, UINT InWideLength, char* OutAnsiStr, UINT OutAnsiLength, DWORD* pOutError);
    bool EncodeUtf8FromWide(WCHAR const* InWideStr, UINT InWideLength, char* OutUtf8Str, UINT OutUtf8Length, DWORD* pOutError);

    bool EncodeWideFromAnsi(char const* InAnsiStr, UINT InAnsiLength, WCHAR* OutWideStr, UINT OutWideLength, DWORD* pOutError);
    bool EncodeWideFromUtf8(char const* InUtf8Str, UINT InUtf8Length, WCHAR* OutWideStr, UINT OutWideLength, DWORD* pOutError);

    DWORD WideStringHashCI(WCHAR const* Str) noexcept;

}

class FStringView;
class FString;


// ! Hash functions.
// ========================================

DWORD GetTypeHash(FStringView& Value) noexcept;
DWORD GetTypeHash(FStringView const& Value) noexcept;

DWORD GetTypeHash(FString& Value) noexcept;
DWORD GetTypeHash(FString const& Value) noexcept;


// ! FString implementation.
// ========================================

template<bool WithRAII>
class FStringBase {
    CONTAINER_TYPEDEFS(WCHAR, UINT, INT)

protected:
    TArrayBase<value_type, WithRAII> Storage{};

public:
    FStringBase() {}
    ~FStringBase() noexcept {}

    const_pointer Chars() const noexcept;

    size_type Capacity() const noexcept;
    size_type Length() const noexcept;

    [[nodiscard]] bool Any() const noexcept;
    [[nodiscard]] bool Empty() const noexcept;

    void Clear();
    void Reset();
    void Reserve(size_type Capacity);

    INT FindStr(const_pointer Needle, bool bIgnoreCase = false) const noexcept;
    template<bool ParamWithRAII>
    INT FindStr(FStringBase<ParamWithRAII>& Needle, bool bIgnoreCase = false) const noexcept;
    template<bool ParamWithRAII>
    INT FindStr(FStringBase<ParamWithRAII> const& Needle, bool bIgnoreCase = false) const noexcept;

    bool Contains(const_pointer Needle, bool bIgnoreCase = false) const noexcept;
    bool StartsWith(const_pointer Needle, bool bIgnoreCase = false) const noexcept;

    template<typename InputIt>
    FStringBase<WithRAII>& Append(InputIt InStr, InputIt InEnd);

    FStringBase<WithRAII>& Append(const_pointer InStr);
    FStringBase<WithRAII>& Append(value_type InChar);

    template<bool ParamWithRAII>
    FStringBase<WithRAII>& Append(FStringBase<ParamWithRAII>& InString);
    template<bool ParamWithRAII>
    FStringBase<WithRAII>& Append(FStringBase<ParamWithRAII> const& InString);

    FStringBase<WithRAII>& AppendFormat(const_pointer Format, ...);
    FStringBase<WithRAII>& AppendFormatv(const_pointer Format, std::va_list Args);

    FStringBase<WithRAII>& AppendAnsi(char const* InAnsiStr);
    FStringBase<WithRAII>& AppendUtf8(char const* InUtf8Str);

    void Assign(wchar_t Char, size_type Count);

    template<bool ParamWithRAII>
    bool Equals(FStringBase<ParamWithRAII>& InString, bool bIgnoreCase = false) const noexcept;
    template<bool ParamWithRAII>
    bool Equals(FStringBase<ParamWithRAII> const& InString, bool bIgnoreCase = false) const noexcept;
    bool Equals(const_pointer InStr, bool bIgnoreCase = false) const noexcept;

    const_pointer operator*() const noexcept;

    operator std::basic_string_view<wchar_t, std::char_traits<wchar_t>>() const {
        return std::basic_string_view<wchar_t, std::char_traits<wchar_t>>{
            Storage.GetData(), Storage.GetData() + this->Length()
        };
    }

    template<typename Alloc = std::allocator<wchar_t>>
    explicit operator std::basic_string<wchar_t, std::char_traits<wchar_t>, Alloc>() const {
        std::basic_string<wchar_t, std::char_traits<wchar_t>, Alloc> Output{};
        Output.append(Storage.GetData(), Storage.GetData() + this->Length());
        return Output;
    }

    explicit operator std::span<wchar_t>() const {
        return static_cast<std::span<wchar_t>>(Storage);
    }

    template<typename Alloc = std::allocator<wchar_t>>
    explicit operator std::vector<wchar_t, Alloc>() const {
        return static_cast<std::vector<wchar_t, Alloc>>(Storage);
    }
};


template<bool WithRAII>
FStringBase<WithRAII>::const_pointer
FStringBase<WithRAII>::Chars() const noexcept {
    return Storage.Count() > 0 ? Storage.GetData() : L"";
}

template<bool WithRAII>
FStringBase<WithRAII>::size_type
FStringBase<WithRAII>::Capacity() const noexcept {
    return Storage.Capacity() > 0 ? Storage.Capacity() - 1 : Storage.Capacity();
}

template<bool WithRAII>
FStringBase<WithRAII>::size_type
FStringBase<WithRAII>::Length() const noexcept {
    return Storage.Count() > 0 ? Storage.Count() - 1 : Storage.Count();
}

template<bool WithRAII>
bool FStringBase<WithRAII>::Any() const noexcept {
    return Storage.Count() != 0;
}

template<bool WithRAII>
bool FStringBase<WithRAII>::Empty() const noexcept {
    return Storage.Count() == 0;
}

template<bool WithRAII>
void FStringBase<WithRAII>::Clear() {
    Storage.Clear();
    if (Storage.Capacity() > 0) {
        *Storage.GetData() = L'\0';
    }
}

template<bool WithRAII>
void FStringBase<WithRAII>::Reset() {
    if (Storage.Count() > 0) {
        Storage.Clear();
    }
    if (Storage.Capacity() > 0) {
        Storage.Shrink();
    }
}

template<bool WithRAII>
void FStringBase<WithRAII>::Reserve(size_type const Capacity) {
    if (Capacity > 0) {
        Storage.Reserve(Capacity + 1);
    }
}

template<bool WithRAII>
INT FStringBase<WithRAII>::FindStr(const_pointer const Needle, bool const bIgnoreCase) const noexcept {
    LESDK_CHECK(Needle != nullptr, "");

    auto const wcsstri = [](const_pointer Haystack, const_pointer const Needle) -> const_pointer
        {
            size_t const Length = wcslen(Needle);
            if (Length == 0) {
                return Haystack;
            }
            while (*Haystack != L'\0') {
                if (_wcsnicmp(Haystack, Needle, Length) == 0) {
                    return Haystack;
                }
                Haystack++;
            }
            return nullptr;
        };

    const_pointer const Chars = this->Chars();
    const_pointer const Pointer = (!bIgnoreCase) ? wcsstr(Chars, Needle) : wcsstri(Chars, Needle);

    return (Pointer != nullptr) ? static_cast<INT>(Pointer - Chars) : -1;
}

template<bool WithRAII>
template<bool ParamWithRAII>
INT FStringBase<WithRAII>::FindStr(FStringBase<ParamWithRAII>& Needle, bool const bIgnoreCase) const noexcept {
    return this->FindStr(Needle.Chars(), bIgnoreCase);
}

template<bool WithRAII>
template<bool ParamWithRAII>
INT FStringBase<WithRAII>::FindStr(FStringBase<ParamWithRAII> const& Needle, bool const bIgnoreCase) const noexcept {
    return this->FindStr(Needle.Chars(), bIgnoreCase);
}

template<bool WithRAII>
bool FStringBase<WithRAII>::Contains(const_pointer const Needle, bool const bIgnoreCase) const noexcept {
    return this->FindStr(Needle, bIgnoreCase) != -1;
}

template<bool WithRAII>
bool FStringBase<WithRAII>::StartsWith(const_pointer const Needle, bool const bIgnoreCase) const noexcept {
    auto const NeedleLength = wcslen(Needle);
    return (NeedleLength <= this->Length())
        && (this->FindStr(Needle, bIgnoreCase) == 0);
}

template<bool WithRAII>
template<typename InputIt>
FStringBase<WithRAII>&
FStringBase<WithRAII>::Append(InputIt const InStr, InputIt const InEnd) {
    LESDK_CHECK(InStr < InEnd, "");

    if (InStr != InEnd) {
        size_type const SizeWithoutNull = static_cast<size_type>(InEnd - InStr);
        pointer NullTerminatorIter = nullptr;

        if (this->Any()) {
            // If this string has characters, we don't need to allocate space for a null-terminator,
            // since we can just reuse the space occupied by it currently.
            pointer const Inserted = Storage.DoInsertUninit(Storage.Count(), SizeWithoutNull);
            NullTerminatorIter = std::copy_n(InStr, SizeWithoutNull, Inserted - 1);
        } else {
            // If the string is currently empty, a null-terminator needs to be allocated.
            pointer const Inserted = Storage.DoInsertUninit(0, SizeWithoutNull + 1);
            NullTerminatorIter = std::copy_n(InStr, SizeWithoutNull, Inserted);
        }

        *NullTerminatorIter = L'\0';
    }

    return *this;
}

template<bool WithRAII>
FStringBase<WithRAII>&
FStringBase<WithRAII>::Append(const_pointer const InStr) {
    LESDK_CHECK(InStr != nullptr, "");
    auto const Size = wcslen(InStr);
    if (Size != 0) {
        this->Append(InStr, InStr + Size);
    }
    return *this;
}

template<bool WithRAII>
FStringBase<WithRAII>&
FStringBase<WithRAII>::Append(value_type const InChar) {
    LESDK_CHECK(InChar != L'\0', "");
    value_type const Buffer[2] = { InChar, L'\0' };
    return this->Append(Buffer, Buffer + 1);
}

template<bool WithRAII>
template<bool ParamWithRAII>
FStringBase<WithRAII>&
FStringBase<WithRAII>::Append(FStringBase<ParamWithRAII>& InString) {
    if (InString.Any()) {
        auto const Ptr = InString.Chars();
        return this->Append(Ptr, Ptr + InString.Length());
    } else {
        return *this;
    }
}

template<bool WithRAII>
template<bool ParamWithRAII>
FStringBase<WithRAII>&
FStringBase<WithRAII>::Append(FStringBase<ParamWithRAII> const& InString) {
    if (InString.Any()) {
        auto const Ptr = InString.Chars();
        return this->Append(Ptr, Ptr + InString.Length());
    } else {
        return *this;
    }
}

template<bool WithRAII>
FStringBase<WithRAII>&
FStringBase<WithRAII>::AppendFormat(const_pointer const Format, ...) {
    std::va_list Args;
    va_start(Args, Format);
    auto& ThisRef = this->AppendFormatv(Format, Args);
    va_end(Args);
    return ThisRef;
}

template<bool WithRAII>
FStringBase<WithRAII>&
FStringBase<WithRAII>::AppendFormatv(const_pointer const Format, std::va_list Args) {
    std::va_list CountArgs;
    va_copy(CountArgs, Args);
    int const FormatCount = _vscwprintf(Format, CountArgs);
    va_end(CountArgs);

    // Do not include present null-terminator in new length calculation.
    LESDK_CHECK(FormatCount >= 0, "invalid _vscwprintf format");
    size_type const NewLength = this->Length() + FormatCount;

    // Reserve space for a new null-terminator too.
    Storage.Reserve(NewLength + 1);

    // Write 'FormatCount' characters + null-terminator, overwriting the present null-terminator.
    int const CheckCount = _vsnwprintf(Storage.GetData() + this->Length(), static_cast<size_t>(-1), Format, Args);
    LESDK_CHECK(CheckCount == FormatCount, "unknown _vsnwprintf failure");

    Storage.CountItems = NewLength + 1;
    return *this;
}

template<bool WithRAII>
FStringBase<WithRAII>& FStringBase<WithRAII>::AppendAnsi(char const* const InAnsiStr) {
    LESDK_CHECK(InAnsiStr != nullptr, "");
    if (InAnsiStr != nullptr) {
        auto const InAnsiLength = static_cast<size_type>(std::strlen(InAnsiStr));
        auto const OutWideLength = LESDK::GetWideLengthAnsi(InAnsiStr, InAnsiLength);

        size_type const NewLength = this->Length() + OutWideLength;
        Storage.Reserve(NewLength + 1);

        DWORD WinError = 0u;
        bool const bEncodeOkay = LESDK::EncodeWideFromAnsi(InAnsiStr, InAnsiLength,
                Storage.GetData() + this->Length(), OutWideLength, &WinError);
        LESDK_CHECK(bEncodeOkay, "failure encoding ansi as wide string");

        Storage.GetData()[NewLength] = L'\0';
        Storage.CountItems = NewLength + 1;
    }
    return *this;
}

template<bool WithRAII>
void FStringBase<WithRAII>::Assign(wchar_t const Char, size_type const Count)
{
    LESDK_CHECK(Count > 0, "");
    if (Count > 0) {
        this->Storage.Resize(Count + 1);
        this->Storage(Count) = L'\0';
        std::fill_n(this->Storage.GetData(), Count, Char);
    }
}

template<bool WithRAII>
FStringBase<WithRAII>& FStringBase<WithRAII>::AppendUtf8(char const* const InUtf8Str) {
    LESDK_CHECK(InUtf8Str != nullptr, "");
    if (InUtf8Str != nullptr) {
        auto const InUtf8Length = static_cast<size_type>(std::strlen(InUtf8Str));
        auto const OutWideLength = LESDK::GetWideLengthUtf8(InUtf8Str, InUtf8Length);

        size_type const NewLength = this->Length() + OutWideLength;
        Storage.Reserve(NewLength + 1);

        DWORD WinError = 0u;
        bool const bEncodeOkay = LESDK::EncodeWideFromUtf8(InUtf8Str, InUtf8Length,
            Storage.GetData() + this->Length(), OutWideLength, &WinError);
        LESDK_CHECK(bEncodeOkay, "failure encoding utf-8 as wide string");

        Storage.GetData()[NewLength] = L'\0';
        Storage.CountItems = NewLength + 1;
    }
    return *this;
}

template<bool WithRAII>
template<bool ParamWithRAII>
bool FStringBase<WithRAII>::Equals(FStringBase<ParamWithRAII>& InString, bool const bIgnoreCase) const noexcept {
    return this->Length() == InString.Length() && 0 == this->FindStr(InString, bIgnoreCase);
}

template<bool WithRAII>
template<bool ParamWithRAII>
bool FStringBase<WithRAII>::Equals(FStringBase<ParamWithRAII> const& InString, bool const bIgnoreCase) const noexcept {
    return this->Length() == InString.Length() && 0 == this->FindStr(InString, bIgnoreCase);
}

template<bool WithRAII>
bool FStringBase<WithRAII>::Equals(const_pointer const InStr, bool const bIgnoreCase) const noexcept {
    auto const InLength = InStr != nullptr ? std::wcslen(InStr) : static_cast<size_type>(-1);
    return this->Length() == InLength && 0 == this->FindStr(InStr, bIgnoreCase);
}

template<bool WithRAII>
FStringBase<WithRAII>::const_pointer
FStringBase<WithRAII>::operator*() const noexcept {
    return this->Chars();
}


// ! Comparison operators for std::map support
// ========================================

template<bool WithRAII>
inline bool operator<(FStringBase<WithRAII> const& Lhs, FStringBase<WithRAII> const& Rhs) noexcept {
    return wcscmp(Lhs.Chars(), Rhs.Chars()) < 0;
}

template<bool WithRAII>
inline bool operator<=(FStringBase<WithRAII> const& Lhs, FStringBase<WithRAII> const& Rhs) noexcept {
    return wcscmp(Lhs.Chars(), Rhs.Chars()) <= 0;
}

template<bool WithRAII>
inline bool operator>(FStringBase<WithRAII> const& Lhs, FStringBase<WithRAII> const& Rhs) noexcept {
    return wcscmp(Lhs.Chars(), Rhs.Chars()) > 0;
}

template<bool WithRAII>
inline bool operator>=(FStringBase<WithRAII> const& Lhs, FStringBase<WithRAII> const& Rhs) noexcept {
    return wcscmp(Lhs.Chars(), Rhs.Chars()) >= 0;
}


/**
 * @brief   Dynamic string compatible with Unreal Engine, without RAII semantics.
 */
class FStringView final : public FStringBase<false> {
    CONTAINER_TYPEDEFS(WCHAR, UINT, INT)

public:
    FStringView() : FStringBase{} {}
    ~FStringView() noexcept {}

    // Copy operations make a field-wise copy, and move operations
    // are disabled, just like with TArrayView. Same warnings apply.

    FStringView(FStringView const& Other);
    FStringView& operator=(FStringView const& Other);
    FStringView(FStringView&& Other) = delete;
    FStringView& operator=(FStringView&& Other) = delete;

    friend inline bool operator==(FStringView const& Lhs, FStringView const& Rhs) noexcept;
    friend inline bool operator!=(FStringView const& Lhs, FStringView const& Rhs) noexcept;

    friend class FString;
};

static_assert(std::is_copy_assignable_v<FStringView>);
static_assert(std::is_copy_constructible_v<FStringView>);
static_assert(! std::is_move_assignable_v<FStringView>);
static_assert(! std::is_move_constructible_v<FStringView>);

inline FStringView::FStringView(FStringView const& Other) {
    Storage = Other.Storage;
}

inline FStringView& FStringView::operator=(FStringView const& Other) {
    if (this != &Other) {
        Storage = Other.Storage;
    }
    return *this;
}

inline bool operator==(FStringView const& Lhs, FStringView const& Rhs) noexcept {
    return Lhs.Equals(Rhs, true);
}

inline bool operator!=(FStringView const& Lhs, FStringView const& Rhs) noexcept {
    return !(Lhs == Rhs);
}


/**
 * @brief   Dynamic string compatible with Unreal Engine, with RAII semantics.
 * @remarks Essentially it is an @ref FStringView with RAII support.
 */
class FString final : public FStringBase<true> {
    CONTAINER_TYPEDEFS(WCHAR, UINT, INT)

public:
    FString() : FStringBase{} {}
    ~FString() noexcept {}

    FString(const_pointer InStr);
    FString& operator=(const_pointer InStr);


    // Copy / move semantics for FString mirror those of TArray.

    FString(FString const& Other);
    FString& operator=(FString const& Other);
    FString(FString&& Other) noexcept;
    FString& operator=(FString&& Other) noexcept;

    inline static FString Printf(const_pointer Format, ...);
    inline static FString Printfv(const_pointer Format, std::va_list Args);

    friend inline bool operator==(FString const& Lhs, FString const& Rhs) noexcept;
    friend inline bool operator!=(FString const& Lhs, FString const& Rhs) noexcept;
};

static_assert(std::is_copy_assignable_v<FString>);
static_assert(std::is_copy_constructible_v<FString>);
static_assert(std::is_move_assignable_v<FString>);
static_assert(std::is_move_constructible_v<FString>);

inline FString::FString(const_pointer const InStr)
    : FStringBase{}
{
    this->Append(InStr);
}

inline FString& FString::operator=(const_pointer const InStr) {
    if (InStr != Storage.GetData()) {
        this->Reset();
        this->Append(InStr);
    }
    return *this;
}

inline FString::FString(FString const& Other)
    : FStringBase{}
{
    this->Append(Other);
}

inline FString& FString::operator=(FString const& Other) {
    if (this != &Other) {
        this->Reset();
        this->Append(Other);
    }
    return *this;
}

inline FString::FString(FString&& Other) noexcept
    : FStringBase{}
{
    Storage.Data = std::exchange(Other.Storage.Data, nullptr);
    Storage.CountItems = std::exchange(Other.Storage.CountItems, 0);
    Storage.CountMax = std::exchange(Other.Storage.CountMax, 0);
}

inline FString& FString::operator=(FString&& Other) noexcept {
    if (this != &Other) {
        this->Reset();
        Storage.Data = std::exchange(Other.Storage.Data, nullptr);
        Storage.CountItems = std::exchange(Other.Storage.CountItems, 0);
        Storage.CountMax = std::exchange(Other.Storage.CountMax, 0);
    }
    return *this;
}

inline FString FString::Printf(const_pointer const Format, ...) {
    FString Instance{};
    std::va_list Args;
    va_start(Args, Format);
    Instance.AppendFormatv(Format, Args);
    va_end(Args);
    return Instance;
}

inline FString FString::Printfv(const_pointer const Format, std::va_list Args) {
    FString Instance{};
    Instance.AppendFormatv(Format, Args);
    return Instance;
}

inline bool operator==(FString const& Lhs, FString const& Rhs) noexcept {
    return Lhs.Equals(Rhs, true);
}

inline bool operator!=(FString const& Lhs, FString const& Rhs) noexcept {
    return !(Lhs == Rhs);
}


static_assert(sizeof(FStringView) == 0x10);
static_assert(sizeof(FString) == 0x10);


// ! std::formatter specializations
// ========================================

template<bool WithRAII>
struct std::formatter<FStringBase<WithRAII>, WCHAR> {
    std::formatter<std::wstring_view, WCHAR> underlying;

    constexpr auto parse(std::basic_format_parse_context<WCHAR>& ctx) {
        return underlying.parse(ctx);
    }

    auto format(const FStringBase<WithRAII>& str, std::wformat_context& ctx) const {
        return underlying.format(std::wstring_view(str.Chars(), str.Length()), ctx);
    }
};

template<bool WithRAII>
struct std::formatter<FStringBase<WithRAII>, char> {
    std::formatter<std::string> underlying;

    constexpr auto parse(std::format_parse_context& ctx) {
        return underlying.parse(ctx);
    }

    auto format(const FStringBase<WithRAII>& str, std::format_context& ctx) const {
        auto const wstr = str.Chars();
        auto const wlen = str.Length();

        if (wlen == 0) {
            return underlying.format("", ctx);
        }

        auto const ansi_len = LESDK::GetAnsiLengthWide(wstr, wlen);
        std::string ansi_str(ansi_len, '\0');

        DWORD error = 0;
        if (LESDK::EncodeAnsiFromWide(wstr, wlen, ansi_str.data(), ansi_len, &error)) {
            return underlying.format(ansi_str, ctx);
        }

        return underlying.format("<encoding error>", ctx);
    }
};

template<>
struct std::formatter<FString, WCHAR> : std::formatter<FStringBase<true>, WCHAR> {};

template<>
struct std::formatter<FString, char> : std::formatter<FStringBase<true>, char> {};

template<>
struct std::formatter<FStringView, WCHAR> : std::formatter<FStringBase<false>, WCHAR> {};

template<>
struct std::formatter<FStringView, char> : std::formatter<FStringBase<false>, char> {};


// ! std::hash specialization 
// ========================================

template<bool WithRAII>
struct std::hash<FStringBase<WithRAII>> {
    std::size_t operator()(FStringBase<WithRAII> const& str) const noexcept {
        return LESDK::WideStringHashCI(str.Chars());
    }
};

template<>
struct std::hash<FString> : std::hash<FStringBase<true>> {};

template<>
struct std::hash<FStringView> : std::hash<FStringBase<false>> {};

