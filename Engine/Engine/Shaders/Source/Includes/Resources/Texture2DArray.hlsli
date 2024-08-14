#pragma once

namespace vt
{
    template<typename T>
    struct UniformTex2DArray
    {
        ResourceHandle handle;
        
        T Sample(in TextureSampler samplerState, in float3 location, in int2 offset)
        {
            Texture2DArray<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DArrayHandle<T>, handle);
            return texture.Sample(samplerState.Get(), location, offset);
        }

        T SampleLevel(in TextureSampler samplerState, in float3 location, in float lod)
        {
            Texture2DArray<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DArrayHandle<T>, handle);
            return texture.SampleLevel(samplerState.Get(), location, lod);
        }
    };

    template<typename T>
    struct UniformRWTex2DArray
    {
        ResourceHandle handle;

        T Load(in int4 location)
        {
            RWTexture2DArray<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
            return texture.Load(location);
        }

        void Store(in uint3 location, T value)
        {
            RWTexture2DArray<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
            texture[location] = value;
        }
    };

    template<typename T>
    struct Tex2DArray
    {
        ResourceHandle handle;
        
        T Sample(in TextureSampler samplerState, in float3 location, in int2 offset)
        {
            Texture2DArray<T> texture = DESCRIPTOR_HEAP(Texture2DArrayHandle<T>, handle);
            return texture.Sample(samplerState.Get(), location, offset);
        }

        T SampleLevel(in TextureSampler samplerState, in float3 location, in float lod)
        {
            Texture2DArray<T> texture = DESCRIPTOR_HEAP(Texture2DArrayHandle<T>, handle);
            return texture.SampleLevel(samplerState.Get(), location, lod);
        }
    };

    template<typename T>
    struct RWTex2DArray
    {
        ResourceHandle handle;

        T Load(in int4 location)
        {
            RWTexture2DArray<T> texture = DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
            return texture.Load(location);
        }

        void Store(in uint3 location, T value)
        {
            RWTexture2DArray<T> texture = DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
            texture[location] = value;
        }
    };
}