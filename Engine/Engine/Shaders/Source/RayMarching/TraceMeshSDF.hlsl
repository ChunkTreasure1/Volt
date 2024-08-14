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

float Intersect(SDFPrimitiveDrawData sdfPrimitive, GPUMeshSDF sdfMesh, vt::TextureSampler pointSampler, Ray ray)
{
    const float maxDist = 2000.f;
    float h = 0.5f;
    float t = ray.t;

    for (uint i = 0; i < 500; ++i)
    {
        if (h < 0.001f || t >= maxDist)
        {
            break;
        }

        float3 pos = ray.origin + ray.direction * t;
        float3 localPosition = pos - sdfPrimitive.transform.position - sdfMesh.min;
        
        float3 uvw = (localPosition / RESOLUTION) / sdfMesh.size;
    
        if (any(uvw < 0.f) || any(uvw > 1.f))
        {
            t += h;
            continue;
        }

        h = sdfMesh.sdfTexture.Sample(pointSampler, uvw);
        t += h;
    }

    if (t >= maxDist)
    {
        t = -1.f;
    }

    return t;
}

float TraceSDFPrimitive(SDFPrimitiveDrawData sdfPrimitive, GPUMeshSDF sdfMesh, vt::TextureSampler pointSampler, Ray ray)
{
    BoundingBox bb;
    bb.bmin = sdfMesh.min + sdfPrimitive.transform.position;
    bb.bmax = sdfMesh.max + sdfPrimitive.transform.position;

    float2 hitTime;
    float intersectionResult = bb.Intersects(ray, hitTime);
    bool isHit = false;

    if (intersectionResult > 0.f)
    {
        ray.t = intersectionResult;
        intersectionResult = Intersect(sdfPrimitive, sdfMesh, pointSampler, ray);
    }

    return intersectionResult;
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

        float intersectionRes = TraceSDFPrimitive(sdfPrimitive, sdfMesh, constants.pointSampler, ray);
        if (intersectionRes > 0.f)
        {
            result = min(intersectionRes, result);
            hasHit = true;
        }
    }

    clip(!hasHit ? -1.f : 1.f);

    Output output;
    output.output = float3(hasHit, 0.f, 0.f);

    return output;
}