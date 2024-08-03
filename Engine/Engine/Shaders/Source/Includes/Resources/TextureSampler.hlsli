#pragma once

namespace vt
{
    struct TextureSampler
    {
        ResourceHandle handle;
        
        SamplerState Get()
        {
            return UNIFORM_DESCRIPTOR_HEAP(SamplerStateHandle, handle);
        }
    };
}