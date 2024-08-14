#include "Resources.hlsli"
#include "Vertex.hlsli"

#include "SkyAtmosphereCommon.hlsli"

struct Constants
{
	vt::UniformBuffer<AtmosphereParameters> atmosphereBuffer;

	float4x4 skyInvViewProj;
	float2 rayMarchMinMaxSPP;
};

struct Output
{
    [[vt::rgba16f]] float4 color : SV_Target0;
};

void UvToLutTransmittanceParams(AtmosphereParameters atmosphere, out float viewHeight, out float viewZenithCosAngle, in float2 uv)
{
	float x_mu = uv.x;
	float x_r = uv.y;

	float H = sqrt(atmosphere.TopRadius * atmosphere.TopRadius - atmosphere.BottomRadius * atmosphere.BottomRadius);
	float rho = H * x_r;
	viewHeight = sqrt(rho * rho + atmosphere.BottomRadius * atmosphere.BottomRadius);

	float d_min = atmosphere.TopRadius - viewHeight;
	float d_max = rho + H;
	float d = d_min + x_mu * (d_max - d_min);
	viewZenithCosAngle = d == 0.0 ? 1.0f : (H * H - rho * rho - d * d) / (2.0 * viewHeight * d);
	viewZenithCosAngle = clamp(viewZenithCosAngle, -1.0, 1.0);
}

float3 GetOpticalDepth(float2 uv, float3 WorldPos, float3 WorldDir, float SampleCountIni, float DepthBufferValue, in AtmosphereParameters Atmosphere, float4x4 skyInvViewProj, float2 rayMarchMinMaxSPP, bool VariableSampleCount, float tMaxMax = 9000000.0f)
{
	float3 ClipSpace = float3(uv * float2(2.0, -2.0) - float2(1.0, -1.0), 1.0);

	float3 earthO = float3(0.0f, 0.0f, 0.0f);
	float tBottom = raySphereIntersectNearest(WorldPos, WorldDir, earthO, Atmosphere.BottomRadius);
	float tTop = raySphereIntersectNearest(WorldPos, WorldDir, earthO, Atmosphere.TopRadius);
	float tMax = 0.0f;
	if (tBottom < 0.0f)
	{
		if (tTop < 0.0f)
		{
			tMax = 0.0f; // No intersection with earth nor atmosphere: stop right away  
			return 0.f;
		}
		else
		{
			tMax = tTop;
		}
	}
	else
	{
		if (tTop > 0.0f)
		{
			tMax = min(tTop, tBottom);
		}
	}

	if (DepthBufferValue >= 0.0f)
	{
		ClipSpace.z = DepthBufferValue;
		if (ClipSpace.z < 1.0f)
		{
			float4 DepthBufferWorldPos = mul(skyInvViewProj, float4(ClipSpace, 1.0));
			DepthBufferWorldPos /= DepthBufferWorldPos.w;

			float tDepth = length(DepthBufferWorldPos.xyz - (WorldPos + float3(0.0, 0.0, -Atmosphere.BottomRadius))); // apply earth offset to go back to origin as top of earth mode. 
			if (tDepth < tMax)
			{
				tMax = tDepth;
			}
		}
		//		if (VariableSampleCount && ClipSpace.z == 1.0f)
		//			return result;
	}
	tMax = min(tMax, tMaxMax);

	float SampleCount = SampleCountIni;
	float SampleCountFloor = SampleCountIni;
	float tMaxFloor = tMax;
	if (VariableSampleCount)
	{
		SampleCount = lerp(rayMarchMinMaxSPP.x, rayMarchMinMaxSPP.y, saturate(tMax * 0.01));
		SampleCountFloor = floor(SampleCount);
		tMaxFloor = tMax * SampleCountFloor / SampleCount;	// rescale tMax to map to the last entire step segment.
	}
	float dt = tMax / SampleCount;

	// Ray march the atmosphere to integrate optical depth
	float3 L = 0.0f;
	float3 throughput = 1.0;
	float3 OpticalDepth = 0.0;
	float t = 0.0f;
	float tPrev = 0.0;
	const float SampleSegmentT = 0.3f;
	for (float s = 0.0f; s < SampleCount; s += 1.0f)
	{
		if (VariableSampleCount)
		{
			// More expenssive but artefact free
			float t0 = (s) / SampleCountFloor;
			float t1 = (s + 1.0f) / SampleCountFloor;
			// Non linear distribution of sample within the range.
			t0 = t0 * t0;
			t1 = t1 * t1;
			// Make t0 and t1 world space distances.
			t0 = tMaxFloor * t0;
			if (t1 > 1.0)
			{
				t1 = tMax;
				//	t1 = tMaxFloor;	// this reveal depth slices
			}
			else
			{
				t1 = tMaxFloor * t1;
			}
			//t = t0 + (t1 - t0) * (whangHashNoise(pixPos.x, pixPos.y, gFrameId * 1920 * 1080)); // With dithering required to hide some sampling artefact relying on TAA later? This may even allow volumetric shadow?
			t = t0 + (t1 - t0)*SampleSegmentT;
			dt = t1 - t0;
		}
		else
		{
			//t = tMax * (s + SampleSegmentT) / SampleCount;
			// Exact difference, important for accuracy of multiple scattering
			float NewT = tMax * (s + SampleSegmentT) / SampleCount;
			dt = NewT - t;
			t = NewT;
		}
	
		float3 P = WorldPos + t * WorldDir;

		MediumSampleRGB medium = sampleMediumRGB(P, Atmosphere);
		const float3 SampleOpticalDepth = medium.extinction * dt;
		const float3 SampleTransmittance = exp(-SampleOpticalDepth);
		OpticalDepth += SampleOpticalDepth;
	}

	return OpticalDepth;
}

Output MainPS(FullscreenTriangleVertex input)
{
    const Constants constants = GetConstants<Constants>();
	const AtmosphereParameters atmosphere = constants.atmosphereBuffer.Load();

    float viewHeight;
	float viewZenithCosAngle;
	UvToLutTransmittanceParams(atmosphere, viewHeight, viewZenithCosAngle, input.uv);

	float3 worldPos = float3(0.f, 0.f, viewHeight);
	float3 worldDir = float3(0.f, sqrt(1.f - viewZenithCosAngle * viewZenithCosAngle), viewZenithCosAngle);

	const float sampleCount = 40.f;
	const float depthBufferValue = -1.f;
	const bool variableSampleCount = false;

	float3 transmittance = exp(-GetOpticalDepth(input.uv, worldPos, worldDir, sampleCount, depthBufferValue, atmosphere, constants.skyInvViewProj, constants.rayMarchMinMaxSPP, variableSampleCount));

	Output output;
	output.color = float4(transmittance, 1.f);

	return output;
}