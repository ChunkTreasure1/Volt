#pragma once

namespace vt
{
    struct TextureSampler
    {
        ResourceHandle handle;
        
        SamplerState Get()
        {
            return DESCRIPTOR_HEAP(SamplerStateHandle, handle);
        }
    };
}