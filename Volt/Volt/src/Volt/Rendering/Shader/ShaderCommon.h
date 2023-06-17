#pragma once

#include "Volt/Core/Base.h"

namespace Volt
{
	enum class ShaderStage : uint32_t // Mapped to VkShaderStageFlagBits
	{
		None = 0,
		Vertex = 0x00000001,
		Pixel = 0x00000010,
		Hull = 0x00000002,
		Domain = 0x00000004,
		Geometry = 0x00000008,
		Compute = 0x00000020,

		All = Vertex | Pixel | Hull | Domain | Geometry | Compute,
		Common = Vertex | Pixel | Geometry | Compute
	};

	VT_SETUP_ENUM_CLASS_OPERATORS(ShaderStage);

	namespace Sets
	{
		inline static constexpr uint32_t TEXTURES = 0;
		inline static constexpr uint32_t MAINBUFFERS = 1;
		inline static constexpr uint32_t SAMPLERS = 2;
		inline static constexpr uint32_t RENDERER_BUFFERS = 3;
		inline static constexpr uint32_t OTHER = 4;
		inline static constexpr uint32_t PBR_RESOURCES = 5;
		inline static constexpr uint32_t MATERIAL = 6;
		inline static constexpr uint32_t DRAW_BUFFERS = 7;
	}

	namespace Bindings
	{
		// Space 0
		inline static constexpr uint32_t TEXTURE2DTABLE = 0;
		inline static constexpr uint32_t TEXTURECUBETABLE = 1;
		inline static constexpr uint32_t TEXTURE3DTABLE = 2;

		// Space 1
		inline static constexpr uint32_t MATERIAL_TABLE = 0;
		inline static constexpr uint32_t MESH_TABLE = 1;

		// Space 2
		inline static constexpr uint32_t SAMPLER_LINEAR = 0;
		inline static constexpr uint32_t SAMPLER_LINEAR_POINT = 1;
		inline static constexpr uint32_t SAMPLER_POINT = 2;
		inline static constexpr uint32_t SAMPLER_POINT_LINEAR = 3;

		inline static constexpr uint32_t SAMPLER_LINEAR_CLAMP = 4;
		inline static constexpr uint32_t SAMPLER_LINEAR_POINT_CLAMP = 5;
		inline static constexpr uint32_t SAMPLER_POINT_CLAMP = 6;
		inline static constexpr uint32_t SAMPLER_POINT_LINEAR_CLAMP = 7;

		inline static constexpr uint32_t SAMPLER_ANISO = 8;
		inline static constexpr uint32_t SAMPLER_SHADOW = 9;
		inline static constexpr uint32_t SAMPLER_REDUCE = 10;

		// Space 3
		inline static constexpr uint32_t CAMERA_BUFFER = 1;
		inline static constexpr uint32_t MAIN_INDIRECT_ARGS = 2;
		inline static constexpr uint32_t OBJECT_DATA = 4;
		inline static constexpr uint32_t DIRECTIONAL_LIGHT = 6;
		inline static constexpr uint32_t POINT_LIGHTS = 7;
		inline static constexpr uint32_t SCENE_DATA = 8;
		inline static constexpr uint32_t SPOT_LIGHTS = 9;
		inline static constexpr uint32_t ANIMATION_DATA = 10;
		inline static constexpr uint32_t SPHERE_LIGHTS = 11;
		inline static constexpr uint32_t RECTANGLE_LIGHTS = 12;
		inline static constexpr uint32_t PARTICLE_INFO = 13;
		inline static constexpr uint32_t VISIBLE_POINT_LIGHTS = 14;
		inline static constexpr uint32_t RENDERER_DATA = 15;
		inline static constexpr uint32_t PAINTED_VERTEX_COLORS = 16;
		inline static constexpr uint32_t VISIBLE_SPOT_LIGHTS = 17;

		// Space 5
		inline static constexpr uint32_t BRDF = 0;
		inline static constexpr uint32_t IRRADIANCE = 1;
		inline static constexpr uint32_t RADIANCE = 2;
		inline static constexpr uint32_t DIR_SHADOWMAP = 3;
		inline static constexpr uint32_t SPOT_SHADOWMAPS = 4;
		inline static constexpr uint32_t POINT_SHADOWMAPS = 5;
		inline static constexpr uint32_t VOLUMETRIC_FOG_TEXTURE = 6;
		inline static constexpr uint32_t AO_TEXTURE = 7;

		// Space 7
		inline static constexpr uint32_t INDIRECT_COUNTS = 0;
		inline static constexpr uint32_t DRAW_TO_OBJECT_ID = 1;
	}
}
