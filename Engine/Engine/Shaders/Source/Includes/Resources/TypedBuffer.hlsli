#pragma once

namespace vt
{
    template<typename T>
    struct UniformTypedBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
            
            uint tempDimension;
            buffer.GetDimensions(tempDimension);
            
            dimension = tempDimension / sizeof(T);
        }
        
        T Load(in uint index)
        {
            ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
            const uint address = index * sizeof(T);
    
            return buffer.Load<T>(address);
        }
        
        T operator[](in uint index)
        {
            return Load(index);
        }
    };
    
    template<typename T>
    struct UniformRWTypedBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
            
            uint tempDimension;
            buffer.GetDimensions(tempDimension);
            
            dimension = tempDimension / sizeof(T);
        }
        
        T Load(in uint index)
        {
            RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
    
            return buffer.Load<T>(address);
        }
      
        void Store(in uint index, T value)
        {
            RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
            
            buffer.Store<T>(address, value);
        }
        
        void InterlockedAdd(in uint index, uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
            
            buffer.InterlockedAdd(address, value, originalValue);
        }
        
        void InterlockedAdd(in uint index, uint value)
        {
            RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
            
            uint tempVal;
            buffer.InterlockedAdd(address, value, tempVal);
        }
    };
    
    template<typename T>
    struct TypedBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            
            uint tempDimension;
            buffer.GetDimensions(tempDimension);
            
            dimension = tempDimension / sizeof(T);
        }
        
        T Load(in uint index)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            const uint address = index * sizeof(T);
    
            return buffer.Load<T>(address);
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
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            
            uint tempDimension;
            buffer.GetDimensions(tempDimension);
            
            dimension = tempDimension / sizeof(T);
        }
        
        T Load(in uint index)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
    
            return buffer.Load<T>(address);
        }
      
        void Store(in uint index, T value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
            
            buffer.Store<T>(address, value);
        }
        
        void InterlockedAdd(in uint index, uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
            
            buffer.InterlockedAdd(address, value, originalValue);
        }
        
        void InterlockedAdd(in uint index, uint value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            const uint address = index * sizeof(T);
            
            uint tempVal;
            buffer.InterlockedAdd(address, value, tempVal);
        }
    };
}