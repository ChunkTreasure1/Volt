#pragma once

#include "ResourceType.hlsli"

namespace RuntimeErrorType
{
    static const uint INVALID_ERROR = 0;
    static const uint INVALID_RESOURCE_HANDLE_TYPE = 1;
}

struct RuntimeValidationError
{
    uint errorType;
    uint userdata0;
    uint userdata1;
    uint userdata2;

    void Initialize()
    {
        errorType = RuntimeErrorType::INVALID_ERROR;
        userdata0 = 0;
        userdata1 = 0;
        userdata2 = 0;
    }
};

#ifdef ENABLE_RUNTIME_VALIDATION

void WriteRuntimeError(RuntimeValidationError error, RWByteAddressBuffer buffer)
{
    uint currentIndex;
    buffer.InterlockedAdd(0u, 1u, currentIndex);
    
    // Offset address by 4 bytes, as first uint is error count
    const uint address = currentIndex * sizeof(RuntimeValidationError) + 4;
    buffer.Store<RuntimeValidationError>(address, error);
}

#endif