#pragma once

namespace LESDK
{
    // These are defined in Common.cpp
    // Declared here so LEL can use them without including other hpps
    void InitializeConsole();
    void TerminateConsole();

    UINT GetAnsiLengthWide(WCHAR const* InWideStr, UINT InWideLength);
    UINT GetUtf8LengthWide(WCHAR const* InWideStr, UINT InWideLength);

    UINT GetWideLengthAnsi(char const* InAnsiStr, UINT InAnsiLength);
    UINT GetWideLengthUtf8(char const* InUtf8Str, UINT InUtf8Length);

    bool EncodeAnsiFromWide(WCHAR const* InWideStr, UINT InWideLength, char* OutAnsiStr, UINT OutAnsiLength, DWORD* pOutError);
    bool EncodeUtf8FromWide(WCHAR const* InWideStr, UINT InWideLength, char* OutUtf8Str, UINT OutUtf8Length, DWORD* pOutError);

    bool EncodeWideFromAnsi(char const* InAnsiStr, UINT InAnsiLength, WCHAR* OutWideStr, UINT OutWideLength, DWORD* pOutError);
    bool EncodeWideFromUtf8(char const* InUtf8Str, UINT InUtf8Length, WCHAR* OutWideStr, UINT OutWideLength, DWORD* pOutError);
}

#if !defined(SDK_TARGET_LEL)
#include "LESDK/Common/Core.hpp"
#include "LESDK/Common/Frame.hpp"
#include "LESDK/Common/FString.hpp"
#include "LESDK/Common/SFXName.hpp"
#include "LESDK/Common/TArray.hpp"
#include "LESDK/Common/TMap.hpp"

// This header *must* be at the end.
#include "LESDK/Common/Misc.hpp"
#endif
