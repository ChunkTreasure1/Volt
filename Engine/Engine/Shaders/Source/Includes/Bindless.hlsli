#pragma once

///// Vertex Buffers /////
ByteAddressBuffer u_vertexPositionsBuffers[] : register(t0, space1);
ByteAddressBuffer u_vertexMaterialDataBuffers[] : register(t1, space1);
ByteAddressBuffer u_vertexAnimationDataBuffers[] : register(t2, space1);
ByteAddressBuffer u_indexBuffers[] : register(t3, space1);
//////////////////////////

Texture1D u_bindlessTexture1D[] : register(t4, space1);
Texture2D u_bindlessTexture2D[] : register(t5, space1);
TextureCube u_bindlessTextureCube[] : register(t6, space1);
Texture3D u_bindlessTexture3D[] : register(t7, space1);

SamplerState u_bindlessSamplers[] : register(s8, space1);

struct ResourceHandle
{
    uint handle;
};

SamplerState GetSampler(uint samplerIndex)
{
    return u_bindlessSamplers[samplerIndex];
}