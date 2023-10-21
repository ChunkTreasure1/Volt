#pragma once

#define DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(textureType, binding, space) \
    textureType<float> u_##textureType##float[] : register(binding, space); \
    textureType<float2> u_##textureType##float2[] : register(binding, space); \
    textureType<float3> u_##textureType##float3[] : register(binding, space); \
    textureType<float4> u_##textureType##float4[] : register(binding, space); \
    textureType<uint> u_##textureType##uint[] : register(binding, space); \
    textureType<uint2> u_##textureType##uint2[] : register(binding, space); \
    textureType<uint3> u_##textureType##uint3[] : register(binding, space); \
    textureType<uint4> u_##textureType##uint4[] : register(binding, space); \

#define DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, registerName, valueType, handleName) \
    resourceType<valueType> operator[](handleName<valueType> identifier) \
    { \
        return registerName##valueType[NonUniformResourceIndex(identifier.handle)]; \
    } \

#define DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(resourceType) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##float, float, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##float2, float2, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##float3, float3, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##float4, float4, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##uint, uint, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##uint2, uint2, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##uint3, uint3, resourceType##Handle) \
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL(resourceType, u_##resourceType##uint4, uint4, resourceType##Handle) \


DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture1D, t0, space1)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture2D, t1, space1)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(Texture3D, t2, space1)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(TextureCube, t3, space1)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture1D, t4, space1)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture2D, t5, space1)
DEFINE_TEXTURE_TYPES_AND_FORMATS_SLOTS(RWTexture3D, t6, space1)

ByteAddressBuffer u_ByteAddressBuffer[] : register(t7, space1);

struct BufferHandle
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

struct VulkanResourceDescriptorHeapInternal
{
    ByteAddressBuffer operator[](BufferHandle handle)
    {
        return u_ByteAddressBuffer[NonUniformResourceIndex(handle.handle)];
    }

    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture1D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture2D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(Texture3D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(TextureCube)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture1D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture2D)
    DEFINE_TEXTURE_TYPE_TEMPLATE_SPECIALIZATION_DECL_MULTI(RWTexture3D)
};

static VulkanResourceDescriptorHeapInternal g_descriptorHeap;

#define DESCRIPTOR_HEAP(handleType, handle) g_descriptorHeap[(handleType)handle]

struct ResourceHandle
{
    uint handle;
};

struct RawByteBuffer
{
    ResourceHandle handle;
    
    void GetDimensions(out uint dimension)
    {
        ByteAddressBuffer buffer = DESCRIPTOR_HEAP(BufferHandle, handle);
        buffer.GetDimensions(dimension);
    }
};