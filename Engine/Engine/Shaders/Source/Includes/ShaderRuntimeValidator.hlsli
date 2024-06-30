#pragma once

namespace ResourceType
{
    static const uint INVALID = 0;
    static const uint BUFFER = 1;
    static const uint RW_BUFFER = 2;
    static const uint UNIFORM_BUFFER = 3;
    static const uint TEXTURE_1D = 4;
    static const uint TEXTURE_2D = 5;
    static const uint TEXTURE_3D = 6;
    static const uint TEXTURE_CUBE = 7;
    static const uint RW_TEXTURE_1D = 8;
    static const uint RW_TEXTURE_2D = 9;
    static const uint RW_TEXTURE_3D = 10;
    static const uint RW_TEXTURE_2D_ARRAY = 11;
    static const uint SAMPLER_STATE = 12;
    static const uint TEXTURE_2D_ARRAY = 13;
}

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