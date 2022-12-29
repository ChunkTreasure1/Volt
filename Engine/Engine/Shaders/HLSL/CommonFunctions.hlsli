#ifndef COMMONFUNCTIONS
#define COMMONFUNCTIONS

float3 ReconstructWorldPosition(float2 texCoords, float pixelDepth)
{
	float x = texCoords.x * 2.f - 1.f;
	float y = (1.f - texCoords.y) * 2.f - 1.f;

	const float4 projSpacePos = float4(x, y, pixelDepth, 1.f);
	float4 viewSpacePos = mul(u_cameraData.inverseProj, projSpacePos);

	viewSpacePos /= viewSpacePos.w;

	const float4 worldSpacePos = mul(u_cameraData.inverseView, viewSpacePos);
	return worldSpacePos.xyz;
}

float LinearizeDepth(const float screenDepth)
{
	return u_cameraData.nearPlane * u_cameraData.farPlane / (u_cameraData.farPlane + screenDepth * (u_cameraData.nearPlane - u_cameraData.farPlane));
}

float LinearizeDepth01(const float screenDepth)
{
	return (u_cameraData.nearPlane * u_cameraData.farPlane / (u_cameraData.farPlane + screenDepth * (u_cameraData.nearPlane - u_cameraData.farPlane))) / u_cameraData.farPlane;
}

float3 ClipToNDC(float4 position)
{
	return position.xyz / position.w;
}

float2 NDCToUV(float2 position)
{
	return position * 0.5f - 0.5f;

}

#endif