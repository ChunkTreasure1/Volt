#pragma once

#include "ResourceType.hlsli"

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

        RWByteAddressBuffer errorBuffer = ResourceDescriptorHeap[u_renderGraphConstants.shaderValidationBuffer];
        WriteRuntimeError(error, errorBuffer);
    }
}

#else

void ValidateResourceAccess(uint handleType, uint shouldBeType)
{}

#endif

struct ResourceDescriptorHeapInternal
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

    template<typename T>
    StructuredBuffer<T> operator[](TypedBufferHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::BUFFER);
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))];
    }   

    template<typename T>
    RWStructuredBuffer<T> operator[](RWTypedBufferHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::BUFFER);
        RWStructuredBuffer<T> buffer = ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle + 1))];
        return buffer;
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

    template<typename T>
    Texture1D<T> operator[](Texture1DHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::TEXTURE_1D); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))]; \
    }

    template<typename T>
    Texture2D<T> operator[](Texture2DHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::TEXTURE_2D); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))]; \
    }

    template<typename T>
    Texture3D<T> operator[](Texture3DHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::TEXTURE_3D); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))]; \
    }

    template<typename T>
    Texture2DArray<T> operator[](Texture2DArrayHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::TEXTURE_2D_ARRAY); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))]; \
    }

    template<typename T>
    TextureCube<T> operator[](TextureCubeHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle), ResourceType::TEXTURE_CUBE); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle))]; \
    }

    template<typename T>
    RWTexture1D<T> operator[](RWTexture1DHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::RW_TEXTURE_1D); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle + 1))]; \
    }

    template<typename T>
    RWTexture2D<T> operator[](RWTexture2DHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::RW_TEXTURE_2D); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle + 1))]; \
    }

    template<typename T>
    RWTexture3D<T> operator[](RWTexture3DHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::RW_TEXTURE_3D); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle + 1))]; \
    }

    template<typename T>
    RWTexture2DArray<T> operator[](RWTexture2DArrayHandle<T> handle)
    {
        ValidateResourceAccess(GetHandleType(handle.handle + 1), ResourceType::RW_TEXTURE_2D_ARRAY); \
        return ResourceDescriptorHeap[NonUniformResourceIndex(GetHandle(handle.handle + 1))]; \
    }
};

static ResourceDescriptorHeapInternal g_descriptorHeap;

#define DESCRIPTOR_HEAP(handleType, handle) g_descriptorHeap[(handleType)handle]

template<typename T>
T GetConstants()
{
    ByteAddressBuffer constantsBuffer = ResourceDescriptorHeap[u_renderGraphConstants.constantsBufferIndex];
    return constantsBuffer.Load<T>(u_renderGraphConstants.constantsOffset);
}
