#include "Resources.hlsli"
#include "Vertex.hlsli"
#include "Structures.hlsli"

struct Constants
{
    vt::Tex2D<float3> sceneColor;
    vt::UniformBuffer<ViewData> viewData;

    vt::TextureSampler linearSampler;
};

struct Output
{
    [[vt::r11f_g11f_b10f]] float3 color : SV_Target0;
};

float RGBToLuma(float3 rgb)
{
    return sqrt(dot(rgb, float3(0.299f, 0.587f, 0.114f)));
}

static const float QUALITY[12] = 
{
    1.f, 1.f, 1.f, 1.f, 1.f, 1.5f,
    2.f, 2.f, 2.f, 2.f, 4.f, 8.f
};

static const float EDGE_THRESHOLD_MIN = 0.0312f;
static const float EDGE_THRESHOLD_MAX = 0.125f;
static const uint ITERATIONS = 12;

Output MainPS(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
    const ViewData viewData = constants.viewData.Load();
    
    vt::Tex2D<float3> sceneColor = constants.sceneColor;
    vt::TextureSampler linearSampler = constants.linearSampler;

    float3 colorCenter = sceneColor.Sample(linearSampler, input.uv);

    // Luma at current fragment
    float lumaCenter = RGBToLuma(colorCenter);

    // Luma of four direct neighbours
    float lumaDown = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(0, -1)));
    float lumaUp = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(0, 1)));
    float lumaLeft = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(-1, 0)));
    float lumaRight = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(1, 0)));

    // Find min and max luma
    float lumaMin = min(lumaCenter, min(min(lumaDown, lumaUp), min(lumaLeft, lumaRight)));
    float lumaMax = max(lumaCenter, max(max(lumaDown, lumaUp), max(lumaLeft, lumaRight)));

    float lumaRange = lumaMax - lumaMin;

    if (lumaRange < max(EDGE_THRESHOLD_MIN, lumaMax * EDGE_THRESHOLD_MAX))
    {
        Output output;
        output.color = colorCenter;
        return output;
    }

    // Get the four remaining corners
    float lumaDownLeft = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(-1, -1)));
    float lumaUpRight = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(1, 1)));
    float lumaUpLeft = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(-1, 1)));
    float lumaDownRight = RGBToLuma(sceneColor.Sample(linearSampler, input.uv, int2(1, -1)));

    // Combine four edges.
    float lumaDownUp = lumaDown + lumaUp;
    float lumaLeftRight = lumaLeft + lumaRight;

    // Combine the corners
    float lumaLeftCorners = lumaDownLeft + lumaUpLeft;
    float lumaDownCorners = lumaDownLeft + lumaDownRight;
    float lumaRightCorners = lumaDownRight + lumaUpRight;
    float lumaUpCorners = lumaUpRight + lumaUpLeft;

    // Compute estimation of the gradient along the horizontal and vertical axis
    float edgeHorizontal = abs(-2.f * lumaLeft + lumaLeftCorners) + abs(-2.f * lumaCenter + lumaDownUp) * 2.f + abs(-2.f * lumaRight + lumaRightCorners);
    float edgeVertical = abs(-2.f * lumaUp + lumaUpCorners) + abs(-2.f * lumaCenter + lumaLeftRight) * 2.f + abs(-2.f * lumaDown + lumaDownCorners);

    bool isHorizontal = (edgeHorizontal >= edgeVertical);

    float luma1 = isHorizontal ? lumaDown : lumaLeft;
    float luma2 = isHorizontal ? lumaUp : lumaRight;

    float gradient1 = luma1 - lumaCenter;
    float gradient2 = luma2 - lumaCenter;

    // Find steepest direction
    bool is1Steepest = abs(gradient1) >= abs(gradient2);
    float gradientScaled = 0.25f * max(abs(gradient1), abs(gradient2));

    float stepLength = isHorizontal ? viewData.invRenderSize.y : viewData.invRenderSize.x;
   
    float lumaLocalAverage = 0.f;
    if (is1Steepest)
    {
        stepLength = -stepLength;
        lumaLocalAverage = 0.5f * (luma1 + lumaCenter);
    }
    else 
    {
        lumaLocalAverage = 0.5f * (luma2 + lumaCenter);
    }

    float2 currentUv = input.uv;

    // Shift UV in the correct direction by half a pixel
    if (isHorizontal)
    {
        currentUv.y += stepLength * 0.5f;
    }
    else
    {
        currentUv.x += stepLength * 0.5f;
    }

    float2 offset = isHorizontal ? float2(viewData.invRenderSize.x, 0.f) : float2(0.f, viewData.invRenderSize.y);

    float2 uv1 = currentUv - offset;
    float2 uv2 = currentUv + offset;

    float lumaEnd1 = RGBToLuma(sceneColor.Sample(linearSampler, uv1));
    float lumaEnd2 = RGBToLuma(sceneColor.Sample(linearSampler, uv2));

    lumaEnd1 -= lumaLocalAverage;
    lumaEnd2 -= lumaLocalAverage;

    // If luma deltas at current offset is larger than local gradient, we have reached the end of the edge.
    bool reached1 = abs(lumaEnd1) >= gradientScaled;
    bool reached2 = abs(lumaEnd2) >= gradientScaled;
    bool reachedBoth = reached1 && reached2;

    if (!reached1)
    {
        uv1 -= offset;
    }

    if (!reached2)
    {
        uv2 += offset;
    }

    if (!reachedBoth)
    {
        for (int i = 0; i < ITERATIONS; i++)
        {
            if (!reached1)
            {
                lumaEnd1 = RGBToLuma(sceneColor.Sample(linearSampler, uv1));
                lumaEnd1 = lumaEnd1 - lumaLocalAverage;
            }

            if (!reached2)
            {
                lumaEnd2 = RGBToLuma(sceneColor.Sample(linearSampler, uv2));
                lumaEnd2 = lumaEnd2 - lumaLocalAverage;
            }

            reached1 = abs(lumaEnd1) >= gradientScaled;
            reached2 = abs(lumaEnd2) >= gradientScaled;
            reachedBoth = reached1 && reached2;

            if (!reached1)
            {
                uv1 -= offset * QUALITY[i];
            }
                        
            if (!reached2)
            {
                uv2 += offset * QUALITY[i];
            }

            if (reachedBoth)
            {
                break;
            }
        }
    }

    float distance1 = isHorizontal ? (input.uv.x - uv1.x) : (input.uv.y - uv1.y);
    float distance2 = isHorizontal ? (uv2.x - input.uv.x) : (uv2.y - input.uv.y);

    bool isDirection1 = distance1 < distance2;
    float distanceFinal = min(distance1, distance2);

    float edgeThickness = (distance1 + distance2);
    float pixelOffset = -distanceFinal / edgeThickness + 0.5f;

    bool isLumaCenterSmaller = lumaCenter < lumaLocalAverage;
    bool correctVariation = ((isDirection1 ? lumaEnd1 : lumaEnd2) < 0.f) != isLumaCenterSmaller;

    float finalOffset = correctVariation ? pixelOffset : 0.f;

    float2 finalUv = input.uv;
    if (isHorizontal)
    {
        finalUv.y += finalOffset * stepLength;
    }
    else    
    {
        finalUv.x += finalOffset * stepLength;
    }

    float3 finalColor = sceneColor.Sample(linearSampler, finalUv);
    
    Output output;
    output.color = finalColor;
    return output;
}