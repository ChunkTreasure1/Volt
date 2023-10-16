#include "Defines.hlsli"
#include "Buffers.hlsli"
#include "FogCommon.hlsli"

static const float PI = 3.14159265359f;
static const float EPSILON = 0.0001f;

RWTexture3D<float4> o_resultTexture : register(u0, SPACE_OTHER);
Texture3D<float4> u_injectedVolumeTexture : register(t1, SPACE_OTHER);

float DistanceToSlice(uint slice)
{
    return u_cameraData.nearPlane * pow(u_cameraData.farPlane / u_cameraData.nearPlane, (float(slice) + 0.5f) / float(VOXEL_GRID_SIZE_Z));
}

float GetSliceThickness(uint slice)
{
    return abs(DistanceToSlice(slice + 1) - DistanceToSlice(slice));
}

float4 ScatterStep(uint slice, float3 accumulatedLight, float accumulatedTransmittance, float3 sliceLight, float sliceDensity)
{
    sliceDensity = max(sliceDensity, EPSILON);
    
    const float thickness = GetSliceThickness(slice);
    const float sliceTransmittance = exp(-sliceDensity * thickness * 0.01f);
    
    float3 sliceLightIntegral = sliceLight * (1.f - sliceTransmittance) / sliceDensity;

    accumulatedLight += sliceLightIntegral * accumulatedTransmittance;
    accumulatedTransmittance *= sliceTransmittance;
    
    return float4(accumulatedLight, accumulatedTransmittance);
}

[numthreads(8, 8, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
    float4 accumulatedScattTransmit = float4(0.f, 0.f, 0.f, 1.f);
    
    for (uint z = 0; z < VOXEL_GRID_SIZE_Z; z++)
    {
        const uint3 coord = uint3(dispatchId.xy, z);
        const float4 sliceScatteringDensity = u_injectedVolumeTexture[coord];
        
        accumulatedScattTransmit = ScatterStep(z, accumulatedScattTransmit.rgb, accumulatedScattTransmit.a, sliceScatteringDensity.rgb, sliceScatteringDensity.a);
        o_resultTexture[coord] = accumulatedScattTransmit;
    }
}