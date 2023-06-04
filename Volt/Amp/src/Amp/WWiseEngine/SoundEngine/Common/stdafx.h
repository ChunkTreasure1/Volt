/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Copyright (c) 2022 Audiokinetic Inc.
*******************************************************************************/

// stdafx.h
/// \file 
/// Precompiled Header File for the Integration Demo application.

#pragma once

////////////////////////////////////////////
// Helper Includes
////////////////////////////////////////////

#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

#define DIRECTINPUT_VERSION 0x0800
#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf _snprintf
#endif
#include <windows.h>
#include <wchar.h>
