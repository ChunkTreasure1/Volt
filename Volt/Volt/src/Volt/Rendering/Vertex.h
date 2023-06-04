#pragma once

#include "Volt/Rendering/Buffer/BufferLayout.h"

#include <gem/gem.h>
#include <half/half.hpp>

namespace Volt
{
	static bool AbsEqualVector(const gem::vec3& aFirst, const gem::vec3& aSecond)
	{
		return std::abs(aFirst.x - aSecond.x) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.y - aSecond.y) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.z - aSecond.z) < std::numeric_limits<float>::epsilon();
	}

	static bool AbsEqualVector(const gem::vec2& aFirst, const gem::vec2& aSecond)
	{
		return std::abs(aFirst.x - aSecond.x) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.y - aSecond.y) < std::numeric_limits<float>::epsilon();
	}

	static bool AbsEqualVector(const gem::vec4& aFirst, const gem::vec4& aSecond)
	{
		return std::abs(aFirst.x - aSecond.x) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.y - aSecond.y) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.z - aSecond.z) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.w - aSecond.w) < std::numeric_limits<float>::epsilon();
	}

	static bool AbsEqualVector(const gem::vec4ui& aFirst, const gem::vec4ui& aSecond)
	{
		return aFirst == aSecond;
	}

	struct Vertex
	{
		Vertex() = default;

		Vertex(const gem::vec3& aPosition)
			: position(aPosition)
		{
		}

		Vertex(const gem::vec3& aPosition, const gem::vec2& aTexCoords)
			: position(aPosition)
		{
			texCoords = aTexCoords;
		}

		bool operator==(const Vertex& aVert) const
		{
			const bool bPos = AbsEqualVector(position, aVert.position);
			const bool bNorm = AbsEqualVector(normal, aVert.normal);
			const bool bTangent = AbsEqualVector(normal, aVert.tangent);

			const bool bTex0 = AbsEqualVector(texCoords, aVert.texCoords);

			const bool bInfluences = AbsEqualVector(influences, aVert.influences);
			const bool bWeights = AbsEqualVector(weights, aVert.weights);

			return bPos && bNorm && 
				bTex0 &&
				bTangent &&
				bWeights && bInfluences;
		}

		bool ComparePosition(const Vertex& vert) const
		{
			bool bPos = AbsEqualVector(position, vert.position);

			return bPos;
		}

		gem::vec3 position = gem::vec3(0.f);
		gem::vec3 normal = gem::vec3(0.f);
		gem::vec3 tangent = gem::vec3(0.f);

		gem::vec2 texCoords{ 0.f };

		gem::vec4ui influences = { 0, 0, 0, 0 };
		gem::vec4 weights = { 0.f, 0.f, 0.f, 0.f };

		inline static const BufferLayout GetVertexLayout()
		{
			BufferLayout result = 
			{
				{ ElementType::Float3, "POSITION" },
				
				{ ElementType::Byte4, "NORMAL" },
				{ ElementType::Float, "TANGENT" },

				{ ElementType::Half2, "TEXCOORDS" },

				{ ElementType::UShort4, "INFLUENCES" },
				{ ElementType::Half4, "WEIGHTS" }
			};

			return result;
		}
	};

	struct EncodedVertex
	{
		gem::vec3 position = 0.f;

		gem::vec<4, uint8_t> normal;
		float tangent = 0.f;

		gem::vec<2, half_float::half> texCoords;

		gem::vec<4, uint16_t> influences = 0;
		gem::vec<4, half_float::half> weights = half_float::half(0.f);
	};

	struct SpriteVertex
	{
		gem::vec4 position = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
		gem::vec2 texCoords = gem::vec2(0.f);
	};

	struct BillboardVertex
	{
		gem::vec4 postition = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
		gem::vec3 scale = gem::vec3(1.f);
		uint32_t textureIndex = 0;
		uint32_t id = 0;

		inline static const BufferLayout GetVertexLayout()
		{
			BufferLayout result =
			{
				{ ElementType::Float4, "POSITION" },
				{ ElementType::Float4, "COLOR" },
				{ ElementType::Float3, "SCALE" },
				{ ElementType::UInt, "TEXINDEX" },
				{ ElementType::UInt, "ID" },
			};

			return result;
		}
	};

	struct LineVertex
	{
		gem::vec4 position = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);

		inline static const BufferLayout GetVertexLayout()
		{
			BufferLayout result =
			{
				{ ElementType::Float4, "POSITION" },
				{ ElementType::Float4, "COLOR" },
			};

			return result;
		}
	};

	struct TextVertex
	{
		gem::vec4 position = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
		gem::vec2 texCoords = gem::vec2(0.f);
		uint32_t textureIndex = 0;

		inline static const BufferLayout GetVertexLayout()
		{
			BufferLayout result =
			{
				{ ElementType::Float4, "POSITION" },
				{ ElementType::Float4, "COLOR" },
				{ ElementType::Float2, "TEXCOORDS" },
				{ ElementType::UInt, "TEXINDEX" }
			};

			return result;
		}
	};
}
