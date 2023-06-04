#pragma once

#include "Volt/Asset/Asset.h"
#include "Volt/Asset/AssetPacker.h"

namespace Volt
{
	class AssetImporter
	{
	public:
		virtual ~AssetImporter() = default;

		virtual bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const = 0;
		virtual void Save(const Ref<Asset>& asset) const = 0;

		virtual void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const = 0;
		virtual bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const = 0;
	};

	class TextureSourceImporter : public AssetImporter
	{
	public:
		~TextureSourceImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class ShaderImporter : public AssetImporter
	{
	public:
		~ShaderImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class RenderPipelineImporter : public AssetImporter
	{
	public:
		~RenderPipelineImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class MaterialImporter : public AssetImporter
	{
	public:
		~MaterialImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class BlendSpaceImporter : public AssetImporter
	{
	public:
		~BlendSpaceImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class FontImporter : public AssetImporter
	{
	public:
		~FontImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>&) const override {}

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class PhysicsMaterialImporter : public AssetImporter
	{
	public:
		~PhysicsMaterialImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class VideoImporter : public AssetImporter
	{
	public:
		~VideoImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class PostProcessingStackImporter : public AssetImporter
	{
	public:
		~PostProcessingStackImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};

	class PostProcessingMaterialImporter : public AssetImporter
	{
	public:
		~PostProcessingMaterialImporter() override = default;

		bool Load(const std::filesystem::path& path, Ref<Asset>& asset) const override;
		void Save(const Ref<Asset>& asset) const override;

		void SaveBinary(uint8_t* buffer, const Ref<Asset>& asset) const override;
		bool LoadBinary(const uint8_t* buffer, const AssetPacker::AssetHeader& header, Ref<Asset>& asset) const override;
	};
}
