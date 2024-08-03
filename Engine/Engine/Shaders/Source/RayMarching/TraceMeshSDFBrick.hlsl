#include "Vertex.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

#include "Structures.hlsli"

struct Constants
{
    GPUScene gpuScene;
    vt::UniformBuffer<ViewData> viewData;

    vt::TextureSampler pointSampler;
    
    uint primitiveCount;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

static const float RESOLUTION = 5.f;

bool Intersect(SDFPrimitiveDrawData sdfPrimitive, GPUMeshSDF sdfMesh, GPUSDFBrick brick, vt::TextureSampler pointSampler, Ray ray, float maxSampleTime, out float hitDistance)
{
    const uint maxSteps = 64;
    const float minStepSize = 1.f / (16.f * maxSteps);
    const float maxDistance = 10000.f;    

    uint stepIndex = 0;
    bool hit = false;
    
    float sampleRayTime = ray.t;

    for (; stepIndex < maxSteps; stepIndex++)
    {
        float3 samplePos = ray.origin + ray.direction * sampleRayTime;
        float3 localPosition = samplePos - sdfPrimitive.transform.position - brick.min;
        float3 voxelPosition = localPosition / 5.f;
        float3 uvw = brick.localCoords + voxelPosition / sdfMesh.size; 
        
        float distanceField = sdfMesh.sdfTexture.Sample(pointSampler, uvw);

        if (distanceField < 0.001f)
        {
            hit = true;
            break;
        }

        float stepDistance = max(distanceField, minStepSize);
        sampleRayTime += stepDistance;

        // We have exceeded the brick
        if (sampleRayTime > maxSampleTime) 
        {
            break;
        }
    }
    
    hitDistance = sampleRayTime;
    return hit;    
}

bool TraceSDFPrimitive(SDFPrimitiveDrawData sdfPrimitive, GPUMeshSDF sdfMesh, vt::TextureSampler pointSampler, Ray ray, inout float hitTime)
{
    BoundingBox bb;
    bb.bmin = sdfMesh.min + sdfPrimitive.transform.position;
    bb.bmax = sdfMesh.max + sdfPrimitive.transform.position;

    float2 tempHitTimes;
    bool hit = bb.Intersects(ray, tempHitTimes);

    if (hit)
    {
        hit = false;

        for (uint i = 0; i < sdfMesh.brickCount; ++i)
        {
            const GPUSDFBrick brick = sdfMesh.bricksBuffer.Load(i);

            BoundingBox brickBB;
            brickBB.bmin = brick.min + sdfPrimitive.transform.position;
            brickBB.bmax = brick.max + sdfPrimitive.transform.position;

            float2 brickHitTimes;
            bool brickHit = brickBB.Intersects(ray, brickHitTimes);
            if (brickHit)
            {
                ray.t = brickHitTimes.x;

                float hitDistance;
                brickHit = Intersect(sdfPrimitive, sdfMesh, brick, pointSampler, ray, brickHitTimes.y, hitDistance);
                if (brickHit)
                {
                    hitTime = min(hitTime, hitDistance);
                    hit = true;                
                }
            }
                    
        }
    }

    return hit;
}

Output MainPS(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();

    const float2 pixelPos = input.position.xy;
    
    float3 clipSpace = float3((pixelPos / viewData.renderSize) * float2(2.f, -2.f) - float2(1.f, -1.f), 1.f);
    float4 viewSpace = mul(viewData.inverseProjection, float4(clipSpace, 1.f));
    float3 worldDir = normalize(mul((float3x3)viewData.inverseView, viewSpace.xyz / viewSpace.w));

    Ray ray;
    ray.direction = worldDir;
    ray.t = 0.f;
    ray.origin = viewData.cameraPosition.xyz;

    float result = 100000.f;
    bool hasHit = false;

    for (uint i = 0; i < constants.primitiveCount; ++i)
    {
        const SDFPrimitiveDrawData sdfPrimitive = constants.gpuScene.sdfPrimitiveDrawDataBuffer.Load(i);   
        const GPUMeshSDF sdfMesh = constants.gpuScene.sdfMeshesBuffer.Load(sdfPrimitive.meshSDFId);

        float traceT = 10000.f;
        bool hit = TraceSDFPrimitive(sdfPrimitive, sdfMesh, constants.pointSampler, ray, traceT);
        if (hit)
        {
            result = traceT;
            hasHit = true;
        }
    }

    clip(!hasHit ? -1.f : 1.f);

    Output output;
    output.output = float3(result, 0.f, 0.f);

    return output;
}