#pragma once

namespace vt
{
    template<typename T>
    struct Tex2D
    {
        ResourceHandle handle;

        T Load(in int3 location)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            return texture.Load(location);
        }

        T Sample(in TextureSampler samplerState, in float2 location)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            return texture.Sample(samplerState.Get(), location);
        }

        T Sample(in TextureSampler samplerState, in float2 location, in int2 offset)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            return texture.Sample(samplerState.Get(), location, offset);
        }

        T SampleLevel(in TextureSampler samplerState, in float2 location, in float lod)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            return texture.SampleLevel(samplerState.Get(), location, lod);
        }

        T SampleGrad(in TextureSampler samplerState, in float2 location, in float2 ddx, in float2 ddy)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            return texture.SampleGrad(samplerState.Get(), location, ddx, ddy);
        }

        void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint numberOfLevels)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            texture.GetDimensions(mipLevel, width, height, numberOfLevels);
        }

        void GetDimensions(out uint width, out uint height)
        {
            Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
            texture.GetDimensions(width, height);
        }

        Texture2D<T> Get()
        {
            return DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        }
    };

    template<typename T>
    struct RWTex2D
    {
        ResourceHandle handle;
        
        T Load(in int3 location)
        {
            RWTexture2D<T> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
            return texture.Load(location);
        }

        void Store(in uint2 location, T value)
        {
            RWTexture2D<T> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
            texture[location] = value;
        }

        void GetDimensions(out uint width, out uint height)
        {
            RWTexture2D<T> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
            texture.GetDimensions(width, height);
        }

        RWTexture2D<T> Get()
        {
            return DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        }
    };
}