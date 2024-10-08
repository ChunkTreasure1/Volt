#pragma once

namespace vt
{
    template<typename T>
    struct Tex3D
    {
        ResourceHandle handle;
        
        T Load(in int4 location)
        {
            Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
            return texture.Load(location);
        }

        T Sample(in TextureSampler samplerState, in float3 location)
        {
            Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
            return texture.Sample(samplerState.Get(), location);
        }

        T SampleLevel(in TextureSampler samplerState, in float3 location, in float lod)
        {
            Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
            return texture.SampleLevel(samplerState.Get(), location, lod);
        }

        void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint depth, out uint numberOfLevels)
        {
            Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
            texture.GetDimensions(mipLevel, width, height, depth, numberOfLevels);
        }

        void GetDimensions(out uint width, out uint height, out uint depth)
        {
            Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
            texture.GetDimensions(width, height, depth);
        }

        Texture3D<T> Get()
        {
            return DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        }
    };

    template<typename T>
    struct RWTex3D
    {
        ResourceHandle handle;
        
        T Load(in int3 location)
        {
            RWTexture3D<T> texture = DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
            return texture.Load(location);
        }

        void Store(in uint3 location, T value)
        {
            RWTexture3D<T> texture = DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
            texture[location] = value;
        }

        void GetDimensions(out uint width, out uint height, out uint depth)
        {
            RWTexture3D<T> texture = DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
            texture.GetDimensions(width, height, depth);
        }

        RWTexture3D<T> Get()
        {
            return DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        }
    };
}