#include "vtpch.h"
#include "Volt/Asset/Serializers/ParticlePresetSerializer.h"

#include <AssetSystem/AssetManager.h>
#include "Volt/Asset/ParticlePreset.h"

namespace Volt
{
	struct ParticlePresetSerializationData
	{
		bool looping;
		float emittionTime;
		float intensity;
		float minLifetime;
		float maxLifetime;

		AssetHandle textureHandle;
		AssetHandle materialHandle;

		glm::vec3 direction;
		glm::vec3 gravity;

		uint32_t shape;
		float coneInnerRadius;
		float coneOuterRadius;
		bool coneSpawnOnEdge;

		float sphereRadius;
		bool sphereSpawnOnEdge;

		float startVelocity;
		float endVelocity;

		Vector<glm::vec3> sizes;

		bool isBurst;
		float burstInterval;
		float burstLength;

		AssetHandle meshHandle;
		AssetHandle meshMaterial;
		ParticlePreset::eType type;

		Vector<glm::vec4> colors;
	
		static void Serialize(BinaryStreamWriter& streamWriter, const ParticlePresetSerializationData& data)
		{
			streamWriter.Write(data.looping);
			streamWriter.Write(data.emittionTime);
			streamWriter.Write(data.intensity);
			streamWriter.Write(data.minLifetime);
			streamWriter.Write(data.maxLifetime);
			streamWriter.Write(data.textureHandle);
			streamWriter.Write(data.materialHandle);
			streamWriter.Write(data.direction);
			streamWriter.Write(data.gravity);
			streamWriter.Write(data.shape);
			streamWriter.Write(data.coneInnerRadius);
			streamWriter.Write(data.coneOuterRadius);
			streamWriter.Write(data.coneSpawnOnEdge);
			streamWriter.Write(data.sphereRadius);
			streamWriter.Write(data.sphereSpawnOnEdge);
			streamWriter.Write(data.startVelocity);
			streamWriter.Write(data.endVelocity);
			streamWriter.Write(data.sizes);
			streamWriter.Write(data.isBurst);
			streamWriter.Write(data.burstInterval);
			streamWriter.Write(data.burstLength);
			streamWriter.Write(data.meshHandle);
			streamWriter.Write(data.meshMaterial);
			streamWriter.Write(data.type);
			streamWriter.Write(data.colors);
		}

		static void Deserialize(BinaryStreamReader& streamReader, ParticlePresetSerializationData& outData)
		{
			streamReader.Read(outData.looping);
			streamReader.Read(outData.emittionTime);
			streamReader.Read(outData.intensity);
			streamReader.Read(outData.minLifetime);
			streamReader.Read(outData.maxLifetime);
			streamReader.Read(outData.textureHandle);
			streamReader.Read(outData.materialHandle);
			streamReader.Read(outData.direction);
			streamReader.Read(outData.gravity);
			streamReader.Read(outData.shape);
			streamReader.Read(outData.coneInnerRadius);
			streamReader.Read(outData.coneOuterRadius);
			streamReader.Read(outData.coneSpawnOnEdge);
			streamReader.Read(outData.sphereRadius);
			streamReader.Read(outData.sphereSpawnOnEdge);
			streamReader.Read(outData.startVelocity);
			streamReader.Read(outData.endVelocity);
			streamReader.Read(outData.sizes);
			streamReader.Read(outData.isBurst);
			streamReader.Read(outData.burstInterval);
			streamReader.Read(outData.burstLength);
			streamReader.Read(outData.meshHandle);
			streamReader.Read(outData.meshMaterial);
			streamReader.Read(outData.type);
			streamReader.Read(outData.colors);
		}
	};

	void ParticlePresetSerializer::Serialize(const AssetMetadata& metadata, const Ref<Asset>& asset) const
	{
		Ref<ParticlePreset> particlePreset = std::reinterpret_pointer_cast<ParticlePreset>(asset);

		ParticlePresetSerializationData serializationData{};
		serializationData.looping = particlePreset->isLooping;
		serializationData.emittionTime = particlePreset->emittionTime;

		serializationData.intensity = particlePreset->intensity;
		serializationData.minLifetime = particlePreset->minLifeTime;
		serializationData.maxLifetime = particlePreset->maxLifeTime;

		serializationData.textureHandle = particlePreset->texture;
		serializationData.materialHandle = particlePreset->material;

		serializationData.direction = particlePreset->direction;
		serializationData.gravity = particlePreset->gravity;
		
		serializationData.shape = particlePreset->shape;
		serializationData.coneInnerRadius = particlePreset->coneInnerRadius;
		serializationData.coneOuterRadius = particlePreset->coneOuterRadius;
		serializationData.coneSpawnOnEdge = particlePreset->coneSpawnOnEdge;

		serializationData.sphereRadius = particlePreset->sphereRadius;
		serializationData.sphereSpawnOnEdge = particlePreset->sphereSpawnOnEdge;

		serializationData.startVelocity = particlePreset->startVelocity;
		serializationData.endVelocity = particlePreset->endVelocity;

		serializationData.sizes = particlePreset->sizes;

		serializationData.isBurst = particlePreset->isBurst;
		serializationData.burstInterval = particlePreset->burstInterval;
		serializationData.burstLength = particlePreset->burstLength;
		
		serializationData.meshHandle = particlePreset->mesh;
		serializationData.meshMaterial = particlePreset->material;
		serializationData.type = particlePreset->type;

		serializationData.colors = particlePreset->colors;

		BinaryStreamWriter streamWriter{};
		const size_t compressedDataOffset = AssetSerializer::WriteMetadata(metadata, asset->GetVersion(), streamWriter);

		streamWriter.Write(serializationData);

		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);
		streamWriter.WriteToDisk(filePath, true, compressedDataOffset);
	}

	bool ParticlePresetSerializer::Deserialize(const AssetMetadata& metadata, Ref<Asset> destinationAsset) const
	{
		const auto filePath = AssetManager::GetFilesystemPath(metadata.filePath);

		if (!std::filesystem::exists(filePath))
		{
			VT_LOG(Error, "File {0} not found!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Missing, true);
			return false;
		}

		BinaryStreamReader streamReader{ filePath };

		if (!streamReader.IsStreamValid())
		{
			VT_LOG(Error, "Failed to open file: {0}!", metadata.filePath);
			destinationAsset->SetFlag(AssetFlag::Invalid, true);
			return false;
		}

		SerializedAssetMetadata serializedMetadata = AssetSerializer::ReadMetadata(streamReader);
		VT_ASSERT_MSG(serializedMetadata.version == destinationAsset->GetVersion(), "Incompatible version!");

		ParticlePresetSerializationData serializationData{};
		streamReader.Read(serializationData);

		Ref<ParticlePreset> preset = std::reinterpret_pointer_cast<ParticlePreset>(destinationAsset);

		preset->isLooping = serializationData.looping;
		preset->emittionTime = serializationData.emittionTime;

		preset->intensity = serializationData.intensity;
		preset->minLifeTime = serializationData.minLifetime;
		preset->maxLifeTime = serializationData.maxLifetime;

		preset->texture = serializationData.textureHandle;
		preset->material = serializationData.materialHandle;
		
		preset->direction = serializationData.direction;
		preset->gravity = serializationData.gravity;
		
		preset->shape = serializationData.shape;
		preset->coneInnerRadius = serializationData.coneInnerRadius;
		preset->coneOuterRadius = serializationData.coneOuterRadius;
		preset->coneSpawnOnEdge = serializationData.coneSpawnOnEdge;
		
		preset->sphereRadius = serializationData.sphereRadius;
		preset->sphereSpawnOnEdge = serializationData.sphereSpawnOnEdge;
		
		preset->startVelocity = serializationData.startVelocity;
		preset->endVelocity = serializationData.endVelocity;

		preset->colors = serializationData.colors;

		preset->isBurst = serializationData.isBurst;
		preset->burstInterval = serializationData.burstInterval;
		preset->burstLength = serializationData.burstLength;

		preset->mesh = serializationData.meshHandle;
		preset->material = serializationData.materialHandle;

		preset->type = serializationData.type;

		preset->sizes = serializationData.sizes;

		if (preset->colors.size() == 0)
		{
			preset->colors.push_back({ 1, 0, 0, 1 });
		}
		if (preset->sizes.size() == 0)
		{
			preset->sizes.push_back({ 0.1f, 0.1f, 0.1f });
		}

		return true;
	}
}
