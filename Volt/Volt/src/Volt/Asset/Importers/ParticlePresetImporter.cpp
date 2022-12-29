#include "vtpch.h"
#include "ParticlePresetImporter.h"
#include "Volt/Log/Log.h"
#include "Volt/Project/ProjectManager.h"

#include <yaml-cpp/yaml.h>
#include "Volt/Asset/ParticlePreset.h"
#include "Volt/Utility/SerializationMacros.h"
#include "Volt/Utility/YAMLSerializationHelpers.h"

bool Volt::ParticlePresetImporter::Load(const std::filesystem::path& path, Ref<Asset>& asset) const
{
	const auto filePath = ProjectManager::GetPath() / path;

	if (!std::filesystem::exists(filePath)) [[unlikely]]
	{
		VT_CORE_ERROR("File {0} not found!", path.string().c_str());
		asset->SetFlag(AssetFlag::Missing, true);
		return false;
	}

	std::ifstream file(filePath);
	if (!file.is_open()) [[unlikely]]
	{
		VT_CORE_ERROR("Failed to open file: {0}!", path.string().c_str());
		asset->SetFlag(AssetFlag::Invalid, true);
		return false;
	}

	asset = CreateRef<ParticlePreset>();
	Ref<ParticlePreset> preset = std::reinterpret_pointer_cast<ParticlePreset>(asset);

	std::stringstream sstream;
	sstream << file.rdbuf();

	YAML::Node root = YAML::Load(sstream.str());
	YAML::Node emitterNode = root["Emitter"];

	{
		VT_DESERIALIZE_PROPERTY(looping, preset->isLooping, emitterNode, true);
		VT_DESERIALIZE_PROPERTY(emittionTime, preset->emittionTime, emitterNode, 1.f);

		VT_DESERIALIZE_PROPERTY(intensity, preset->intensity, emitterNode, 0.f);
		VT_DESERIALIZE_PROPERTY(minLifeTime, preset->minLifeTime, emitterNode, 0.f);
		VT_DESERIALIZE_PROPERTY(maxLifeTime, preset->maxLifeTime, emitterNode, 0.f);

		VT_DESERIALIZE_PROPERTY(texture, preset->texture, emitterNode, Volt::AssetHandle(0));
		VT_DESERIALIZE_PROPERTY(shader, preset->shader, emitterNode, std::string(""));

		VT_DESERIALIZE_PROPERTY(direction, preset->direction, emitterNode, gem::vec3(0.f));
		VT_DESERIALIZE_PROPERTY(gravity, preset->gravity, emitterNode, gem::vec3(0.f));

		VT_DESERIALIZE_PROPERTY(shape, preset->shape, emitterNode, 0);
		VT_DESERIALIZE_PROPERTY(innerRadius, preset->coneInnerRadius, emitterNode, 0.f);
		VT_DESERIALIZE_PROPERTY(outerRadius, preset->coneOuterRadius, emitterNode, 0.f);
		VT_DESERIALIZE_PROPERTY(coneSpawnOnEdge, preset->coneSpawnOnEdge, emitterNode, false);

		VT_DESERIALIZE_PROPERTY(sphereRadius, preset->sphereRadius, emitterNode, 0.f);
		VT_DESERIALIZE_PROPERTY(sphereSpawnOnEdge, preset->sphereSpawnOnEdge, emitterNode, false);

		VT_DESERIALIZE_PROPERTY(startVelocity, preset->startVelocity, emitterNode, 0.f);
		VT_DESERIALIZE_PROPERTY(endVelocity, preset->endVelocity, emitterNode, 0.f);

		VT_DESERIALIZE_PROPERTY(startColor, preset->startColor, emitterNode, gem::vec4(1.f));
		VT_DESERIALIZE_PROPERTY(endColor, preset->endColor, emitterNode, gem::vec4(1.f));

		VT_DESERIALIZE_PROPERTY(startSize, preset->startSize, emitterNode, gem::vec3(0.f));
		VT_DESERIALIZE_PROPERTY(endSize, preset->endSize, emitterNode, gem::vec3(0.f));
	}
	return true;
}

void Volt::ParticlePresetImporter::Save(const Ref<Asset>& asset) const
{
	Ref<ParticlePreset> p = std::reinterpret_pointer_cast<ParticlePreset>(asset);
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Emitter" << YAML::Value;
	{
		out << YAML::BeginMap;

		VT_SERIALIZE_PROPERTY(looping, p->isLooping, out);
		VT_SERIALIZE_PROPERTY(emittionTime, p->emittionTime, out);

		VT_SERIALIZE_PROPERTY(intensity, p->intensity, out);
		VT_SERIALIZE_PROPERTY(minLifeTime, p->minLifeTime, out);
		VT_SERIALIZE_PROPERTY(maxLifeTime, p->maxLifeTime, out);

		VT_SERIALIZE_PROPERTY(texture, p->texture, out);
		VT_SERIALIZE_PROPERTY(shader, p->shader, out);

		VT_SERIALIZE_PROPERTY(direction, p->direction, out);
		VT_SERIALIZE_PROPERTY(gravity, p->gravity, out);

		VT_SERIALIZE_PROPERTY(shape, p->shape, out);
		VT_SERIALIZE_PROPERTY(innerRadius, p->coneInnerRadius, out);
		VT_SERIALIZE_PROPERTY(outerRadius, p->coneOuterRadius, out);
		VT_SERIALIZE_PROPERTY(coneSpawnOnEdge, p->coneSpawnOnEdge, out);

		VT_SERIALIZE_PROPERTY(sphereRadius, p->sphereRadius, out);
		VT_SERIALIZE_PROPERTY(sphereSpawnOnEdge, p->sphereSpawnOnEdge, out);

		VT_SERIALIZE_PROPERTY(startVelocity, p->startVelocity, out);
		VT_SERIALIZE_PROPERTY(endVelocity, p->endVelocity, out);

		VT_SERIALIZE_PROPERTY(startSize, p->startSize, out);
		VT_SERIALIZE_PROPERTY(endSize, p->endSize, out);

		VT_SERIALIZE_PROPERTY(startColor, p->startColor, out);
		VT_SERIALIZE_PROPERTY(endColor, p->endColor, out);

		out << YAML::EndMap;
	}
	out << YAML::EndMap;
	std::ofstream fout(ProjectManager::GetPath() / asset->path);
	fout << out.c_str();
	fout.close();
}

void Volt::ParticlePresetImporter::SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const
{
}
bool Volt::ParticlePresetImporter::LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const
{
	return false;
}