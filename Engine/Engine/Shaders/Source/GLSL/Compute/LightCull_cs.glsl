#version 450 core

#extension GL_EXT_debug_printf : enable

#include "Buffers.glsl" //! #include "../Includes/Buffers.glsl"
#include "SamplerStates.glsl" //! #include "../Includes/SamplerStates.glsl"

layout(set = SPACE_OTHER, binding = 0) uniform texture2D u_DepthMap;

#define TILE_SIZE 16
#define MAX_LIGHT_COUNT 1024

layout(push_constant) uniform PushConstantObject
{
	ivec2 targetSize;
	vec2 depthUnpackConsts;
	
} u_cullData;

// From XeGTAO
float ScreenSpaceToViewSpaceDepth(const float screenDepth)
{
	float depthLinearizeMul = u_cameraData.depthUnpackConsts.x;
	float depthLinearizeAdd = u_cameraData.depthUnpackConsts.y;
	// Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"
	return depthLinearizeMul / (depthLinearizeAdd - screenDepth);
}
// Shared values between all the threads in the group
shared uint minDepthInt;
shared uint maxDepthInt;
shared uint visiblePointLightCount;
shared uint visibleSpotLightCount;
shared vec4 frustumPlanes[6];

// Shared local storage for visible indices, will be written out to the global buffer at the end
shared int visiblePointLightIndices[MAX_LIGHT_COUNT];
shared int visibleSpotLightIndices[MAX_LIGHT_COUNT];

layout(local_size_x = TILE_SIZE, local_size_y = TILE_SIZE, local_size_z = 1) in;
void main()
{
    ivec2 location = ivec2(gl_GlobalInvocationID.xy);
    ivec2 itemID = ivec2(gl_LocalInvocationID.xy);
    ivec2 tileID = ivec2(gl_WorkGroupID.xy);
    ivec2 tileNumber = ivec2(gl_NumWorkGroups.xy);
    uint index = tileID.y * tileNumber.x + tileID.x;

    // Initialize shared global values for depth and light count
    if (gl_LocalInvocationIndex == 0)
    {
		minDepthInt = 0xFFFFFFFF;
		maxDepthInt = 0;
		visiblePointLightCount = 0;
		visibleSpotLightCount = 0;
    }

    barrier();

    // Step 1: Calculate the minimum and maximum depth values (from the depth buffer) for this group's tile
    vec2 tc = vec2(location) / u_cullData.targetSize;
    float linearDepth = textureLod(sampler2D(u_DepthMap, u_pointSampler), tc, 0).r;

	linearDepth = ScreenSpaceToViewSpaceDepth(linearDepth);

    // Convert depth to uint so we can do atomic min and max comparisons between the threads
    uint depthInt = floatBitsToUint(linearDepth);
    atomicMin(minDepthInt, depthInt);
    atomicMax(maxDepthInt, depthInt);

    barrier();

    // Step 2: One thread should calculate the frustum planes to be used for this tile
    if (gl_LocalInvocationIndex == 0)
    {
		// Convert the min and max across the entire tile back to float
		float minDepth = uintBitsToFloat(minDepthInt);
		float maxDepth = uintBitsToFloat(maxDepthInt);

		// Steps based on tile sale
		vec2 negativeStep = (2.0 * vec2(tileID)) / vec2(tileNumber);
		vec2 positiveStep = (2.0 * vec2(tileID + ivec2(1, 1))) / vec2(tileNumber);

		// Set up starting values for planes using steps and min and max z values
		frustumPlanes[0] = vec4(1.0, 0.0, 0.0, 1.0 - negativeStep.x); // Left
		frustumPlanes[1] = vec4(-1.0, 0.0, 0.0, -1.0 + positiveStep.x); // Right
		frustumPlanes[2] = vec4(0.0, 1.0, 0.0, -1.0 + negativeStep.y); // Bottom
		frustumPlanes[3] = vec4(0.0, -1.0, 0.0, 1.0 - positiveStep.y); // Top
		frustumPlanes[4] = vec4(0.0, 0.0, 1.0, minDepth); // Near
		frustumPlanes[5] = vec4(0.0, 0.0, -1.0, maxDepth); // Far

		// Transform the first four planes
		for (uint i = 0; i < 4; i++)
		{
		    frustumPlanes[i] *= u_cameraData.viewProjection;
		    frustumPlanes[i] /= length(frustumPlanes[i].xyz);
		}

		// Transform the depth planes
		frustumPlanes[4] *= u_cameraData.view;
		frustumPlanes[4] /= length(frustumPlanes[4].xyz);
		frustumPlanes[5] *= u_cameraData.view;
		frustumPlanes[5] /= length(frustumPlanes[5].xyz);

    }

    barrier();

    // Step 3: Cull lights.
    // Parallelize the threads against the lights now.
    // Can handle 256 simultaniously. Anymore lights than that and additional passes are performed
    const uint threadCount = TILE_SIZE * TILE_SIZE;
    uint passCount = (u_sceneData.pointLightCount + threadCount - 1) / threadCount;
    for (uint i = 0; i < passCount; i++)
    {
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
		if (lightIndex >= u_sceneData.pointLightCount)
		    break;

		vec4 position = vec4(vec3(u_pointLights[lightIndex].position), 1.0f);
		float radius = u_pointLights[lightIndex].radius;
		radius += radius * 0.2f;

		// Check if light radius is in frustum

		float distance = 0.0;
		for (uint j = 0; j < 6; j++)
		{
		    distance = dot(position, frustumPlanes[j]) + radius;
		    if (distance <= 0.0) // No intersection
			{
				break;
			}
		}

		// If greater than zero, then it is a visible light
		if (distance > 0.0)
		{
		    // Add index to the shared array of visible indices
		    uint offset = atomicAdd(visiblePointLightCount, 1);
		    visiblePointLightIndices[offset] = int(lightIndex);
		}
    }
	passCount = (u_sceneData.spotLightCount + threadCount - 1) / threadCount;
	for (uint i = 0; i < passCount; i++)
	{
		// Get the lightIndex to test for this thread / pass. If the index is >= light count, then this thread can stop testing lights
		uint lightIndex = i * threadCount + gl_LocalInvocationIndex;
		if (lightIndex >= u_sceneData.spotLightCount)
			break;

		SpotLight light = u_spotLights[lightIndex];
		float radius = light.range;
		// Check if light radius is in frustum
		float distance = 0.0;
		for (uint j = 0; j < 6; j++)
		{
			distance = dot(vec4(light.position - light.direction * (light.range * 0.7), 1.0), frustumPlanes[j]) + radius * 1.3;
			if (distance < 0.0) // No intersection
				break;
		}

		// If greater than zero, then it is a visible light
		if (distance > 0.0)
		{
			// Add index to the shared array of visible indices
			uint offset = atomicAdd(visibleSpotLightCount, 1);
			visibleSpotLightIndices[offset] = int(lightIndex);
		} 
		
	}

    barrier();

    // One thread should fill the global light buffer
    if (gl_LocalInvocationIndex == 0)
    {
		const uint offset = index * MAX_LIGHT_COUNT; // Determine position in global buffer
		for (uint i = 0; i < visiblePointLightCount; i++) 
		{
			u_visiblePointLightIndices[offset + i] = visiblePointLightIndices[i];
		}

		for (uint i = 0; i < visibleSpotLightCount; i++) 
		{
			u_visibleSpotLightIndices[offset + i] = visibleSpotLightIndices[i];
		}

		if (visiblePointLightCount != MAX_LIGHT_COUNT)
		{
		    // Unless we have totally filled the entire array, mark it's end with -1
		    // Final shader step will use this to determine where to stop (without having to pass the light count)
			u_visiblePointLightIndices[offset + visiblePointLightCount] = -1;
		}

		if (visibleSpotLightCount != MAX_LIGHT_COUNT)
		{
			// Unless we have totally filled the entire array, mark it's end with -1
			// Final shader step will use this to determine where to stop (without having to pass the light count)
			u_visibleSpotLightIndices[offset + visibleSpotLightCount] = -1;
		}
    }
}