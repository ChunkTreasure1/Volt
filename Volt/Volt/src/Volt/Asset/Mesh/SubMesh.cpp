#include "vtpch.h"
#include "SubMesh.h"

#include "Volt/Rendering/Shader/ShaderUtility.h"

namespace Volt
{
	SubMesh::SubMesh(uint32_t aMaterialIndex, uint32_t aVertexCount, uint32_t aIndexCount, uint32_t aVertexStartOffset, uint32_t aIndexStartOffset)
		: materialIndex(aMaterialIndex), vertexCount(aVertexCount), indexCount(aIndexCount), vertexStartOffset(aVertexStartOffset), indexStartOffset(aIndexStartOffset)
	{
		GenerateHash();
	}

	void SubMesh::GenerateHash()
	{
		m_hash = Utility::HashCombine(m_hash, std::hash<uint32_t>()(materialIndex));
		m_hash = Utility::HashCombine(m_hash, std::hash<uint32_t>()(vertexCount));
		m_hash = Utility::HashCombine(m_hash, std::hash<uint32_t>()(indexCount));
		m_hash = Utility::HashCombine(m_hash, std::hash<uint32_t>()(vertexStartOffset));
		m_hash = Utility::HashCombine(m_hash, std::hash<uint32_t>()(indexStartOffset));

		using namespace std::chrono;
		auto time = std::chrono::system_clock::now();
		uint64_t count = duration_cast<milliseconds>(time.time_since_epoch()).count();

		m_hash = Utility::HashCombine(m_hash, std::hash<uint64_t>()(count));
		m_hash = Utility::HashCombine(m_hash, std::hash<uint64_t>()(UUID()));
	}

	bool SubMesh::operator==(const SubMesh& rhs)
	{
		return m_hash == rhs.m_hash;
	}

	bool SubMesh::operator!=(const SubMesh& rhs)
	{
		return m_hash != rhs.m_hash;
	}

	bool operator>(const SubMesh& lhs, const SubMesh& rhs)
	{
		return lhs.m_hash > rhs.m_hash;
	}

	bool operator<(const SubMesh& lhs, const SubMesh& rhs)
	{
		return lhs.m_hash < rhs.m_hash;
	}
}