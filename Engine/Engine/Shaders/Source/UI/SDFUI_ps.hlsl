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
}

struct UICommand
{
    uint type;
    uint primitiveGroup;
    float rotation;
    float scale;
    float2 radiusHalfSize;
    float2 pixelPos;
};

struct Constants
{
    UniformTypedBuffer<UICommand> commands;
    uint commandCount;
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

Output main(FullscreenTriangleVertex input)
{
    const float2 pixelPos = input.uv * float2(1024, 1024);

    const Constants constants = GetConstants<Constants>();

    Output output;
    output.color = float4(0.f, 0.f, 1.f, 1.f);

    for (uint i = 0; i < constants.commandCount; i++)
    {
        UICommand command = constants.commands.Load(i);

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
                    output.color = BlendColors(output.color, float4(1.f, 0.f, 0.f, alpha));
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
                const float sdf = SDF_Rectangle(position, command.radiusHalfSize) * command.scale;

                const float alpha = SDF_AA(sdf);
                if (alpha > 0.f)
                {
                    output.color = BlendColors(output.color, float4(0.f, 1.f, 0.f, alpha));
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
        }
    }

    return output;
}