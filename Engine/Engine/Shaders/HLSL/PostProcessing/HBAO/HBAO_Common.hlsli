cbuffer HBAOData : register(b13)
{
	int u_uvOffsetIndex;
	float u_negInvR2;
	float u_multiplier;
	float u_powExponent;

	float4 u_float2Offsets[16];
	float4 u_jitters[16];
	float4 u_perspectiveInfo;
	
	float2 u_inverseQuarterSize;
	float u_radiusToScreen;
	float u_NdotVBias;

	float2 u_invResDirection;
	float u_sharpness;
	float padding;
}