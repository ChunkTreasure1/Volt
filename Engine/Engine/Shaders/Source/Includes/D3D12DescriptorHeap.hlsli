#pragma once

#define DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, valueType, handleName, validationType, offset) \
    resourceType<valueType> operator[](handleName<valueType> identifier) \
    { \
        ValidateResourceAccess(GetHandleType(identifier.handle + offset), validationType); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(identifier.handle + offset))]; \
    } \

#define DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(resourceType, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float2, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float3, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float4, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint2, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint3, resourceType##Handle, validationType, offset) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint4, resourceType##Handle, validationType, offset) \


#define DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, valueType, handleName, validationType, offset) \
    resourceType<valueType> operator[](handleName<valueType> identifier) \
    { \
        ValidateResourceAccess(GetHandleType(identifier.handle + offset), validationType); \
        return ResourceDescriptorHeap[GetHandle(identifier.handle + offset)]; \
    } \

#define DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(resourceType, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float2, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float3, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, float4, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint2, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint3, resourceType##Handle, validationType, offset) \
    DEFINE_UNIFORM_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, uint4, resourceType##Handle, validationType, offset) \

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

        WriteRuntimeError(error, ResourceDescriptorHeap[u_pushConstantData.shaderValidationBuffer]);
    }
}

#else

void ValidateResourceAccess(uint handleType, uint shouldBeType)
{}

#endif

struct D3D12ResourceDescriptorHeapInternal
{
    ByteAddressBuffer operator[](BufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::BUFFER);
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))];
    }   

    RWByteAddressBuffer operator[](RWBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::RW_BUFFER);
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle + 1))];
    }

    ByteAddressBuffer operator[](UniformBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::UNIFORM_BUFFER);
        return ResourceDescriptorHeap[GetHandle(handle.handle)];
    }
    
    SamplerState operator[](SamplerStateHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::SAMPLER_STATE);
        return SamplerDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))];
    }

    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture1D, ResourceType::TEXTURE_1D, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D, ResourceType::TEXTURE_2D, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture3D, ResourceType::TEXTURE_3D, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(TextureCube, ResourceType::TEXTURE_CUBE, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture1D, ResourceType::RW_TEXTURE_1D, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D, ResourceType::RW_TEXTURE_2D, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture3D, ResourceType::RW_TEXTURE_3D, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2DArray, ResourceType::RW_TEXTURE_2D_ARRAY, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2DArray, ResourceType::TEXTURE_2D_ARRAY, 0)
};

struct D3D12UniformResourceDescriptorHeapInternal
{
    ByteAddressBuffer operator[](BufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::BUFFER);
        return ResourceDescriptorHeap[GetHandle(handle.handle)];
    }   

    RWByteAddressBuffer operator[](RWBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::RW_BUFFER);
        return ResourceDescriptorHeap[GetHandle(handle.handle + 1)];
    }

    ByteAddressBuffer operator[](UniformBufferHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::UNIFORM_BUFFER);
        return ResourceDescriptorHeap[GetHandle(handle.handle)];
    }
    
    SamplerState operator[](SamplerStateHandle handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::SAMPLER_STATE);
        return SamplerDescriptorHeap[GetHandle(handle.handle)];
    }

    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture1D, ResourceType::TEXTURE_1D, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D, ResourceType::TEXTURE_2D, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture3D, ResourceType::TEXTURE_3D, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(TextureCube, ResourceType::TEXTURE_CUBE, 0)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture1D, ResourceType::RW_TEXTURE_1D, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D, ResourceType::RW_TEXTURE_2D, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture3D, ResourceType::RW_TEXTURE_3D, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2DArray, ResourceType::RW_TEXTURE_2D_ARRAY, 1)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2DArray, ResourceType::TEXTURE_2D_ARRAY, 0)
};

static D3D12ResourceDescriptorHeapInternal g_descriptorHeap;
static D3D12UniformResourceDescriptorHeapInternal g_uniformDescriptorHeap;

#define DESCRIPTOR_HEAP(handleType, handle) g_descriptorHeap[(handleType)handle]
#define UNIFORM_DESCRIPTOR_HEAP(handleType, handle) g_uniformDescriptorHeap[(handleType)handle]

#ifndef NO_RENDERGRAPH
template<typename T>
T GetConstants()
{
    ByteAddressBuffer constantsBuffer = ResourceDescriptorHeap[u_pushConstantData.constantsBufferIndex];
    return constantsBuffer.Load<T>(u_pushConstantData.constantsOffset);
}
#endif