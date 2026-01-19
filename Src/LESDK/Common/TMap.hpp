/**
 * @file        LESDK/Common/TMap.hpp
 * @brief       This file contains a partial implementation of TMap from ME3Tweaks ASIs.
 *
 * @author      SirCxyrtyx (original code, adapted with permission)
 * @see         https://github.com/ME3Tweaks/LEASIMods/blob/master/Shared-ASI/ME3Tweaks/TMap.h
 * @see         https://github.com/SirCxyrtyx/ASI_LEC_Loader/blob/master/LEC_NativeTest/LEC_NativeTest.cpp
 */

#pragma once

// #include <cstring>
// #include <tuple>

#include "LESDK/Common/Core.hpp"
#include "LESDK/Common/TArray.hpp"


#pragma pack(push, 4)


// ! FBitArray and dependencies.
// ========================================

class FBitReference final {
    DWORD&  Data;
    DWORD   Mask;

public:
    FBitReference(DWORD& InData, DWORD const InMask)
        : Data{ InData }, Mask{ InMask } {}

    operator bool() const { return (Data & Mask) != 0; }
    void operator=(bool const bNewValue) { bNewValue ? (Data |= Mask) : (Data & ~Mask); }
};

class FBitArray final {
    static constexpr int k_maxInlineElements = 4;

    DWORD   InlineData[k_maxInlineElements];
    DWORD*  IndirectData;
    int     NumBits;
    int     MaxBits;

public:

    void Empty(int const ExpectedNumBits = 0) {
        NumBits = 0;
        if (MaxBits != ExpectedNumBits) {
            MaxBits = ExpectedNumBits;
            this->Realloc(0);
        }
    }

    int AddItem(bool const InValue) {
        int const Index = NumBits;
        bool const bShouldReallocate = (NumBits + 1) > MaxBits;

        NumBits++;

        if (bShouldReallocate) {
            DWORD const MaxDWORDs = this->CalculateSlack((NumBits + 32 - 1) / 32);
            MaxBits = MaxDWORDs * 32;
            this->Realloc(NumBits - 1);
        }

        (*this)(Index) = InValue;
        return Index;
    }

    FBitReference operator()(int const Index) {
        return FBitReference(GetAllocation()[Index / 32], 1 << (Index & (32 - 1)));
    }

    FBitReference const operator()(int const Index) const {
        return FBitReference(GetAllocation()[Index / 32], 1 << (Index & (32 - 1)));
    }

private:

    DWORD* GetAllocation() const {
        return (DWORD*)(IndirectData ? IndirectData : InlineData);
    }

    void Realloc(int const PreviousNumBits) {
        int const PreviousNumDWORDs = (PreviousNumBits + 32 - 1) / 32;
        int const MaxDWORDs = (MaxBits + 32 - 1) / 32;

        this->ResizeAllocation(PreviousNumDWORDs, MaxDWORDs, sizeof(DWORD));

        if (MaxDWORDs != 0) {
            std::memset(GetAllocation() + PreviousNumDWORDs, 0, (MaxDWORDs - PreviousNumDWORDs) * sizeof(DWORD));
        }
    }

    void ResizeAllocation(int const PreviousNumElements, int const NumElements, int const BytesPerElement) {
        if (NumElements <= k_maxInlineElements) {
            if (IndirectData) {
                std::memcpy(InlineData, IndirectData, PreviousNumElements * BytesPerElement);
                sdkFree(IndirectData);
            }
        } else {
            if (IndirectData == nullptr) {
                IndirectData = (DWORD*)sdkMalloc(NumElements * BytesPerElement);
                std::memcpy(IndirectData, InlineData, PreviousNumElements * BytesPerElement);
            } else {
                IndirectData = (DWORD*)sdkRealloc(IndirectData, NumElements * BytesPerElement);
            }
        }
    }

    int CalculateSlack(int const NumElements) const {
        return NumElements <= k_maxInlineElements ? k_maxInlineElements : NumElements;
    }
};


// ! TSparseArray and dependencies.
// ========================================

template<typename TElement>
class TSparseArray final {
    #pragma warning(disable: 4624)
    union ElementOrFreeListLink {
        TElement ElementData;
        int NextFreeIndex;

        // Black magic fuckery to make this build with TArray elements' copy/move semantics
        // that is so unsafe a "Modern C++" advocate would commit ritualistic murder immediately
        // upon reading this code.

        ~ElementOrFreeListLink() noexcept {}

        ElementOrFreeListLink(ElementOrFreeListLink const& Other) {
            std::memcpy(this, &Other, sizeof(ElementOrFreeListLink));
        }

        ElementOrFreeListLink& operator=(ElementOrFreeListLink const& Other) {
            if (this != &Other) {
                std::memcpy(this, &Other, sizeof(ElementOrFreeListLink));
            }
            return *this;
        }

        // I'm glad Jason Turner can't possibly know where I live.

        // Also yes, we're not calling actual destructors here, so I have no idea
        // how TSparseArray / TSet / TMap would even work with non-POD types.  ¯\_(ツ)_/¯

    };
    #pragma warning(default: 4624)

    using DataArrayType = TArray<ElementOrFreeListLink>;

    DataArrayType           Data;
    FBitArray               AllocationFlags;
    int                     FirstFreeIndex;
    int                     NumFreeIndices;

private:

    ElementOrFreeListLink& GetData(int const Index) {
        return Data(Index);
    }

    ElementOrFreeListLink const& GetData(int const Index) const {
        return Data(Index);
    }

    bool IsAllocated(int const Index) const {
        return AllocationFlags(Index);
    }

public:

    int Num() const {
        return Data.Count() - NumFreeIndices;
    }

    std::pair<void*, int> Add() {
        std::pair<void*, int> Result{};

        if (NumFreeIndices > 0) {
            Result.second = FirstFreeIndex;
            FirstFreeIndex = GetData(FirstFreeIndex).NextFreeIndex;
            --NumFreeIndices;
        } else {
            Result.second = static_cast<int>(Data.AddUninit(1));
            AllocationFlags.AddItem(true);
        }

        // Compute the pointer to the new element's data.
        Result.first = &GetData(Result.second).ElementData;

        // Flag the element as allocated.
        AllocationFlags(Result.second) = true;

        return Result;
    }

    TElement& operator()(int const Index) {
        return *(TElement*)&GetData(Index).ElementData;
    }

    TElement const& operator()(int const Index) const {
        return *(TElement*)&GetData(Index).ElementData;
    }

    // Iterator definition.
    // ----------------------------------------

    class SparseArrayIterator final {
        using iterator_category = std::forward_iterator_tag;
        using value_type = TElement;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using reference = value_type&;
        using const_reference = value_type const&;

        TSparseArray const&     Array;
        int                     Index;

    public:

        SparseArrayIterator(TSparseArray const& InArray, bool const bAtEnd)
            : Array{ InArray }, Index{ 0 }
        {
            if (bAtEnd) {
                Index = static_cast<int>(Array.Data.Count());
            }
        }

        SparseArrayIterator& operator++() {
            do {
                ++Index;
                    // FIXME: Wait for SirCxyrtyx before considering this fixed.
            } while (Index < static_cast<int>(Array.Data.Count()) && !Array.IsAllocated(Index));
            return *this;
        }

        SparseArrayIterator operator++(int) {
            SparseArrayIterator const retval = *this;
            ++(*this);
            return retval;
        }

        bool operator==(SparseArrayIterator const Other) const {
            return &Array == &Other.Array && Index == Other.Index;
        }

        bool operator!=(SparseArrayIterator const Other) const {
            return !(*this == Other);
        }

        const_reference operator*() {
            return Array.GetData(Index).ElementData;
        }

        const_reference operator*() const {
            return Array.GetData(Index).ElementData;
        }

        int GetIndex() const {
            return Index;
        }

        operator bool() const {
            return Index < static_cast<int>(Array.Data.Count());
        }
    };

    auto begin() { return SparseArrayIterator{ this, false }; }
    auto begin() const { return SparseArrayIterator{ this, false }; }
    auto cbegin() const { return SparseArrayIterator{ this, false }; }

    auto end() { return SparseArrayIterator{ this, true }; }
    auto end() const { return SparseArrayIterator{ this, true }; }
    auto cend() const { return SparseArrayIterator{ this, true }; }
};


// ! TSet and dependencies.
// ========================================

constexpr unsigned
UnBitCeil(unsigned Value) noexcept {
    --Value;
    Value |= Value >> 1;
    Value |= Value >> 2;
    Value |= Value >> 4;
    Value |= Value >> 8;
    Value |= Value >> 16;
    return Value + 1;
}

template<typename TElement, bool CAllowDuplicateKeys = false>
struct DefaultKeyFuncs {
    typedef typename TElement KeyType;
    enum { AllowDuplicateKeys = CAllowDuplicateKeys };

    static KeyType& GetKey(TElement& Element) { return Element; }
    static bool Matches(KeyType const& Lhs, KeyType const& Rhs) { return Lhs == Rhs; }
    static DWORD GetKeyHash(KeyType const Element) { return GetTypeHash(Element); }
};

class FSetElementId final {
    int Index;

public:
    FSetElementId() : Index(-1) {}
    FSetElementId(int const InIndex) : Index(InIndex) {}

    bool IsValidId() const { return Index != -1; }
    operator int() const { return Index; }

    friend bool operator==(FSetElementId const& Lhs, FSetElementId const& Rhs) {
        return Lhs.Index == Rhs.Index;
    }
};

template<typename TElement, typename TKeyFuncs = DefaultKeyFuncs<TElement>>
class TSet final {
    typedef typename TKeyFuncs::KeyType KeyType;
    typedef typename TElement ElementType;

    class FElement final {
    public:

                ElementType         Value;
        mutable FSetElementId       HashNextId;
        mutable int                 HashIndex;

        FElement(ElementType& value) : Value(value) {};
    };

            TSparseArray<FElement>  Elements;
    mutable FSetElementId           InlineHash;
    mutable FSetElementId*          Hash;
    mutable int                     HashSize;

public:

    // Necessary for TMap to access our "Elements"'s iterator funcs.
    template<typename TKey, typename TValue> friend class TMap;


    FSetElementId Add(ElementType& InElementValue) {
        FSetElementId Id = FindId(TKeyFuncs::GetKey(InElementValue));

        if (!Id.IsValidId()) {
            auto const Allocation = Elements.Add();

            Id = FSetElementId(Allocation.second);
            FElement& Element = *new (Allocation.first) FElement(InElementValue);

            if (!ConditionalRehash(Elements.Num())) {
                HashElement(Id, Element);
            }
        } else {
            ElementType& a = Elements(Id).Value;
            a.~ElementType();
            new (&a) ElementType(InElementValue);
        }

        return Id;
    }

    ElementType* Find(KeyType const Key) {
        FSetElementId const Id = FindId(Key);
        if (Id.IsValidId()) {
            return &Elements(Id).Value;
        }
        else {
            return nullptr;
        }
    }

    ElementType const * Find(KeyType const Key) const {
        FSetElementId const Id = FindId(Key);
        if (Id.IsValidId()) {
            return &Elements(Id).Value;
        } else {
            return nullptr;
        }
    }

    ElementType& operator()(FSetElementId const Id) {
        return Elements(Id).Value;
    }

private:

    int GetNumberOfHashBuckets(int const NumHashedElements) const {
        constexpr unsigned int ElementsPerBucket = 2;
        constexpr unsigned int BaseNumberOfBuckets = 8;
        constexpr unsigned int MinNumberOfHashedElements = 4;

        if (NumHashedElements >= MinNumberOfHashedElements) {
            return (int)UnBitCeil((unsigned int)NumHashedElements / ElementsPerBucket + BaseNumberOfBuckets);
        }

        return 1;
    }

    FSetElementId& GetTypedHash(INT const InHashIndex) const {
        return GetHash()[InHashIndex & (HashSize - 1)];
    }

    void HashElement(FSetElementId const ElementId, FElement const& Element) const {
        Element.HashIndex = TKeyFuncs::GetKeyHash(TKeyFuncs::GetKey(Element.Value)) & (HashSize - 1);
        Element.HashNextId = GetTypedHash(Element.HashIndex);
        GetTypedHash(Element.HashIndex) = ElementId;
    }

    void Rehash() const {
        ResizeHash(0, 0);

        if (HashSize) {
            ResizeHash(0, HashSize);
            for (int i = 0; i < HashSize; i++) {
                GetTypedHash(i) = FSetElementId();
            }

            for (TSparseArray<FElement>::SparseArrayIterator It{ Elements, false }; It; ++It) {
                HashElement(FSetElementId(It.GetIndex()), *It);
            }
        }
    }

    FSetElementId* GetHash() const {
        return Hash ? Hash : &InlineHash;
    }

    void ResizeHash(int const PreviousNumElements, int const NumElements) const {
        int const previousNumBytes = PreviousNumElements * sizeof(FSetElementId);

        if (NumElements <= 1) {
            if (Hash) {
                std::memcpy(&InlineHash, Hash, previousNumBytes);
                sdkFree(Hash);
            }
        } else {
            if (Hash == nullptr) {
                Hash = sdkMallocTyped<FSetElementId>(NumElements);
                std::memcpy(Hash, &InlineHash, previousNumBytes);
            } else {
                Hash = sdkReallocTyped<FSetElementId>(Hash, NumElements);
            }
        }
    }

    bool ConditionalRehash(int const NumHashedElements) const {
        int const DesiredHashSize = GetNumberOfHashBuckets(NumHashedElements);

        if (NumHashedElements > 0 && (!HashSize || HashSize < DesiredHashSize)) {
            HashSize = DesiredHashSize;
            Rehash();
            return true;
        }

        return false;
    }

    FSetElementId FindId(KeyType const InKey) const {
        if (HashSize) {
            FSetElementId elementId = GetTypedHash(TKeyFuncs::GetKeyHash(InKey));
            for (; elementId.IsValidId(); elementId = Elements(elementId).HashNextId) {
                auto const ElementKey = TKeyFuncs::GetKey(Elements(elementId).Value);
                if (TKeyFuncs::Matches(ElementKey, InKey)) {
                    return elementId;
                }
            }
        }

        return FSetElementId();
    }
};


// ! TMap and dependencies.
// ========================================

template<typename TKey, typename TValue>
class TMap final {

    class FPair final {
    public:
        TKey Key;
        TValue Value;

        FPair(TKey& InKey, TValue& InValue) : Key(InKey), Value(InValue) {}
        FPair(FPair const& Other) : Key(Other.Key), Value(Other.Value) {}
    };

    struct KeyFuncs {
        typedef typename TKey KeyType;
        enum { AllowDuplicateKeys = false };

        static const KeyType& GetKey(FPair const& Element) { return Element.Key; }
        static bool Matches(KeyType const& Lhs, KeyType const& Rhs) { return Lhs == Rhs; }
        static DWORD GetKeyHash(KeyType const Element) { return GetTypeHash(Element); }
    };

    TSet<FPair, KeyFuncs> Pairs;

public:

    TValue* Set(TKey& Key, TValue& Value) {
        FPair Pair(Key, Value);
        FSetElementId const Id = Pairs.Add(Pair);
        return &Pairs(Id).Value;
    }

    TValue* Find(TKey&& Key) {
        FPair* const Pair = Pairs.Find(Key);
        return Pair ? &Pair->Value : nullptr;
    }

    TValue const* Find(TKey&& Key) const {
        FPair const* const Pair = Pairs.Find(Key);
        return Pair ? &Pair->Value : nullptr;
    }

    // Iterator definition.
    // ----------------------------------------

    class ValueIterator final {
    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = FPair;
        using difference_type = ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using reference = value_type&;
        using const_reference = value_type const&;

    private:

        friend class TMap<TKey, TValue>;
        using InnerArray = TSparseArray<typename TSet<typename FPair, typename KeyFuncs>::FElement>;
        InnerArray::SparseArrayIterator Inner;

        ValueIterator(InnerArray const& Array, bool const bAtEnd)
            : Inner{ Array, bAtEnd } { }

    public:

        ValueIterator& operator++() {
            ++Inner;
            return *this;
        }

        ValueIterator operator++(int) {
            ValueIterator const Prev = *this;
            ++(*this);
            return Prev;
        }

        bool operator==(ValueIterator const Other) const {
            return Inner == Other.Inner;
        }

        bool operator!=(ValueIterator const Other) const {
            return Inner != Other.Inner;
        }

        const_reference operator*() {
            return (*Inner).Value;
        }

        const_reference operator*() const {
            return (*Inner).Value;
        }

        int GetIndex() const {
            return Inner.GetIndex();
        }

        operator bool() const {
            return static_cast<bool>(Inner);
        }

    };

    auto begin() { return ValueIterator{ Pairs.Elements, false }; }
    auto begin() const { return ValueIterator{ Pairs.Elements, false }; }
    auto cbegin() const { return ValueIterator{ Pairs.Elements, false }; }

    auto end() { return ValueIterator{ Pairs.Elements, true }; }
    auto end() const { return ValueIterator{ Pairs.Elements, true }; }
    auto cend() const { return ValueIterator{ Pairs.Elements, true }; }

};

static_assert(sizeof(TMap<unsigned char, char>) == 72);
static_assert(sizeof(TMap<unsigned long long, void*>) == 72);


// ! Container hash functions.
// ========================================

DWORD GetTypeHash(INT8 Value) noexcept;
DWORD GetTypeHash(UINT8 Value) noexcept;
DWORD GetTypeHash(INT16 Value) noexcept;
DWORD GetTypeHash(UINT16 Value) noexcept;
DWORD GetTypeHash(INT Value) noexcept;
DWORD GetTypeHash(UINT Value) noexcept;

DWORD GetTypeHash(WCHAR* Value) noexcept;
DWORD GetTypeHash(WCHAR const* Value) noexcept;

DWORD GetTypeHash(void* Value) noexcept;
DWORD GetTypeHash(void const* Value) noexcept;

// FString and SFXName provide their own hash functions.

struct FGuid;
DWORD GetTypeHash(FGuid& Value) noexcept;
DWORD GetTypeHash(FGuid const& Value) noexcept;

template<typename T>
DWORD GetTypeHash(T* const Value) noexcept {
    return GetTypeHash(reinterpret_cast<void*>(Value));
}

template<typename T>
DWORD GetTypeHash(T const* const Value) noexcept {
    return GetTypeHash(reinterpret_cast<void const*>(Value));
}

#pragma pack(pop)
