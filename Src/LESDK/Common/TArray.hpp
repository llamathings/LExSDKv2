/**
 * @file        LESDK/Common/TArray.hpp
 * @brief       This file implements partial TArray, Unreal's dynamic array type.
 */

#pragma once

// #include <algorithm>
// #include <concepts>
// #include <initializer_list>
// #include <memory>
// #include <span>
// #include <type_traits>
// #include <vector>

#include "LESDK/Common/Core.hpp"


template<typename T> concept TArrayElement = true;


template<TArrayElement T, bool WithRAII>
class TArrayBase {
    CONTAINER_TYPEDEFS(T, UINT, INT)

protected:

    pointer     Data{ nullptr };
    size_type   CountItems{ 0 };
    size_type   CountMax{ 0 };

    // Destructor must not be virtual to avoid introducing vtable.
    // This makes use of compile-time conditional to deallocate if WithRAII is set.
    ~TArrayBase() noexcept;

public:

    static constexpr SIZE_T k_defaultAlignment = UN_DEFAULT_ALIGNMENT;
    static_assert(k_defaultAlignment && ((k_defaultAlignment& (k_defaultAlignment - 1)) == 0));

    template<bool DerivedWithRAII>
    friend class FStringBase;
    friend class FString;

    TArrayBase() = default;
    TArrayBase(std::initializer_list<value_type> List);

    pointer GetData() noexcept;
    const_pointer GetData() const noexcept;

    size_type Capacity() const noexcept;
    size_type Count() const noexcept;

    [[nodiscard]] bool Any() const noexcept;
    [[nodiscard]] bool Empty() const noexcept;

    void Clear();
    void Reserve(size_type Capacity);
    void Shrink();

    void Resize(size_type NewCount);
    void Resize(size_type NewCount, const_reference Value);

    reference Add();
    void Add(const_reference Value);
    void Add(value_type&& Value)
        requires (std::movable<value_type>);

    size_type AddUninit(size_type AddedCount);

    reference Insert(size_type Position);
    reference Insert(size_type Position, const_reference Value);
    reference Insert(size_type Position, value_type&& Value)
        requires (std::movable<value_type>);

    void InsertRange(size_type Position, size_type Count, const_reference Value);
    template<typename InputIt>
    void InsertRange(size_type Position, InputIt First, InputIt Last);
    void InsertRange(size_type Position, std::initializer_list<value_type> List);

    void Erase(size_type Position, size_type Count = 1);

    reference operator()(size_type Index);
    const_reference operator()(size_type Index) const;

protected:

    void DoDestroyRange(size_type Offset);
    void DoDestroyRange(size_type Offset, size_type Count);

    void DoDestroyContents();
    pointer DoEraseUninit(size_type Offset, size_type Count);
    pointer DoInsertUninit(size_type Offset, size_type Count);

    size_type FindNextCapacity(size_type LowestBound);

    [[nodiscard]]   static pointer  DoAlloc(size_type Count);
    [[nodiscard]]   static pointer  DoRealloc(pointer OldData, size_type OldCount, size_type NewCount);
                    static void     DoFree(pointer Data);

public:  // Standard container compatibility.

    std::span<value_type> AsSpan() const noexcept;

    template<typename Allocator = std::allocator<value_type>>
    std::vector<value_type, Allocator> AsVec(Allocator const& Alloc = Allocator()) const noexcept;

public:  // Standard iterator compatibility.

    [[nodiscard]] inline iterator           begin() noexcept { return Data; }
    [[nodiscard]] inline const_iterator     begin() const noexcept { return Data; }
    [[nodiscard]] inline const_iterator     cbegin() const noexcept { return Data; }
    [[nodiscard]] inline iterator           end() noexcept { return Data + CountItems; }
    [[nodiscard]] inline const_iterator     end() const noexcept { return Data + CountItems; }
    [[nodiscard]] inline const_iterator     cend() const noexcept { return Data + CountItems; }

};


template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::~TArrayBase() noexcept {
    if constexpr (WithRAII) {
        DoDestroyContents();
    }
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::TArrayBase(std::initializer_list<value_type> const List)
    : TArrayBase{}
{
    InsertRange(0, List);
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::pointer
TArrayBase<T, WithRAII>::GetData() noexcept {
    return Data;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::const_pointer
TArrayBase<T, WithRAII>::GetData() const noexcept {
    return Data;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::size_type
TArrayBase<T, WithRAII>::Capacity() const noexcept {
    return CountMax;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::size_type
TArrayBase<T, WithRAII>::Count() const noexcept {
    return CountItems;
}

template<TArrayElement T, bool WithRAII>
bool TArrayBase<T, WithRAII>::Any() const noexcept {
    return CountItems != 0;
}

template<TArrayElement T, bool WithRAII>
bool TArrayBase<T, WithRAII>::Empty() const noexcept {
    return CountItems == 0;
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Clear() {
    if (CountItems != 0) {
        // Can't use Resize() because it requires a default-constructor.
        DoDestroyRange(0);
        CountItems = 0;
    }
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Reserve(size_type const Capacity) {
    auto const CurrentCapacity = this->Capacity();
    if (Capacity > CurrentCapacity) {
        if (CurrentCapacity == 0) {
            Data = DoAlloc(Capacity);
        } else {
            Data = DoRealloc(Data, CurrentCapacity, Capacity);
        }
        CountMax = Capacity;
    }
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Shrink() {
    if (CountItems < CountMax) {
        if (CountItems != 0) {
            Data = DoRealloc(Data, CountMax, CountItems);
        } else {
            DoFree(std::exchange(Data, nullptr));
        }
        CountMax = CountItems;
    }
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Resize(size_type const NewCount) {
    Resize(NewCount, T{});
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Resize(size_type const NewCount, const_reference const Value) {
    if (NewCount > CountItems) {
        Reserve(NewCount);
        std::uninitialized_fill(Data + CountItems, Data + NewCount, Value);
    } else if (NewCount < CountItems) {
        DoDestroyRange(NewCount);
    }
    CountItems = NewCount;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::reference
TArrayBase<T, WithRAII>::Add() {
    return Insert(Count(), T{});
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Add(const_reference Value) {
    Insert(Count(), Value);
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Add(value_type&& Value)
    requires (std::movable<value_type>)
{
    Insert(Count(), std::move(Value));
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::size_type
TArrayBase<T, WithRAII>::AddUninit(size_type const AddedCount) {
    auto const OrigCount = CountItems;
    CountItems += AddedCount;
    this->Reserve(CountItems);
    return OrigCount;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::reference
TArrayBase<T, WithRAII>::Insert(size_type const Position) {
    return Insert(Position, T{});
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::reference
TArrayBase<T, WithRAII>::Insert(size_type const Position, const_reference Value) {
    LESDK_CHECK(Position <= CountItems, "");

    pointer const Inserted = DoInsertUninit(Position, 1);
    new (Inserted) T{ Value };

    return *Inserted;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::reference
TArrayBase<T, WithRAII>::Insert(size_type Position, value_type&& Value)
    requires (std::movable<value_type>)
{
    LESDK_CHECK(Position <= CountItems, "");

    pointer const Inserted = DoInsertUninit(Position, 1);
    new (Inserted) T{ std::move(Value) };

    return *Inserted;
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::InsertRange(size_type const Position, size_type const Count, const_reference Value) {
    LESDK_CHECK(Position <= CountItems, "");
    LESDK_CHECK(Count > 0, "");

    pointer const Inserted = DoInsertUninit(Position, Count);
    std::fill_n(Inserted, Count, Value);
}

template<TArrayElement T, bool WithRAII>
template<typename InputIt>
void TArrayBase<T, WithRAII>::InsertRange(size_type const Position, InputIt const First, InputIt const Last) {
    LESDK_CHECK(Position <= CountItems, "");
    LESDK_CHECK(First <= Last, "");

    if (First < Last) {
        auto const Count = static_cast<size_type>(Last - First);
        pointer const Inserted = DoInsertUninit(Position, Count);
        // TODO: Should we maybe move here if possible?
        std::copy(First, Last, Inserted);
    }
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::InsertRange(size_type const Position, std::initializer_list<value_type> const List) {
    this->InsertRange(Position, List.begin(), List.end());
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::Erase(size_type const Position, size_type const Count) {
    LESDK_CHECK(Count > 0, "");
    LESDK_CHECK(Position <= CountItems, "");
    LESDK_CHECK(Position + Count <= CountItems, "");

    DoDestroyRange(Position, Count);
    DoEraseUninit(Position, Count);
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::reference
TArrayBase<T, WithRAII>::operator()(size_type const Index) {
    LESDK_CHECK(Index < Count(), "");
    return Data[Index];
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::const_reference
TArrayBase<T, WithRAII>::operator()(size_type const Index) const {
    LESDK_CHECK(Index < Count(), "");
    return Data[Index];
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::DoDestroyRange(size_type const Offset) {
    if constexpr (!std::is_trivially_destructible<T>::value) {
        std::destroy(Data + Offset, Data + CountItems);
    }
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::DoDestroyRange(size_type const Offset, size_type const Count) {
    if constexpr (!std::is_trivially_destructible<T>::value) {
        auto const Bound = std::min<size_type>(Offset + Count, CountItems);
        for (auto Index = Offset; Index < Bound; ++Index) {
            (&this->operator(Index))->~T();
        }
    }
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::DoDestroyContents() {
    Clear();
    Shrink();
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::pointer
TArrayBase<T, WithRAII>::DoEraseUninit(size_type const Offset, size_type const Count) {
    LESDK_CHECK(Count > 0, "");
    LESDK_CHECK(Offset <= CountItems, "");
    LESDK_CHECK(Offset + Count <= CountItems, "");

    if (CountItems >= Offset + Count) {
        if (CountItems != Offset + Count) {
            std::move(Data + Offset + Count, Data + Offset + CountItems, Data + Offset);
        }
        CountItems -= Count;
    }

    return &Data[Offset];
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::pointer
TArrayBase<T, WithRAII>::DoInsertUninit(size_type const Offset, size_type const Count) {
    LESDK_CHECK(Offset <= CountItems, "");
    LESDK_CHECK(Count > 0, "");

    auto const NewCountItems = CountItems + Count;
    if (NewCountItems > CountMax) {
        auto const NextCountMax = FindNextCapacity(NewCountItems);
        Reserve(NextCountMax);
    }

    // 1. Find number (N) of elements to be shifted right.
    // 2. Copy N elements from [Data + Offset] to [Data + Offset + Count].

    auto const MoveCount = CountItems - Offset;
    if (MoveCount != 0) {
        auto const MoveBegin = Data + Offset;

        if constexpr (std::movable<value_type>) {
            if (Count >= MoveCount) {
                std::move(MoveBegin, MoveBegin + MoveCount, MoveBegin + Count);
            } else {
                std::move_backward(MoveBegin, MoveBegin + MoveCount, MoveBegin + Count + MoveCount);
            }
        } else {
            if (Count >= MoveCount) {
                std::copy(MoveBegin, MoveBegin + MoveCount, MoveBegin + Count);
            } else {
                std::copy_backward(MoveBegin, MoveBegin + MoveCount, MoveBegin + Count + MoveCount);
            }
        }
    }

    CountItems = NewCountItems;
    return Data + Offset;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::size_type
TArrayBase<T, WithRAII>::FindNextCapacity(size_type const LowestBound) {
    size_type NextCountMax = std::max<size_type>(CountMax, LowestBound);

    // https://graphics.stanford.edu/%7Eseander/bithacks.html#RoundUpPowerOf2

    NextCountMax--;
    NextCountMax |= NextCountMax >> 1;
    NextCountMax |= NextCountMax >> 2;
    NextCountMax |= NextCountMax >> 4;
    NextCountMax |= NextCountMax >> 8;
    NextCountMax |= NextCountMax >> 16;
    NextCountMax++;

    return NextCountMax;
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::pointer
TArrayBase<T, WithRAII>::DoAlloc(size_type const Count) {
    return sdkMallocTyped<T>(Count, k_defaultAlignment);
}

template<TArrayElement T, bool WithRAII>
TArrayBase<T, WithRAII>::pointer
TArrayBase<T, WithRAII>::DoRealloc(pointer const OldData, size_type const OldCount, size_type const NewCount) {
    LESDK_CHECK(OldData != nullptr, "");
    LESDK_CHECK(OldCount != 0, "");
    LESDK_CHECK(NewCount != 0, "");

    // We cannot use `GMalloc->Realloc` here because copy/move constructors may need to be called.
    pointer const Allocated = sdkMallocTyped<T>(NewCount, k_defaultAlignment);
    LESDK_CHECK(Allocated != nullptr, "");

    auto const MoveCount = std::min<size_type>(OldCount, NewCount);
    if constexpr (std::movable<value_type>) {
        std::move(OldData, OldData + MoveCount, Allocated);
    } else {
        std::copy(OldData, OldData + MoveCount, Allocated);
    }
    std::destroy(OldData, OldData + OldCount);

    sdkFreeTyped(OldData);
    return Allocated;
}

template<TArrayElement T, bool WithRAII>
void TArrayBase<T, WithRAII>::DoFree(pointer const Data) {
    sdkFreeTyped(Data);
}

template<TArrayElement T, bool WithRAII>
std::span<typename TArrayBase<T, WithRAII>::value_type>
TArrayBase<T, WithRAII>::AsSpan() const noexcept {
    return std::span<value_type>(Data, Data + Count());
}

template<TArrayElement T, bool WithRAII>
template<typename Allocator>
std::vector<typename TArrayBase<T, WithRAII>::value_type, Allocator>
TArrayBase<T, WithRAII>::AsVec(Allocator const& Alloc) const noexcept {
    return std::vector<value_type, Allocator>(Data, Data + Count(), Alloc);
}


/**
 * @brief   Dynamic array compatible with Unreal Engine, without RAII semantics.
 * @tparam  T Element type.
 */
template<TArrayElement T>
class TArrayView final : public TArrayBase<T, false> {
    CONTAINER_TYPEDEFS(T, UINT, INT)

public:
    TArrayView() : TArrayBase{} {}

    // TODO: Remove the constructor, since TArrayView should be constructed via cast from TArray.
    TArrayView(std::initializer_list<value_type> const List) : TArrayBase{ List } {}

    // Copy semantics on this type make a field-wise copy, without copying the contents,
    // and move semantics are disabled. Make sure to avoid double-free operations if using
    // this type outside autogenerated headers!

    TArrayView(TArrayView const& Other) = default;
    TArrayView& operator=(TArrayView const& Other) = default;
    TArrayView(TArrayView&& Other) = delete;
    TArrayView& operator=(TArrayView&& Other) = delete;

    template<TArrayElement T> friend class TArray;
};

static_assert(std::is_copy_assignable_v<TArrayView<int>>);
static_assert(std::is_copy_constructible_v<TArrayView<int>>);
static_assert(! std::is_move_assignable_v<TArrayView<int>>);
static_assert(! std::is_move_constructible_v<TArrayView<int>>);


/**
 * @brief   Dynamic array compatible with Unreal Engine, with RAII semantics.
 * @tparam  T Element type.
 */
template<TArrayElement T>
class TArray final : public TArrayBase<T, true> {
    CONTAINER_TYPEDEFS(T, UINT, INT)

public:
    TArray() : TArrayBase{} {}
    explicit TArray(size_type Capacity);
    TArray(size_type Count, const_reference Value);
    TArray(std::initializer_list<value_type> const List) : TArrayBase{ List } {}

    ~TArray() noexcept {}

    // Unlike with TArrayView, copy / move semantics here actually copy / destroy contents.

    TArray(TArray const& Other);
    TArray& operator=(TArray const& Other);
    TArray(TArray&& Other) noexcept;
    TArray& operator=(TArray&& Other) noexcept;

    // We need to be able to seamlessly get TArrayView from TArray.
    // Until the new container types are tested, I would like to avoid implicit casting.

    TArrayView<T> AsView() noexcept;
};

static_assert(std::is_copy_assignable_v<TArray<int>>);
static_assert(std::is_copy_constructible_v<TArray<int>>);
static_assert(std::is_move_assignable_v<TArray<int>>);
static_assert(std::is_move_constructible_v<TArray<int>>);

template<TArrayElement T>
TArray<T>::TArray(size_type const Capacity)
    : TArrayBase{}
{
    this->Reserve(Capacity);
}

template<TArrayElement T>
TArray<T>::TArray(size_type const Count, const_reference Value)
    : TArrayBase{}
{
    this->InsertRange(0, Count, Value);
}

template<TArrayElement T>
TArray<T>::TArray(TArray const& Other)
    : TArray{}
{
    // This relies on InsertRange always copying elements...
    this->InsertRange(0, Other.begin(), Other.end());
}

template<TArrayElement T>
TArray<T>& TArray<T>::operator=(TArray const& Other) {
    if (this != &Other) {
        this->DoDestroyContents();
        this->InsertRange(0, Other.begin(), Other.end());
    }
    return *this;
}

template<TArrayElement T>
TArray<T>::TArray(TArray&& Other) noexcept
    : TArray{}
{
    this->Data = std::exchange(Other.Data, nullptr);
    this->CountItems = std::exchange(Other.CountItems, 0);
    this->CountMax = std::exchange(Other.CountMax, 0);
}

template<TArrayElement T>
TArray<T>& TArray<T>::operator=(TArray&& Other) noexcept {
    if (this != &Other) {
        this->DoDestroyContents();
        this->Data = std::exchange(Other.Data, nullptr);
        this->CountItems = std::exchange(Other.CountItems, 0);
        this->CountMax = std::exchange(Other.CountMax, 0);
    }
    return *this;
}

template<TArrayElement T>
TArrayView<T> TArray<T>::AsView() noexcept
{
    TArrayView<T> View{};
    View.Data = this->Data;
    View.CountItems = this->CountItems;
    View.CountMax = this->CountMax;
    return View;
}


static_assert(sizeof(TArrayView<BYTE>) == 0x10);
static_assert(sizeof(TArray<BYTE>) == 0x10);
