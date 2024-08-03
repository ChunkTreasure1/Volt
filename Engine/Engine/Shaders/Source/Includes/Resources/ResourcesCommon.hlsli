#pragma once

#ifndef NO_RENDERGRAPH

struct RenderGraphConstants 
{
    uint constantsBufferIndex;
    uint shaderValidationBuffer;
    uint constantsOffset;
};

#define RENDER_GRAPH_CONSTANTS_BINDING b998
ConstantBuffer<RenderGraphConstants> u_renderGraphConstants : register(RENDER_GRAPH_CONSTANTS_BINDING, space1);

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

template<typename T>
struct Texture2DArrayHandle
{
    uint handle;
};

struct SamplerStateHandle
{
    uint handle;
};

struct ResourceHandle
{
    uint handle;
};

#if __VULKAN__
    #include "VulkanDescriptorHeap.hlsli"
#elif __D3D12__
    #include "D3D12DescriptorHeap.hlsli"
#endif