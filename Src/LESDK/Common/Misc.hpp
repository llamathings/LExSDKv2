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

#pragma pack(pop)


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
            AppendObjectName(InObject->Class, OutString, SFXName::k_formatBasic);
            OutString.Append(L" ");

            if (InObject->Outer != nullptr) {
                if (InObject->Outer->Outer != nullptr) {
                    AppendObjectName(InObject->Outer->Outer, OutString, SFXName::k_formatBasic);
                    OutString.Append(L".");
                }

                AppendObjectName(InObject->Outer, OutString, SFXName::k_formatBasic);
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
                    AppendObjectName(InObject->Outer->Outer, OutString, SFXName::k_formatBasic);
                    OutString.Append(L".");
                }
                AppendObjectName(InObject->Outer, OutString, SFXName::k_formatBasic);
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
