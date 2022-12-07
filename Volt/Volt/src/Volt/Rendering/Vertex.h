#pragma once

#include <GEM/gem.h>

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
		{}

		Vertex(const gem::vec3& aPosition, const gem::vec2& aTexCoords)
			: position(aPosition)
		{
			texCoords[0] = aTexCoords;
		}

		bool operator==(const Vertex& aVert) const
		{
			const bool bPos = AbsEqualVector(position, aVert.position);
			const bool bNorm = AbsEqualVector(normal, aVert.normal);
			const bool bTangent = AbsEqualVector(normal, aVert.tangent);
			const bool bBitangent = AbsEqualVector(normal, aVert.bitangent);

			const bool bTex0 = AbsEqualVector(texCoords[0], aVert.texCoords[0]);
			const bool bTex1 = AbsEqualVector(texCoords[1], aVert.texCoords[1]);
			const bool bTex2 = AbsEqualVector(texCoords[2], aVert.texCoords[2]);
			const bool bTex3 = AbsEqualVector(texCoords[3], aVert.texCoords[3]);

			const bool bCol0 = AbsEqualVector(color[0], aVert.color[0]);
			const bool bCol1 = AbsEqualVector(color[1], aVert.color[1]);
			const bool bCol2 = AbsEqualVector(color[2], aVert.color[2]);
			const bool bCol3 = AbsEqualVector(color[3], aVert.color[3]);

			const bool bInfluences = AbsEqualVector(influences, aVert.influences);
			const bool bWeights = AbsEqualVector(weights, aVert.weights);

			return bPos && bNorm && 
				bTex0 && bTex1 && bTex2 && bTex3 && 
				bTangent && bBitangent &&
				bCol0 && bCol1 && bCol2 && bCol3 &&
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
		gem::vec3 bitangent = gem::vec3(0.f);

		gem::vec2 texCoords[4] = { { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f }, { 0.f, 0.f } };
		gem::vec4 color[4] = { { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f } };

		gem::vec4ui influences = { 0, 0, 0, 0 };
		gem::vec4 weights = { 0.f, 0.f, 0.f, 0.f };
	};

	struct SpriteVertex
	{
		gem::vec4 position = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
		gem::vec2 texCoords = gem::vec2(0.f);
		uint32_t textureIndex = 0;
		uint32_t id = 0;
	};

	struct BillboardVertex
	{
		gem::vec4 postition = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
		gem::vec3 scale = gem::vec3(1.f);
		uint32_t textureIndex = 0;
		uint32_t id = 0;
	};

	struct LineVertex
	{
		gem::vec4 position = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
	};

	struct TextVertex
	{
		gem::vec4 position = gem::vec4(0.f);
		gem::vec4 color = gem::vec4(1.f);
		gem::vec2 texCoords = gem::vec2(0.f);
		uint32_t textureIndex = 0;
	};
}