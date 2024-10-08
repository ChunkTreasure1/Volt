#pragma once

namespace vt
{
    template<typename T>
    struct TypedBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            StructuredBuffer<T> buffer = DESCRIPTOR_HEAP(TypedBufferHandle<T>, handle);
            buffer.GetDimensions(dimension);
        }
        
        T Load(in uint index)
        {
            StructuredBuffer<T> buffer = DESCRIPTOR_HEAP(TypedBufferHandle<T>, handle);
            return buffer[index];
        }
        
        T operator[](in uint index)
        {
            return Load(index);
        }
    };
    
    template<typename T>
    struct RWTypedBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            RWStructuredBuffer<T> buffer = DESCRIPTOR_HEAP(RWTypedBufferHandle<T>, handle);
            buffer.GetDimensions(dimension);
        }
        
        T Load(in uint index)
        {
            RWStructuredBuffer<T> buffer = DESCRIPTOR_HEAP(RWTypedBufferHandle<T>, handle);
            return buffer[index];
        }
      
        void Store(in uint index, T value)
        {
            RWStructuredBuffer<T> buffer = DESCRIPTOR_HEAP(RWTypedBufferHandle<T>, handle);
            buffer[index] = value;
        }
    };
}