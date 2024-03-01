#include "vtpch.h"
#include "AssetSerializationCommon.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"
#include "Volt/Utility/FileIO/BinaryStreamReader.h"

namespace Volt
{
	void SerializedAssetMetadata::Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data)
	{
		streamWriter.Write(data.handle);
		streamWriter.Write(data.type);
		streamWriter.Write(data.version);
	}

	void SerializedAssetMetadata::Deserialize(BinaryStreamReader& streamReader, SerializedAssetMetadata& outData)
	{
		streamReader.Read(outData.handle);
		streamReader.Read(outData.type);
		streamReader.Read(outData.version);
	}
}
