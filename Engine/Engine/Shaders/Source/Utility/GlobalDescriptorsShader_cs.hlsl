#include "Defines.hlsli"
#include "Resources.hlsli"

#define GET_TEXTURES_WITH_TYPES(textureType, index) \
   textureType##<float> textureType##_f1 = u_##textureType##float[index]; \
   textureType##<float2> textureType##_f2 = u_##textureType##float2[index]; \
   textureType##<float3> textureType##_f3 = u_##textureType##float3[index]; \
   textureType##<float4> textureType##_f4 = u_##textureType##float4[index]; \
    textureType##<uint> textureType##_u1 = u_##textureType##uint[index]; \
    textureType##<uint2> textureType##_u2 = u_##textureType##uint2[index]; \
    textureType##<uint3> textureType##_u3 = u_##textureType##uint3[index]; \
    textureType##<uint4> textureType##_u4 = u_##textureType##uint4[index]; \

#define USE_TEXTURES_WITH_TYPES(textureType, location) \
    RW##textureType##_f1[location] = textureType##_f1[location]; \
    RW##textureType##_f2[location] = textureType##_f2[location]; \
    RW##textureType##_f3[location] = textureType##_f3[location]; \
    RW##textureType##_f4[location] = textureType##_f4[location]; \
    RW##textureType##_u1[location] = textureType##_u1[location]; \
    RW##textureType##_u2[location] = textureType##_u2[location]; \
    RW##textureType##_u3[location] = textureType##_u3[location]; \
    RW##textureType##_u4[location] = textureType##_u4[location]; \

[numthreads(1, 1, 1)]
void main()
{
    GET_TEXTURES_WITH_TYPES(Texture1D, 0)
    GET_TEXTURES_WITH_TYPES(Texture2D, 0)
    GET_TEXTURES_WITH_TYPES(Texture3D, 0)
    GET_TEXTURES_WITH_TYPES(TextureCube, 0)
    GET_TEXTURES_WITH_TYPES(RWTexture1D, 0)
    GET_TEXTURES_WITH_TYPES(RWTexture2D, 0)
    GET_TEXTURES_WITH_TYPES(RWTexture3D, 0)
    GET_TEXTURES_WITH_TYPES(RWTexture2DArray, 0)
    
    USE_TEXTURES_WITH_TYPES(Texture1D, 0)
    USE_TEXTURES_WITH_TYPES(Texture2D, uint2(0, 0))
    USE_TEXTURES_WITH_TYPES(Texture3D, uint3(0, 0, 0))
    
    SamplerState samplerS = u_SamplerState[0];
    
    RWTexture2DArray_f1[uint3(0, 0, 0)] = TextureCube_f1.SampleLevel(samplerS, 0.f, 0.f);
    RWTexture2DArray_f2[uint3(0, 0, 0)] = TextureCube_f2.SampleLevel(samplerS, 0.f, 0.f);
    RWTexture2DArray_f3[uint3(0, 0, 0)] = TextureCube_f3.SampleLevel(samplerS, 0.f, 0.f);
    RWTexture2DArray_f4[uint3(0, 0, 0)] = TextureCube_f4.SampleLevel(samplerS, 0.f, 0.f);
    
    //RWTexture2DArray_u1[uint3(0, 0, 0)] = TextureCube_u1.SampleLevel(samplerS, 0.f, 0.f);
    //RWTexture2DArray_u2[uint3(0, 0, 0)] = TextureCube_u2.SampleLevel(samplerS, 0.f, 0.f);     
    //RWTexture2DArray_u3[uint3(0, 0, 0)] = TextureCube_u3.SampleLevel(samplerS, 0.f, 0.f);     
    //RWTexture2DArray_u4[uint3(0, 0, 0)] = TextureCube_u4.SampleLevel(samplerS, 0.f, 0.f);

    ByteAddressBuffer buffVal = u_ByteAddressBuffer[0];
    RWByteAddressBuffer buffValRW = u_RWByteAddressBuffer[0];
    ByteAddressBuffer uniBuffVal = u_UniformBuffer[0];
    
    buffValRW.Store(0, buffVal.Load(0));
    buffValRW.Store(0, uniBuffVal.Load(0));
    
    float s = Texture2D_f1.SampleLevel(samplerS, float2(0.f, 0.f), 0);

    buffValRW.Store<float>(0, s);
}