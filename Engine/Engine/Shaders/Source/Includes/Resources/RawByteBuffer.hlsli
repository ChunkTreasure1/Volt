#pragma once

namespace vt
{
    struct RawByteBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            buffer.GetDimensions(dimension);
        }
        
        uint Load(in uint address)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            return buffer.Load(address);
        }
        
        uint2 Load2(in uint address)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            return buffer.Load2(address);
        }
        
        uint3 Load3(in uint address)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            return buffer.Load3(address);
        }
        
        uint4 Load4(in uint address)
        {
            ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
            return buffer.Load4(address);
        }
    };
    
    struct RWRawByteBuffer
    {
        ResourceHandle handle;
        
        void GetDimensions(out uint dimension)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.GetDimensions(dimension);
        }
        
        uint Load(in uint address)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            return buffer.Load(address);
        }
        
        uint2 Load2(in uint address)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            return buffer.Load2(address);
        }
        
        uint3 Load3(in uint address)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            return buffer.Load3(address);
        }
        
        uint4 Load4(in uint address)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            return buffer.Load4(address);
        }
        
        void Store(in uint address, in uint value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.Store(address, value);
        }
        
        void Store2(in uint address, in uint2 value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.Store2(address, value);
        }
        
        void Store3(in uint address, in uint3 value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.Store3(address, value);
        }
        
        void Store4(in uint address, in uint4 value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.Store4(address, value);
        }
        
        void InterlockedAdd(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedAdd(dest, value, originalValue);
        }
        
        void InterlockedAnd(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedAnd(dest, value, originalValue);
        }
        
        void InterlockedCompareExchange(in uint dest, in uint compareValue, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedCompareExchange(dest, compareValue, value, originalValue);
        }
        
        void InterlockedCompareStore(in uint dest, in uint compareValue, in uint value)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedCompareStore(dest, compareValue, value);
        }
        
        void InterlockedExchange(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedExchange(dest, value, originalValue);
        }
        
        void InterlockedMax(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedMax(dest, value, originalValue);
        }
        
        void InterlockedMin(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedMin(dest, value, originalValue);
        }
        
        void InterlockedOr(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedOr(dest, value, originalValue);
        }
        
        void InterlockedXor(in uint dest, in uint value, out uint originalValue)
        {
            RWByteAddressBuffer buffer = DESCRIPTOR_HEAP(RWBufferHandle, handle);
            buffer.InterlockedXor(dest, value, originalValue);
        }
    };
}

