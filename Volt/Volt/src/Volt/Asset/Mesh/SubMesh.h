#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace Volt
{
	struct SubMesh
	{
		SubMesh(uint32_t aMaterialIndex, uint32_t aVertexCount, uint32_t aIndexCount, uint32_t aVertexStartOffset, uint32_t aIndexStartOffset);
		SubMesh() = default;

		void GenerateHash();

		inline const size_t GetHash() const { return m_hash; }

		const bool operator==(const SubMesh& rhs) const;
		const bool operator!=(const SubMesh& rhs) const;

		friend bool operator>(const SubMesh& lhs, const SubMesh& rhs);
		friend bool operator<(const SubMesh& lhs, const SubMesh& rhs);

		uint32_t materialIndex = 0;
		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		uint32_t vertexStartOffset = 0;
		uint32_t indexStartOffset = 0;

		uint32_t meshletStartOffset = 0;
		uint32_t meshletCount = 0;
		uint32_t meshletIndexStartOffset = 0;

		glm::mat4 transform = { 1.f };
		std::string name;

	private:
		size_t m_hash = 0;
	};
}
