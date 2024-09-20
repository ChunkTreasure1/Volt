#include "Vertex.hlsli"
#include "Resources.hlsli"

static const float PI = 3.14159265359f;

struct Output
{
    [[vt::rgba8]] float4 color : SV_Target;
};

namespace UIPrimitiveType
{
    static const uint CIRCLE = 0;
    static const uint RECTANGLE = 1;
    static const uint TEXT_CHARACTER = 2;
}

struct UICommand
{
    uint type;
    uint primitiveGroup;
    float rotation;
    float scale;

    float2 radiusHalfSize;
    float2 pixelPos;

    uint color;
    vt::Tex2D<float4> texture;
    float2 padding;    

    float4 minMaxUV;
    float4 minMaxPx;
};

struct Constants
{
    vt::UniformTypedBuffer<UICommand> commands;
    vt::TextureSampler linearSampler;
    uint commandCount;
    uint2 renderSize;
};

float SDF_Circle(float2 pixelPos, float radius)
{
    return length(pixelPos) - radius;
}

float SDF_Rectangle(float2 pixelPos, float2 halfSize)
{
    float2 compWiseEdgeDistance = abs(pixelPos) - halfSize;
    float outsideDistance = length(max(compWiseEdgeDistance, 0.f));
    float insideDistance = min(max(compWiseEdgeDistance.x, compWiseEdgeDistance.y), 0.f);

    return outsideDistance + insideDistance;
}

float2 SDF_Scale(float2 pos, float scale)
{
    const float rcpScale = rcp(scale);
    return pos * rcpScale;
}

float2 SDF_Rotate(float2 pos, float rotation)
{
    const float angle = rotation * PI * 2.f * -1.f;
    float sine, cosine;
    sincos(angle, sine, cosine);

    return float2(cosine * pos.x + sine * pos.y, cosine * pos.y - sine * pos.x);
}

float2 SDF_Translate(float2 posA, float2 posB)
{
    return posA - posB;
}

float SDF_AA(float sdf)
{
    float dsdf = fwidth(sdf) * 0.5f;
    float aa = smoothstep(dsdf, -dsdf, sdf);
    
    return aa;
}

float SDF_Merge(float sdf0, float sdf1)
{
    return min(sdf0, sdf1);
}

float SDF_Intersection(float sdf0, float sdf1)
{
    return max(sdf0, sdf1);
}

float SDF_Subtract(float sdf0, float sdf1)
{
    return SDF_Intersection(sdf0, -sdf1);
}

float SDF_Interpolate(float sdf0, float sdf1, float t)
{
    return lerp(sdf0, sdf1, t);
}

float4 BlendColors(float4 colorA, float4 colorB)
{
    float4 result;
    result.rgb = colorA.rgb * (1.f - colorB.a) + colorB.rgb * colorB.a;
    result.a = colorB.a;

    return result;
}

float ScreenPxRange(float2 msdfSize, float2 screenSize)
{
    const float pxRange = 2.f;
    float2 unitRange = pxRange / msdfSize;
    
    return max(0.5f * dot(unitRange, screenSize), 1.f);
}

float SDF_TextMedian(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

Output main(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
    const float2 pixelPos = input.uv * float2(constants.renderSize);

    Output output;
    output.color = float4(0.f, 0.f, 1.f, 1.f);

    for (uint i = 0; i < constants.commandCount; i++)
    {
        UICommand command = constants.commands.Load(i);
        const uint mask = 0xff;
        const float4 color = float4(
        ((command.color >> 24)  & mask) / 255.f,
        ((command.color >> 16)  & mask) / 255.f,
        ((command.color >> 8)   & mask) / 255.f,
        (command.color          & mask) / 255.f);
        
        
        switch (command.type)
        {
            case UIPrimitiveType::CIRCLE:
            {
                float2 position = SDF_Translate(command.pixelPos, pixelPos);
                position = SDF_Rotate(position, command.rotation);
                position = SDF_Scale(position, command.scale);
                const float sdf = SDF_Circle(position, command.radiusHalfSize.x) * command.scale;

                const float alpha = SDF_AA(sdf);
                if (alpha > 0.f)
                {
                    output.color = BlendColors(output.color, float4(color.rgb, alpha));
                }

                const float dropShadowRadius = command.radiusHalfSize.x * 0.05f;
                const float t = sdf / dropShadowRadius;

                if (t > 0.f && t <= 1.f)
                {
                    const float strength = 0.5f;
                    output.color = BlendColors(output.color, float4(0.f, 0.f, 0.f, lerp(1.f, 0.f, t) * strength));
                }

                break;
            }

            case UIPrimitiveType::RECTANGLE:
            {
                float2 position = SDF_Translate(command.pixelPos, pixelPos);
                position = SDF_Rotate(position, command.rotation);
                position = SDF_Scale(position, command.scale);
                position = SDF_Translate(position, -command.radiusHalfSize);
                
                const float sdf = SDF_Rectangle(position, command.radiusHalfSize) * command.scale;

                const float alpha = SDF_AA(sdf);
                if (alpha > 0.f)
                {
                    output.color = BlendColors(output.color, float4(color.rgb, alpha));
                }

                const float dropShadowRadius = command.radiusHalfSize.x * 0.05f;
                const float t = sdf / dropShadowRadius;

                if (t > 0.f && t <= 1.f)
                {
                    const float strength = 0.5f;
                    output.color = BlendColors(output.color, float4(0.f, 0.f, 0.f, lerp(1.f, 0.f, t) * strength));
                }

                break;
            }

            case UIPrimitiveType::TEXT_CHARACTER:
            {
                if (pixelPos.x < command.minMaxPx.x || pixelPos.x > command.minMaxPx.z || pixelPos.y < command.minMaxPx.w || pixelPos.y > command.minMaxPx.y)
                {
                    break;
                }

                // Calculate UV within character quad
                const float xPercent = (pixelPos.x - command.minMaxPx.x) / (command.minMaxPx.z - command.minMaxPx.x);
                const float yPercent = (pixelPos.y - command.minMaxPx.y) / (command.minMaxPx.w - command.minMaxPx.y);
                
                const float2 textTexUv = float2(lerp(command.minMaxUV.x, command.minMaxUV.z, xPercent), lerp(command.minMaxUV.y, command.minMaxUV.w, yPercent));
                
                // Sample UV
                const float3 msd = command.texture.Sample(constants.linearSampler, textTexUv).rgb;
                
                uint2 msdfSize;
                command.texture.GetDimensions(msdfSize.x, msdfSize.y);
                
                float4 bgColor = float4(color.xyz, 0.f);
                float4 fgColor = color;
                
                float sd = SDF_TextMedian(msd.x, msd.y, msd.z);
                float screenPxDistance = ScreenPxRange((float2)msdfSize, (float2)constants.renderSize) * (sd - 0.5f);
                float opacity = clamp(screenPxDistance + 0.5f, 0.f, 1.f);    
                
                if (opacity > 0.f)
                {
                    output.color = BlendColors(output.color, float4(lerp(bgColor.rgb, fgColor.rgb, opacity), opacity));
                }

                break;
            }
        }
    }

    return output;
}