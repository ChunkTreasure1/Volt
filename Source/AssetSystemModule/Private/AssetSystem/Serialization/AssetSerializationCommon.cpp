#include "aspch.h"
#include "Serialization/AssetSerializationCommon.h"

#include <CoreUtilities/FileIO/BinaryStreamWriter.h>
#include <CoreUtilities/FileIO/BinaryStreamReader.h>

namespace Volt
{
	void SerializedAssetMetadata::Serialize(BinaryStreamWriter& streamWriter, const SerializedAssetMetadata& data)
	{
		streamWriter.Write(data.handle);
		streamWriter.Write(data.type->GetGUID());
		streamWriter.Write(data.version);
	}

	void SerializedAssetMetadata::Deserialize(BinaryStreamReader& streamReader, SerializedAssetMetadata& outData)
	{
		VoltGUID guid{};

		streamReader.Read(outData.handle);
		streamReader.Read(guid);
		streamReader.Read(outData.version);

		outData.type = GetAssetTypeRegistry().GetTypeFromGUID(guid);
	}
}
