#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/AssetPacker.h"

namespace Volt
{
	class AssetImporter
	{
	public:
		virtual ~AssetImporter() = default;

		virtual bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const = 0;
		virtual void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const = 0;
	};

	class TextureSourceImporter : public AssetImporter
	{
	public:
		~TextureSourceImporter() override = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};

	class ShaderDefinitionImporter : public AssetImporter
	{
	public:
		~ShaderDefinitionImporter() override = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};

	//class MaterialImporter : public AssetImporter
	//{
	//public:
	//	~MaterialImporter() override = default;

	//	bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
	//	void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	//};

	class BlendSpaceImporter : public AssetImporter
	{
	public:
		~BlendSpaceImporter() override = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};

	class FontImporter : public AssetImporter
	{
	public:
		~FontImporter() override = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata&, const Ref<Asset>&) const override {}
	};

	class PhysicsMaterialImporter : public AssetImporter
	{
	public:
		~PhysicsMaterialImporter() override = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};

	class VideoImporter : public AssetImporter
	{
	public:
		~VideoImporter() override = default;

		bool Load(const AssetMetadata& metadata, Ref<Asset>& asset) const override;
		void Save(const AssetMetadata& metadata, const Ref<Asset>& asset) const override;
	};
}
