#pragma once

#include "Volt/Rendering/Buffer/BufferLayout.h"
#include <CoreUtilities/FileIO/BinaryStreamWriter.h>

#include <glm/glm.hpp>
#include <half/half.hpp>

namespace Volt
{
	static bool AbsEqualVector(const glm::vec3& aFirst, const glm::vec3& aSecond)
	{
		return std::abs(aFirst.x - aSecond.x) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.y - aSecond.y) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.z - aSecond.z) < std::numeric_limits<float>::epsilon();
	}

	static bool AbsEqualVector(const glm::vec2& aFirst, const glm::vec2& aSecond)
	{
		return std::abs(aFirst.x - aSecond.x) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.y - aSecond.y) < std::numeric_limits<float>::epsilon();
	}

	static bool AbsEqualVector(const glm::vec4& aFirst, const glm::vec4& aSecond)
	{
		return std::abs(aFirst.x - aSecond.x) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.y - aSecond.y) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.z - aSecond.z) < std::numeric_limits<float>::epsilon() &&
			std::abs(aFirst.w - aSecond.w) < std::numeric_limits<float>::epsilon();
	}

	static bool AbsEqualVector(const glm::uvec4& aFirst, const glm::uvec4& aSecond)
	{
		return aFirst == aSecond;
	}

	struct Vertex
	{
		Vertex() = default;

		Vertex(const glm::vec3& aPosition)
			: position(aPosition)
		{
		}

		Vertex(const glm::vec3& aPosition, const glm::vec2& aUV)
			: position(aPosition)
		{
			uv = aUV;
		}

		Vertex(const glm::vec3& aPosition, const glm::vec3& aNormal, const glm::vec3& aTangent, const glm::vec2& aUV)
			: position(aPosition), normal(aNormal), tangent(aTangent), uv(aUV)
		{ }

		bool operator==(const Vertex& aVert) const
		{
			const bool bPos = AbsEqualVector(position, aVert.position);
			const bool bNorm = AbsEqualVector(normal, aVert.normal);
			const bool bTangent = AbsEqualVector(normal, aVert.tangent);

			const bool bTex0 = AbsEqualVector(uv, aVert.uv);

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

		glm::vec3 position = glm::vec3(0.f);
		glm::vec3 normal = glm::vec3(0.f);
		glm::vec3 tangent = glm::vec3(0.f);

		glm::vec2 uv{ 0.f };

		glm::uvec4 influences = { 0, 0, 0, 0 };
		glm::vec4 weights = { 0.f, 0.f, 0.f, 0.f };

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
		glm::vec3 position = 0.f;

		glm::vec<4, uint8_t> normal;
		float tangent = 0.f;

		half_float::half texCoords[2] = { half_float::half(0.f), half_float::half(0.f) };

		glm::vec<4, uint16_t> influences = 0;
		half_float::half weights[4] = { half_float::half(0.f), half_float::half(0.f), half_float::half(0.f), half_float::half(0.f) };
	};

	struct SpriteVertex
	{
		glm::vec4 position = glm::vec4(0.f);
		glm::vec4 color = glm::vec4(1.f);
		glm::vec2 texCoords = glm::vec2(0.f);
	};

	struct BillboardVertex
	{
		glm::vec4 postition = glm::vec4(0.f);
		glm::vec4 color = glm::vec4(1.f);
		glm::vec3 scale = glm::vec3(1.f);
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
		glm::vec4 position = glm::vec4(0.f);
		glm::vec4 color = glm::vec4(1.f);

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
		glm::vec4 position = glm::vec4(0.f);
		glm::vec4 color = glm::vec4(1.f);
		glm::vec2 texCoords = glm::vec2(0.f);
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
