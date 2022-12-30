static const float m_hdrRange = 10.0f;

// Encode HDR to a 32 bit uint
// Alpha is 1 bit + 7 bit HDR remapping
uint PackVoxelColor(in float4 color)
{
	// Normalize color to LDR
	float hdr = length(color.rgb);
	color.rgb /= hdr;

	// Encode LDR color and HDR range
	uint3 iColor = uint3(color.rgb * 255.f);
	uint iHDR = (uint)(saturate(hdr / m_hdrRange) * 127);
	uint colorMask = (iHDR << 24u) | (iColor.r << 16u) | (iColor.g << 8u) | iColor.b;

	// Encode alpha into highest bit
	uint iAlpha = (color.a > 0 ? 1u : 0u);
	colorMask |= iAlpha << 31u;

	return colorMask;
}

inline uint PackUnitVector(in float3 value)
{
	uint retVal = 0;
	retVal |= (uint)((value.x * 0.5 + 0.5) * 255.0) << 0u;
	retVal |= (uint)((value.y * 0.5 + 0.5) * 255.0) << 8u;
	retVal |= (uint)((value.z * 0.5 + 0.5) * 255.0) << 16u;
	return retVal;
}

// 3D array index to flattened 1D array index
inline uint Flatten3D(uint3 coord, uint3 dim)
{
	return (coord.z * dim.x * dim.y) + (coord.y * dim.x) + coord.x;
}

cbuffer Data : register(b13)
{
	float3 u_center;
	float u_size;

	float2 u_resolution;
}