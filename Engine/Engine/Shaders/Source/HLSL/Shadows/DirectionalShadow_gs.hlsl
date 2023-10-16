#include <Common.hlsli>
#include <Buffers.hlsli>

struct Output
{
    float4 position : SV_Position;
    uint target : SV_RenderTargetArrayIndex;
};

[maxvertexcount(15)]
void main(triangle float4 input[3] : POSITION, inout TriangleStream<Output> output)
{
    const uint cascadeCount = 4;
	
    for (uint cascade = 0; cascade < cascadeCount; cascade++)
    {
        Output result;
        result.target = cascade;

        for (uint i = 0; i < 3; i++)
        {
            result.position = mul(u_directionalLight.viewProjections[cascade], input[i]);
            result.target = cascade;
            output.Append(result);
        }

        output.RestartStrip();
    }
}