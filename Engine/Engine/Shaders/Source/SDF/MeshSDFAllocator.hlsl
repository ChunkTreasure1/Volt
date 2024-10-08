#include "Resources.hlsli"
#include "Utility.hlsli"
#include "GPUScene.hlsli"

struct BrickInfo
{
    float3 min;
    float3 max;
};

struct Constants
{
    vt::RWTypedBuffer<GPUSDFBrick> bricks;
    vt::RWTex3D<float> brickTexture;

    vt::TypedBuffer<float> brickData;
    vt::TypedBuffer<BrickInfo> brickInfo;

    uint brickTextureSize;
};

[numthreads(512, 1, 1)]
void MainCS(uint groupThreadId : SV_GroupThreadID, uint groupId : SV_GroupID)
{
    const Constants constants = GetConstants<Constants>();
    const uint brickTextureSizeInBricks = constants.brickTextureSize / 8;

    uint3 targetBrickCoord = Get3DCoordFrom1DIndex(groupId, brickTextureSizeInBricks, brickTextureSizeInBricks) * 8;
    uint3 brickLocalCoord = Get3DCoordFrom1DIndex(groupThreadId, 8, 8);

    BrickInfo brickInfo = constants.brickInfo.Load(groupId);

    GPUSDFBrick outBrick;
    outBrick.localCoords = (float3)targetBrickCoord / (float)constants.brickTextureSize;
    outBrick.min = brickInfo.min;
    outBrick.max = brickInfo.max;
    
    constants.brickTexture.Store(targetBrickCoord + brickLocalCoord, constants.brickData.Load(groupId * 512 + groupThreadId));
    constants.bricks.Store(groupId, outBrick);
}