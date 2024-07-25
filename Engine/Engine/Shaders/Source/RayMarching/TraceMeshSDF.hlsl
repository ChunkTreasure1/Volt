#include "Vertex.hlsli"
#include "Resources.hlsli"

#include "GPUScene.hlsli"

#include "Structures.hlsli"

struct Constants
{
    GPUScene gpuScene;
    UniformBuffer<ViewData> viewData;

    TextureSampler pointSampler;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 output : SV_Target0;
};

static const float RESOLUTION = 1.f;

float Intersect(SDFPrimitiveDrawData sdfPrimitive, GPUMeshSDF sdfMesh, TextureSampler pointSampler, Ray ray)
{
    const float maxDist = 500.f;
    float h = 0.5f;
    float t = 0.f;

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

        h = sdfMesh.sdfTexture.Sample3D(pointSampler, uvw);
        t += h;
    }

    return h;
}

Output MainPS(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();
    const SDFPrimitiveDrawData sdfPrimitive = constants.gpuScene.sdfPrimitiveDrawDataBuffer.Load(0);   
    const GPUMeshSDF sdfMesh = constants.gpuScene.sdfMeshesBuffer.Load(sdfPrimitive.meshSDFId);

    const float2 pixelPos = input.position.xy;
    
    float3 clipSpace = float3((pixelPos / viewData.renderSize) * float2(2.f, -2.f) - float2(1.f, -1.f), 1.f);
    float4 viewSpace = mul(viewData.inverseProjection, float4(clipSpace, 1.f));
    float3 worldDir = normalize(mul((float3x3)viewData.inverseView, viewSpace.xyz / viewSpace.w));

    Ray ray;
    ray.direction = worldDir;
    ray.length = 100000.f;
    ray.origin = viewData.cameraPosition.xyz;

    BoundingBox bb;
    bb.bmin = sdfMesh.min + sdfPrimitive.transform.position;
    bb.bmax = sdfMesh.max + sdfPrimitive.transform.position;
 
    bool isValid = bb.Intersects(ray);
    bool isHit = false;

    if (isValid)
    {
        float t = Intersect(sdfPrimitive, sdfMesh, constants.pointSampler, ray);
        if (t < 0.f)
        {
            isHit = true;
        }
    }

    clip(!isValid || !isHit ? -1.f : 1.f);

    Output output;
    output.output = float3(isHit, 0.f, 0.f);

    return output;
}