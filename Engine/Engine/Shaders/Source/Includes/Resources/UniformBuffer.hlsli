#pragma once

namespace vt
{
    template<typename T>
    struct UniformBuffer
    {
        ResourceHandle handle;
        
        T Load()
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(UniformBufferHandle, handle);
            return buffer.Load<T>(0);
        }
    };
}