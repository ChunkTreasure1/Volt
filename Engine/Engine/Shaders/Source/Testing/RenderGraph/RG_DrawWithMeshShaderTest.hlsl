#include "Resources.hlsli"

struct DummyPayload
{
    uint d;
};

groupshared DummyPayload m_dummyPayload;

[numthreads(1, 1, 1)]
void AmpMain()
{
    DispatchMesh(3, 1, 1, m_dummyPayload);
}

struct VertexOutput
{
	float4 position: SV_Position;
	float4 color : COLOR0;
};

static const float4 m_positions[3] = {
	float4( 0.0, -1.0, 0.0, 1.0),
	float4(-1.0,  1.0, 0.0, 1.0),
	float4( 1.0,  1.0, 0.0, 1.0)
};

static const float4 m_colors[3] = {
	float4(0.0, 1.0, 0.0, 1.0),
	float4(0.0, 0.0, 1.0, 1.0),
	float4(1.0, 0.0, 0.0, 1.0)
};

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MeshMain(out indices uint3 triangles[1], out vertices VertexOutput vertices[3], uint3 dispatchThreadId : SV_DispatchThreadID)
{
    float4 offset = float4(0.f, 0.f, (float)dispatchThreadId.x, 0.f);

	SetMeshOutputCounts(3, 1);
	for (uint i = 0; i < 3; i++)
	{
		vertices[i].position = m_positions[i] + offset;	
		vertices[i].color = m_colors[i];
	}

	SetMeshOutputCounts(3, 1);
	triangles[0] = uint3(0, 1, 2);
}

struct ColorOutput
{
	[[vt::rgba8]] float4 color : SV_Target0;
};

ColorOutput MainPS(VertexOutput input)
{
	ColorOutput output;
	output.color = input.color;
	return output;
}