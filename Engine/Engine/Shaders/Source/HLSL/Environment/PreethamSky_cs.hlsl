// From https://www.shadertoy.com/view/llSSDR

RWTexture2DArray<float4> o_output : register(u0);

#define PI 3.14159265359f

struct PushConstants
{
    float turbidity;
    float azimuth;
    float inclination;
};

[[vk::push_constant]] PushConstants u_pushConstants;

float3 GetCubeMapTexCoord(uint3 dispatchId)
{
	uint2 texSize;
	uint elements;

	o_output.GetDimensions(texSize.x, texSize.y, elements);

	float2 ST = dispatchId.xy / float2(texSize.x, texSize.y);
	float2 UV = 2.f * float2(ST.x, 1.f - ST.y) - 1.f;

	float3 result = 0.f;
	switch (dispatchId.z)
	{
		case 0: result = float3(1.f, UV.y, -UV.x); break;
		case 1: result = float3(-1.f, UV.y, UV.x); break;
		case 2: result = float3(UV.x, 1.f, -UV.y); break;
		case 3: result = float3(UV.x, -1.f, UV.y); break;
		case 4: result = float3(UV.x, UV.y, 1.f); break;
		case 5: result = float3(-UV.x, UV.y, -1.f); break;
	}

	return normalize(result);
}

float SaturatedDot(in float3 a, in float3 b)
{
	return max(dot(a, b), 0.f);
}

float3 YxyToXYZ(in float3 Yxy)
{
	float Y = Yxy.r;
	float x = Yxy.g;
	float y = Yxy.b;

	float X = x * (Y / y);
	float Z = (1.f - x - y) * (Y / y);

	return float3(X, Y, Z);
}

float3 XYZToRGB(in float3 XYZ)
{
	float3x3 M = float3x3
		(
			2.3706743, -0.9000405, -0.4706338,
			-0.5138850, 1.4253036, 0.0885814,
			0.0052982, -0.0146949, 1.0093968
			);

	return mul(M, XYZ);
}

float3 YxyToRGB(in float3 Yxy)
{
	const float3 XYZ = YxyToXYZ(Yxy);
	const float3 RGB = XYZToRGB(XYZ);
	return RGB;
}

void CalculatePerezDistribution(in float t, out float3 A, out float3 B, out float3 C, out float3 D, out float3 E)
{
	A = float3(0.1787 * t - 1.4630, -0.0193 * t - 0.2592, -0.0167 * t - 0.2608);
	B = float3(-0.3554 * t + 0.4275, -0.0665 * t + 0.0008, -0.0950 * t + 0.0092);
	C = float3(-0.0227 * t + 5.3251, -0.0004 * t + 0.2125, -0.0079 * t + 0.2102);
	D = float3(0.1206 * t - 2.5771, -0.0641 * t - 0.8989, -0.0441 * t - 1.6537);
	E = float3(-0.0670 * t + 0.3703, -0.0033 * t + 0.0452, -0.0109 * t + 0.0529);
}

float3 CalculateZenithLuminanceYxy(in float t, in float thetaS)
{
	float chi = (4.0 / 9.0 - t / 120.0) * (PI - 2.0 * thetaS);
	float Yz = (4.0453 * t - 4.9710) * tan(chi) - 0.2155 * t + 2.4192;

	float theta2 = thetaS * thetaS;
	float theta3 = theta2 * thetaS;
	float T = t;
	float T2 = t * t;

	float xz =
		(0.00165 * theta3 - 0.00375 * theta2 + 0.00209 * thetaS + 0.0) * T2 +
		(-0.02903 * theta3 + 0.06377 * theta2 - 0.03202 * thetaS + 0.00394) * T +
		(0.11693 * theta3 - 0.21196 * theta2 + 0.06052 * thetaS + 0.25886);

	float yz =
		(0.00275 * theta3 - 0.00610 * theta2 + 0.00317 * thetaS + 0.0) * T2 +
		(-0.04214 * theta3 + 0.08970 * theta2 - 0.04153 * thetaS + 0.00516) * T +
		(0.15346 * theta3 - 0.26756 * theta2 + 0.06670 * thetaS + 0.26688);

	return float3(Yz, xz, yz);
}

float3 CalculatePerezLuminanceYxy(in float theta, in float gamma, in float3 A, in float3 B, in float3 C, in float3 D, in float3 E)
{
	return (1.0 + A * exp(B / cos(theta))) * (1.0 + C * exp(D * gamma) + E * cos(gamma) * cos(gamma));
}

float3 CalculateSkyLuminanceRGB(in float3 s, in float3 e, in float t)
{
	float3 A, B, C, D, E;
	CalculatePerezDistribution(t, A, B, C, D, E);

	float thetaS = acos(SaturatedDot(s, float3(0, 1, 0)));
	float thetaE = acos(SaturatedDot(e, float3(0, 1, 0)));
	float gammaE = acos(SaturatedDot(s, e));

	float3 Yz = CalculateZenithLuminanceYxy(t, thetaS);

	float3 fThetaGamma = CalculatePerezLuminanceYxy(thetaE, gammaE, A, B, C, D, E);
	float3 fZeroThetaS = CalculatePerezLuminanceYxy(0.0, thetaS, A, B, C, D, E);

	float3 Yp = Yz * (fThetaGamma / fZeroThetaS);

	return YxyToRGB(Yp);
}

[numthreads(32, 32, 1)]
void main(uint3 dispatchId : SV_DispatchThreadID)
{
	const float3 cubeTC = GetCubeMapTexCoord(dispatchId);

    const float3 sunDir = normalize(float3(sin(u_pushConstants.inclination) * cos(u_pushConstants.azimuth), cos(u_pushConstants.inclination), sin(u_pushConstants.inclination) * sin(u_pushConstants.azimuth)));
    const float3 skyLuminance = CalculateSkyLuminanceRGB(sunDir, cubeTC, u_pushConstants.turbidity);

	const float4 color = float4(skyLuminance * 0.05f, 1.f);
	o_output[dispatchId] = color;
}