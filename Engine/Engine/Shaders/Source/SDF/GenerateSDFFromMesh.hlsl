#include "Resources.hlsli"
#include "Utility.hlsli"

struct Constants
{
	vt::UniformRWTex3D<float> outSDFTexture;
    vt::UniformTypedBuffer<float3> vertexPositions;
    vt::UniformTypedBuffer<uint> indexBuffer;

    uint indexCount;
    uint indexStartOffset;
    uint vertexStartOffset;

	float3 bbMin;

    float voxelSize;
    uint3 size;
};

float PointToTriangleDistance(float3 p, float3 tv0, float3 tv1, float3 tv2)
{
	float3 edge0 = tv1 - tv0;
	float3 edge1 = tv2 - tv0;
	float3 v0 = tv0 - p;

	float a = dot(edge0, edge0);
	float b = dot(edge0, edge1);
	float c = dot(edge1, edge1);
	float d = dot(edge0, v0);
	float e = dot(edge1, v0);

	float det = a * c - b * b;
	float s = b * e - c * d;
	float t = b * d - a * e;

	if (s + t <= det)
	{
		if (s < 0.f)
		{
			if (t < 0.f)
			{
				if (d < 0.f)
				{
					s = clamp(-d / a, 0.f, 1.f);
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = clamp(-e / c, 0.f, 1.f);
				}
			}
			else
			{
				s = 0.f;
				t = clamp(-e / c, 0.f, 1.f);
			}
		}
		else if (t < 0.f)
		{
			s = clamp(-d / a, 0.f, 1.f);
		}
		else
		{
			float invDet = 1.f / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if (s < 0.0f)
		{
			float tmp0 = b + d;
			float tmp1 = c + e;
			if (tmp1 > tmp0)
			{
				float numer = tmp1 - tmp0;
				float denom = a - 2 * b + c;
				s = clamp(numer / denom, 0.0f, 1.0f);
				t = 1 - s;
			}
			else
			{
				t = clamp(-e / c, 0.0f, 1.0f);
				s = 0.0f;
			}
		}
		else if (t < 0.0f)
		{
			if (a + d > b + e)
			{
				float numer = c + e - b - d;
				float denom = a - 2 * b + c;
				s = clamp(numer / denom, 0.0f, 1.0f);
				t = 1 - s;
			}
			else
			{
				s = clamp(-e / c, 0.0f, 1.0f);
				t = 0.0f;
			}
		}
		else
		{
			float numer = c + e - b - d;
			float denom = a - 2 * b + c;
			s = clamp(numer / denom, 0.0f, 1.0f);
			t = 1 - s;
		}
	}

	float3 closestPoint = tv0 + s * edge0 + t * edge1;
	return length(closestPoint - p);
}

float SignedDistance(float3 p, float3 tv0, float3 tv1, float3 tv2)
{
    float3 normal = normalize(cross(tv1 - tv0, tv2 - tv0));
    float distance = PointToTriangleDistance(p, tv0, tv1, tv2);
	float sign = dot(p - tv0, normal);
	return sign < 0.f ? -distance : distance;
}

[numthreads(8, 8, 8)]
void MainCS(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const Constants constants = GetConstants<Constants>();
    
    if (any(dispatchThreadId >= constants.size))
    {
        return;
    }

    const float3 pointPos = float3(dispatchThreadId.x * constants.voxelSize, dispatchThreadId.y * constants.voxelSize, dispatchThreadId.z * constants.voxelSize) + constants.bbMin;

    for (uint index = constants.indexStartOffset; index < constants.indexStartOffset + constants.indexCount; index += 3)
    {
        const uint idx0 = constants.indexBuffer.Load(index + 0) + constants.vertexStartOffset;
        const uint idx1 = constants.indexBuffer.Load(index + 1) + constants.vertexStartOffset;
        const uint idx2 = constants.indexBuffer.Load(index + 2) + constants.vertexStartOffset;
    
        const float3 v0 = constants.vertexPositions.Load(idx0);
        const float3 v1 = constants.vertexPositions.Load(idx1);
        const float3 v2 = constants.vertexPositions.Load(idx2);

        const float distance = SignedDistance(pointPos, v0, v1, v2);

		if (abs(distance) < abs(constants.outSDFTexture.Load(dispatchThreadId)))
		{
			constants.outSDFTexture.Store(dispatchThreadId, distance);
		}
    }
}