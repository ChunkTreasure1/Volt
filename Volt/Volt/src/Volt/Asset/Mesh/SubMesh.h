#pragma once

#include <cstdint>

#include <GEM/gem.h>

namespace Volt
{
	struct SubMesh
	{
		SubMesh(uint32_t aMaterialIndex, uint32_t aVertexCount, uint32_t aIndexCount, uint32_t aVertexStartOffset, uint32_t aIndexStartOffset);
		SubMesh() = default;

		void GenerateHash();

		bool operator==(const SubMesh& rhs);
		bool operator!=(const SubMesh& rhs);

		friend bool operator>(const SubMesh& lhs, const SubMesh& rhs);
		friend bool operator<(const SubMesh& lhs, const SubMesh& rhs);

		uint32_t materialIndex = 0;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		uint32_t vertexStartOffset = 0;
		uint32_t indexStartOffset = 0;

		gem::mat4 transform;
		std::string name;

	private:
		size_t m_hash = 0;
	};
}