#pragma once

#include "Defines.hlsli"

#define DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(textureType, binding, space) \
    textureType<float> u_##textureType##float[] : register(binding, space); \
    textureType<float2> u_##textureType##float2[] : register(binding, space); \
    textureType<float3> u_##textureType##float3[] : register(binding, space); \
    textureType<float4> u_##textureType##float4[] : register(binding, space); \
    textureType<uint> u_##textureType##uint[] : register(binding, space); \
    textureType<uint2> u_##textureType##uint2[] : register(binding, space); \
    textureType<uint3> u_##textureType##uint3[] : register(binding, space); \
    textureType<uint4> u_##textureType##uint4[] : register(binding, space); \

#define DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, registerName, valueType, handleName, validationType) \
    resourceType<valueType> operator[](handleName<valueType> identifier) \
    { \
        ValidateResourceAccess(GetHandleType(identifier.handle), validationType); \
        return registerName##valueType[NonUniformResourceIndex(GetHandle(identifier.handle))]; \
    } \

#define DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(resourceType, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float2, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float3, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float4, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint2, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint3, resourceType##Handle, validationType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint4, resourceType##Handle, validationType) \

#define DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, registerName, valueType, handleName,  validationType) \
    resourceType<valueType> operator[](handleName<valueType> identifier) \
    { \
        ValidateResourceAccess(GetHandleType(identifier.handle), validationType); \
        return registerName##valueType[GetHandle(identifier.handle)]; \
    } \

#define DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(resourceType, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float2, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float3, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, float4, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint2, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint3, resourceType##Handle, validationType) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType, uint4, resourceType##Handle, validationType) \

DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture1D, t0, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture2D, t1, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture3D, t2, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(TextureCube, t3, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture1D, u4, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture2D, u5, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture3D, u6, space0)

DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture2DArray, u11, space0)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture2DArray, t12, space0)

ByteAddressBuffer u_ByteAddressBuffer[] : register(t7, space0);
RWByteAddressBuffer u_RWByteAddressBuffer[] : register(u8, space0);

ByteAddressBuffer u_UniformBuffer[] : register(t9, space0);
SamplerState u_SamplerState[] : register(s10, space0);

#ifndef NO_RENDERGRAPH

struct PushContantData
{
    uint constantsBufferIndex;
    uint shaderValidationBuffer;
    uint constantsOffset;
};

PUSH_CONSTANT(PushContantData, u_pushConstantData);

template<typename T>
T GetConstants()
{
    return u_ByteAddressBuffer[u_pushConstantData.constantsBufferIndex].Load<T>(u_pushConstantData.constantsOffset);
}

#endif

// Resource Handle Layout:
// 8 bit resource type
// 24 bit handle

uint GetHandle(uint resourceHandle)
{
    return resourceHandle & 0xFFFFFF;
}

uint GetHandleType(uint resourceHandle)
{
    return (resourceHandle >> 24) & 0xFF;
}

struct BufferHandle
{
    uint handle;
};

struct RWBufferHandle
{
    uint handle;
};

struct UniformBufferHandle
{
    uint handle;
};

template<typename T>
struct Texture1DHandle
{
    uint handle;
};

template<typename T>
struct Texture2DHandle
{
    uint handle;
};

template<typename T>
struct Texture3DHandle
{
    uint handle;
};

template<typename T>
struct TextureCubeHandle
{
    uint handle;
};

template<typename T>
struct RWTexture1DHandle
{
    uint handle;
};

template<typename T>
struct RWTexture2DHandle
{
    uint handle;
};

template<typename T>
struct RWTexture3DHandle
{
    uint handle;
};

template<typename T>
struct RWTexture2DArrayHandle
{
    uint handle;
};

struct SamplerStateHandle
{
    uint handle;
};

namespace ResourceType
{
    static const uint INVALID = 0;
    static const uint BUFFER = 1;
    static const uint RW_BUFFER = 2;
    static const uint UNIFORM_BUFFER = 3;
    static const uint TEXTURE_1D = 4;
    static const uint TEXTURE_2D = 5;
    static const uint TEXTURE_3D = 6;
    static const uint TEXTURE_CUBE = 7;
    static const uint RW_TEXTURE_1D = 8;
    static const uint RW_TEXTURE_2D = 9;
    static const uint RW_TEXTURE_3D = 10;
    static const uint RW_TEXTURE_2D_ARRAY = 11;
    static const uint SAMPLER_STATE = 12;
}

#ifdef ENABLE_RUNTIME_VALIDATION

#include "ShaderRuntimeValidator.hlsli"

void ValidateResourceAccess(uint handleType, uint shouldBeType)
{
    // We skip the type "INVALID" as this can mean that the resource hasn't been marked with any type
    if (handleType != shouldBeType && handleType != ResourceType::INVALID)
    {
        RuntimeValidationError error;
        error.Initialize();
        error.errorType = RuntimeErrorType::INVALID_RESOURCE_HANDLE_TYPE;
        error.userdata0 = handleType;
        error.userdata1 = shouldBeType;

        WriteRuntimeError(error, u_RWByteAddressBuffer[u_pushConstantData.shaderValidationBuffer]);
    }
}

#else

void ValidateResourceAccess(uint handleType, uint shouldBeType)
{}

#endif

// #TODO_Ivar: Add ability to access resources uniformly
struct VulkanResourceDescriptorHeapInternal
{
    ByteAddressBuffer operator[](BufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::BUFFER);
        return u_ByteAddressBuffer[NonUniformResourceIndex(GetHandle(handle.handle))];
    }   

    RWByteAddressBuffer operator[](RWBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::RW_BUFFER);
        return u_RWByteAddressBuffer[NonUniformResourceIndex(GetHandle(handle.handle))];
    }

    ByteAddressBuffer operator[](UniformBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::UNIFORM_BUFFER);
        return u_UniformBuffer[GetHandle(handle.handle)];
    }
    
    SamplerState operator[](SamplerStateHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::SAMPLER_STATE);
        return u_SamplerState[GetHandle(handle.handle)];
    }

    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture1D, ResourceType::TEXTURE_1D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D, ResourceType::TEXTURE_2D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture3D, ResourceType::TEXTURE_3D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(TextureCube, ResourceType::TEXTURE_CUBE)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture1D, ResourceType::RW_TEXTURE_1D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D, ResourceType::RW_TEXTURE_2D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture3D, ResourceType::RW_TEXTURE_3D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2DArray, ResourceType::RW_TEXTURE_2D_ARRAY)
};

struct VulkanUniformResourceDescriptorHeapInternal
{
    ByteAddressBuffer operator[](BufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::BUFFER);
        return u_ByteAddressBuffer[GetHandle(handle.handle)];
    }

    RWByteAddressBuffer operator[](RWBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::RW_BUFFER);
        return u_RWByteAddressBuffer[GetHandle(handle.handle)];
    }

    ByteAddressBuffer operator[](UniformBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::UNIFORM_BUFFER);
        return u_UniformBuffer[GetHandle(handle.handle)];
    }
    
    SamplerState operator[](SamplerStateHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::SAMPLER_STATE);
        return u_SamplerState[GetHandle(handle.handle)];
    }

    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture1D, ResourceType::TEXTURE_1D)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D, ResourceType::TEXTURE_2D)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture3D, ResourceType::TEXTURE_3D)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(TextureCube, ResourceType::TEXTURE_CUBE)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture1D, ResourceType::RW_TEXTURE_1D)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D, ResourceType::RW_TEXTURE_2D)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture3D, ResourceType::RW_TEXTURE_3D)
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2DArray, ResourceType::RW_TEXTURE_2D_ARRAY)
};

static VulkanResourceDescriptorHeapInternal g_descriptorHeap;
static VulkanUniformResourceDescriptorHeapInternal g_uniformDescriptorHeap;

#define DESCRIPTOR_HEAP(handleType, handle) g_descriptorHeap[(handleType)handle]
#define UNIFORM_DESCRIPTOR_HEAP(handleType, handle) g_uniformDescriptorHeap[(handleType)handle]

struct ResourceHandle
{
    uint handle;
};

struct TextureSampler
{
    ResourceHandle handle;
    
    SamplerState Get()
    {
        return UNIFORM_DESCRIPTOR_HEAP(SamplerStateHandle, handle);
    }
};

struct UniformRawByteBuffer
{
    ResourceHandle handle;
    
    void GetDimensions(out uint dimension)
    {
        ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
        buffer.GetDimensions(dimension);
    }
    
    uint Load(in uint address)
    {
        ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
        return buffer.Load(address);
    }
    
    uint2 Load2(in uint address)
    {
        ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
        return buffer.Load2(address);
    }
    
    uint3 Load3(in uint address)
    {
        ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
        return buffer.Load3(address);
    }
    
    uint4 Load4(in uint address)
    {
        ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(BufferHandle, handle);
        return buffer.Load4(address);
    }
};

struct RWUniformRawByteBuffer
{
    ResourceHandle handle;
    
    void GetDimensions(out uint dimension)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.GetDimensions(dimension);
    }
    
    uint Load(in uint address)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        return buffer.Load(address);
    }
    
    uint2 Load2(in uint address)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        return buffer.Load2(address);
    }
    
    uint3 Load3(in uint address)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        return buffer.Load3(address);
    }
    
    uint4 Load4(in uint address)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        return buffer.Load4(address);
    }
    
    void Store(in uint address, in uint value)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.Store(address, value);
    }
    
    void Store2(in uint address, in uint2 value)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.Store2(address, value);
    }
    
    void Store3(in uint address, in uint3 value)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.Store3(address, value);
    }
    
    void Store4(in uint address, in uint4 value)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.Store4(address, value);
    }
    
    void InterlockedAdd(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedAdd(dest, value, originalValue);
    }
    
    void InterlockedAnd(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedAnd(dest, value, originalValue);
    }
    
    void InterlockedCompareExchange(in uint dest, in uint compareValue, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedCompareExchange(dest, compareValue, value, originalValue);
    }
    
    void InterlockedCompareStore(in uint dest, in uint compareValue, in uint value)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedCompareStore(dest, compareValue, value);
    }
    
    void InterlockedExchange(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedExchange(dest, value, originalValue);
    }
    
    void InterlockedMax(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedMax(dest, value, originalValue);
    }
    
    void InterlockedMin(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedMin(dest, value, originalValue);
    }
    
    void InterlockedOr(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedOr(dest, value, originalValue);
    }
    
    void InterlockedXor(in uint dest, in uint value, out uint originalValue)
    {
        RWByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(RWBufferHandle, handle);
        buffer.InterlockedXor(dest, value, originalValue);
    }
};

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

template<typename T>
struct UniformBuffer
{
    ResourceHandle handle;
    
    T Load()
    {
        ByteAddressBuffer buffer = UNIFORM_DESCRIPTOR_HEAP(UniformBufferHandle, handle);
        return buffer.Load<T>(0);
    }
};

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
struct RWUniformTypedBuffer
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

template<typename T>
struct UniformTexture
{
    ResourceHandle handle;
    
    T Load1D(in int2 location)
    {
        Texture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load2D(in int3 location)
    {
        Texture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load3D(in int4 location)
    {
        Texture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Sample1D(in TextureSampler samplerState, in float location)
    {
        Texture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T Sample2D(in TextureSampler samplerState, in float2 location)
    {
        Texture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T Sample3D(in TextureSampler samplerState, in float3 location)
    {
        Texture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T SampleCube(in TextureSampler samplerState, in float3 location)
    {
        TextureCube<T> texture = UNIFORM_DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T SampleLevel1D(in TextureSampler samplerState, in float location, in float lod)
    {
        Texture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleLevel2D(in TextureSampler samplerState, in float2 location, in float lod)
    {
        Texture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleLevel3D(in TextureSampler samplerState, in float3 location, in float lod)
    {
        Texture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleLevelCube(in TextureSampler samplerState, in float3 location, in float lod)
    {
        TextureCube<T> texture = UNIFORM_DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleGrad2D(in TextureSampler samplerState, in float2 location, in float2 ddx, in float2 ddy)
    {
        Texture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.SampleGrad(samplerState.Get(), location, ddx, ddy);
    }
    
    void GetDimensions(in uint mipLevel, out uint width, out uint numberOfLevels)
    {
        Texture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, numberOfLevels);
    }
    
    void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint numberOfLevels)
    {
        Texture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, height, numberOfLevels);
    }
    
    void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint depth, out uint numberOfLevels)
    {
        Texture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, height, depth, numberOfLevels);
    }
    
    void GetDimensions(out uint width)
    {
        Texture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        texture.GetDimensions(width);
    }
    
    void GetDimensions(out uint width, out uint height)
    {
        Texture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        texture.GetDimensions(width, height);
    }
    
    void GetDimensions(out uint width, out uint height, out uint depth)
    {
        Texture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        texture.GetDimensions(width, height, depth);
    }
    
    void GetDimensionsCube(in uint mipLevel, out uint width, out uint height, out uint numberOfLevels)
    {
        TextureCube<T> texture = UNIFORM_DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, height, numberOfLevels);
    }
    
    Texture1D<T> Get1D()
    {
        return UNIFORM_DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
    }
    
    Texture2D<T> Get2D()
    {
        return UNIFORM_DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
    }
    
    Texture3D<T> Get3D()
    {
        return UNIFORM_DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
    }
    
    TextureCube<T> GetCube()
    {
        return UNIFORM_DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
    }
};

template<typename T>
struct UniformRWTexture
{
    ResourceHandle handle;
    
    T Load1D(in int2 location)
    {
        RWTexture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load2D(in int3 location)
    {
        RWTexture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load3D(in int4 location)
    {
        RWTexture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load2DArray(in int4 location)
    {
        RWTexture2DArray<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
        return texture.Load(location);
    }
    
    void Store1D(in uint location, T value)
    {
        RWTexture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
        texture[location] = value;
    }
    
    void Store2D(in uint2 location, T value)
    {
        RWTexture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        texture[location] = value;
    }
    
    void Store3D(in uint3 location, T value)
    {
        RWTexture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        texture[location] = value;
    }
    
    void Store2DArray(in uint3 location, T value)
    {
        RWTexture2DArray<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
        texture[location] = value;
    }
    
    void GetDimensions(out uint width)
    {
        RWTexture1D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
        texture.GetDimensions(width);
    }
    
    void GetDimensions(out uint width, out uint height)
    {
        RWTexture2D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        texture.GetDimensions(width, height);
    }
    
    void GetDimensions(out uint width, out uint height, out uint depth)
    {
        RWTexture3D<T> texture = UNIFORM_DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        texture.GetDimensions(width, height, depth);
    }
    
    RWTexture1D<T> Get1D()
    {
        return UNIFORM_DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
    }
    
    RWTexture2D<T> Get2D()
    {
        return UNIFORM_DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
    }
    
    RWTexture3D<T> Get3D()
    {
        return UNIFORM_DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
    }
};

template<typename T>
struct TTexture
{
    ResourceHandle handle;
    
    T Load1D(in int2 location)
    {
        Texture1D<T> texture = DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load2D(in int3 location)
    {
        Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load3D(in int4 location)
    {
        Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Sample1D(in TextureSampler samplerState, in float location)
    {
        Texture1D<T> texture = DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T Sample2D(in TextureSampler samplerState, in float2 location)
    {
        Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T Sample3D(in TextureSampler samplerState, in float3 location)
    {
        Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T SampleCube(in TextureSampler samplerState, in float3 location)
    {
        TextureCube<T> texture = DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        return texture.Sample(samplerState.Get(), location);
    }
    
    T SampleLevel1D(in TextureSampler samplerState, in float location, in float lod)
    {
        Texture1D<T> texture = DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleLevel2D(in TextureSampler samplerState, in float2 location, in float lod)
    {
        Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleLevel3D(in TextureSampler samplerState, in float3 location, in float lod)
    {
        Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleLevelCube(in TextureSampler samplerState, in float3 location, in float lod)
    {
        TextureCube<T> texture = DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        return texture.SampleLevel(samplerState.Get(), location, lod);
    }
    
    T SampleGrad2D(in TextureSampler samplerState, in float2 location, in float2 ddx, in float2 ddy)
    {
        Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        return texture.SampleGrad(samplerState.Get(), location, ddx, ddy);
    }
    
    void GetDimensions(in uint mipLevel, out uint width, out uint numberOfLevels)
    {
        Texture1D<T> texture = DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, numberOfLevels);
    }
    
    void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint numberOfLevels)
    {
        Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, height, numberOfLevels);
    }
    
    void GetDimensions(in uint mipLevel, out uint width, out uint height, out uint depth, out uint numberOfLevels)
    {
        Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, height, depth, numberOfLevels);
    }
    
    void GetDimensions(out uint width)
    {
        Texture1D<T> texture = DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
        texture.GetDimensions(width);
    }
    
    void GetDimensions(out uint width, out uint height)
    {
        Texture2D<T> texture = DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
        texture.GetDimensions(width, height);
    }
    
    void GetDimensions(out uint width, out uint height, out uint depth)
    {
        Texture3D<T> texture = DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
        texture.GetDimensions(width, height, depth);
    }
    
    void GetDimensionsCube(in uint mipLevel, out uint width, out uint height, out uint numberOfLevels)
    {
        TextureCube<T> texture = DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
        texture.GetDimensions(mipLevel, width, height, numberOfLevels);
    }
    
    Texture1D<T> Get1D()
    {
        return DESCRIPTOR_HEAP(Texture1DHandle<T>, handle);
    }
    
    Texture2D<T> Get2D()
    {
        return DESCRIPTOR_HEAP(Texture2DHandle<T>, handle);
    }
    
    Texture3D<T> Get3D()
    {
        return DESCRIPTOR_HEAP(Texture3DHandle<T>, handle);
    }
    
    TextureCube<T> GetCube()
    {
        return DESCRIPTOR_HEAP(TextureCubeHandle<T>, handle);
    }
};

template<typename T>
struct RWTexture
{
    ResourceHandle handle;
    
    T Load1D(in int2 location)
    {
        RWTexture1D<T> texture = DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load2D(in int3 location)
    {
        RWTexture2D<T> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load3D(in int4 location)
    {
        RWTexture3D<T> texture = DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        return texture.Load(location);
    }
    
    T Load2DArray(in int4 location)
    {
        RWTexture2DArray<T> texture = DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
        return texture.Load(location);
    }
    
    void Store1D(in uint location, T value)
    {
        RWTexture1D<T> texture = DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
        texture[location] = value;
    }
    
    void Store2D(in uint2 location, T value)
    {
        RWTexture2D<T> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        texture[location] = value;
    }
    
    void Store3D(in uint3 location, T value)
    {
        RWTexture3D<T> texture = DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        texture[location] = value;
    }
    
    void Store2DArray(in uint3 location, T value)
    {
        RWTexture2DArray<T> texture = DESCRIPTOR_HEAP(RWTexture2DArrayHandle<T>, handle);
        texture[location] = value;
    }
    
    void GetDimensions(out uint width)
    {
        RWTexture1D<T> texture = DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
        texture.GetDimensions(width);
    }
    
    void GetDimensions(out uint width, out uint height)
    {
        RWTexture2D<T> texture = DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
        texture.GetDimensions(width, height);
    }
    
    void GetDimensions(out uint width, out uint height, out uint depth)
    {
        RWTexture3D<T> texture = DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
        texture.GetDimensions(width, height, depth);
    }
    
    RWTexture1D<T> Get1D()
    {
        return DESCRIPTOR_HEAP(RWTexture1DHandle<T>, handle);
    }
    
    RWTexture2D<T> Get2D()
    {
        return DESCRIPTOR_HEAP(RWTexture2DHandle<T>, handle);
    }
    
    RWTexture3D<T> Get3D()
    {
        return DESCRIPTOR_HEAP(RWTexture3DHandle<T>, handle);
    }
};

SamplerState GetSampler(uint samplerIndex)
{
    return u_SamplerState[NonUniformResourceIndex(samplerIndex)];
}