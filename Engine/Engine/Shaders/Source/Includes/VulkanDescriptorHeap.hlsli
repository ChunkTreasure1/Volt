#pragma once

#include "ResourceType.hlsli"

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

SamplerState u_SamplerState[] : register(s10, space0);

//#ifdef ENABLE_RUNTIME_VALIDATION
//
//#include "ShaderRuntimeValidator.hlsli"
//
//void ValidateResourceAccess(uint handleType, uint shouldBeType)
//{
//    // We skip the type "INVALID" as this can mean that the resource hasn't been marked with any type
//    if (handleType != shouldBeType && handleType != ResourceType::INVALID)
//    {
//        RuntimeValidationError error;
//        error.Initialize();
//        error.errorType = RuntimeErrorType::INVALID_RESOURCE_HANDLE_TYPE;
//        error.userdata0 = handleType;
//        error.userdata1 = shouldBeType;
//
//        WriteRuntimeError(error, u_RWByteAddressBuffer[u_renderGraphConstants.shaderValidationBuffer]);
//    }
//}

//#else

void ValidateResourceAccess(uint handleType, uint shouldBeType)
{}

//#endif

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
        return u_ByteAddressBuffer[GetHandle(handle.handle)];
    }
    
    SamplerState operator[](SamplerStateHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::SAMPLER_STATE);
        return u_SamplerState[NonUniformResourceIndex(GetHandle(handle.handle))];
    }

    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture1D, ResourceType::TEXTURE_1D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D, ResourceType::TEXTURE_2D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture3D, ResourceType::TEXTURE_3D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(TextureCube, ResourceType::TEXTURE_CUBE)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture1D, ResourceType::RW_TEXTURE_1D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D, ResourceType::RW_TEXTURE_2D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture3D, ResourceType::RW_TEXTURE_3D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2DArray, ResourceType::RW_TEXTURE_2D_ARRAY)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2DArray, ResourceType::TEXTURE_2D_ARRAY)
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
        return u_ByteAddressBuffer[GetHandle(handle.handle)];
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
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2DArray, ResourceType::TEXTURE_2D_ARRAY)
};

static VulkanResourceDescriptorHeapInternal g_descriptorHeap;
static VulkanUniformResourceDescriptorHeapInternal g_uniformDescriptorHeap;

#define DESCRIPTOR_HEAP(handleType, handle) g_descriptorHeap[(handleType)handle]
#define UNIFORM_DESCRIPTOR_HEAP(handleType, handle) g_uniformDescriptorHeap[(handleType)handle]

#ifndef NO_RENDERGRAPH
template<typename T>
T GetConstants()
{
    return u_ByteAddressBuffer[u_renderGraphConstants.constantsBufferIndex].Load<T>(u_renderGraphConstants.constantsOffset);
}
#endif