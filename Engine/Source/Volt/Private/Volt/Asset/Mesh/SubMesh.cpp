#include "vtpch.h"
#include "Volt/Asset/Mesh/SubMesh.h"

#include <CoreUtilities/UUID.h>

#include <CoreUtilities/Math/Hash.h>
#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

namespace Volt
{
	SubMesh::SubMesh(uint32_t aMaterialIndex, uint32_t aVertexCount, uint32_t aIndexCount, uint32_t aVertexStartOffset, uint32_t aIndexStartOffset)
		: materialIndex(aMaterialIndex), vertexCount(aVertexCount), indexCount(aIndexCount), vertexStartOffset(aVertexStartOffset), indexStartOffset(aIndexStartOffset)
	{
		GenerateHash();
	}

	void SubMesh::GenerateHash()
	{
		m_hash = Math::HashCombine(m_hash, std::hash<uint32_t>()(materialIndex));
		m_hash = Math::HashCombine(m_hash, std::hash<uint32_t>()(vertexCount));
		m_hash = Math::HashCombine(m_hash, std::hash<uint32_t>()(indexCount));
		m_hash = Math::HashCombine(m_hash, std::hash<uint32_t>()(vertexStartOffset));
		m_hash = Math::HashCombine(m_hash, std::hash<uint32_t>()(indexStartOffset));

		using namespace std::chrono;
		auto time = std::chrono::system_clock::now();
		uint64_t count = duration_cast<milliseconds>(time.time_since_epoch()).count();

		m_hash = Math::HashCombine(m_hash, std::hash<uint64_t>()(count));
		m_hash = Math::HashCombine(m_hash, std::hash<uint64_t>()(UUID64()));
	}

	const bool SubMesh::operator==(const SubMesh& rhs) const
	{
		return m_hash == rhs.m_hash;
	}

	const bool SubMesh::operator!=(const SubMesh& rhs) const
	{
		return m_hash != rhs.m_hash;
	}

	void SubMesh::Serialize(BinaryStreamWriter& streamWriter, const SubMesh& data)
	{
		streamWriter.Write(data.materialIndex);
		streamWriter.Write(data.vertexCount);
		streamWriter.Write(data.indexCount);
		streamWriter.Write(data.vertexStartOffset);
		streamWriter.Write(data.indexStartOffset);
		streamWriter.Write(data.transform);
		streamWriter.Write(data.name);
	}

	void SubMesh::Deserialize(BinaryStreamReader& streamReader, SubMesh& outData)
	{
		streamReader.Read(outData.materialIndex);
		streamReader.Read(outData.vertexCount);
		streamReader.Read(outData.indexCount);
		streamReader.Read(outData.vertexStartOffset);
		streamReader.Read(outData.indexStartOffset);
		streamReader.Read(outData.transform);
		streamReader.Read(outData.name);
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
