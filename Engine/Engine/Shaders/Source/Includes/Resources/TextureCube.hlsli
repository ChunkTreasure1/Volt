#pragma once

namespace vt
{
    template<typename T>
    struct TexCube
    {
        ResourceHandle handle;
        
        T Sample(in TextureSampler samplerState, in float3 location)
        {
            TextureCube<T> texture = DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
            return texture.Sample(samplerState.Get(), location);
        }

        T SampleLevel(in TextureSampler samplerState, in float3 location, in float lod)
        {
            TextureCube<T> texture = DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
            return texture.SampleLevel(samplerState.Get(), location, lod);
        }

        void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint numberOfLevels)
        {
            TextureCube<T> texture = DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
            texture.GetDimensions(mipLevel, width, height, numberOfLevels);
        }

        TextureCube<T> Get()
        {
            return DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        }
    };
}