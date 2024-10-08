#pragma once

#include "Resources.hlsli"

namespace vt
{
    template<typename T>
    void InterlockedAdd(in vt::RWTypedBuffer<T> buffer, in uint index, in uint value, out uint originalValue)
    {
        RWStructuredBuffer<T> structuredBuffer = DESCRIPTOR_HEAP(RWTypedBufferHandle<T>, buffer.handle);
        InterlockedAdd(structuredBuffer[index], value, originalValue);
    }

    template<typename T>
    void InterlockedAdd(in vt::RWTypedBuffer<T> buffer, in uint index, in uint value)
    {
        RWStructuredBuffer<T> structuredBuffer = DESCRIPTOR_HEAP(RWTypedBufferHandle<T>, buffer.handle);

        uint tempVal;
        InterlockedAdd(structuredBuffer[index], value, tempVal);
    }
}