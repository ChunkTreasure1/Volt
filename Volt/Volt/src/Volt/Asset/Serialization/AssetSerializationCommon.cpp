#include "vtpch.h"
#include "AssetSerializationCommon.h"

#include "Volt/Utility/FileIO/BinaryStreamWriter.h"

namespace Volt
{
	void SerializedAssetMetadata::Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data)
	{
		streamWriter.Write(data.handle);
		streamWriter.Write(data.type);
		streamWriter.Write(data.version);
	}
}
