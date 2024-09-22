#pragma once

#include "AssetSystem/Config.h"

#include <LogModule/LogCommon.h>
#include <CoreUtilities/Containers/Vector.h>

#include <functional>
#include <filesystem>

namespace Volt
{
	class Asset;

	struct SourceAssetUserImportData
	{
		std::function<void(float)> progressCallback;
		std::function<void(const std::string&, LogVerbosity)> messageCallback;
		std::function<std::string()> passwordRequiredCallback;

		VT_INLINE void OnInfo(const std::string& string) const
		{
			if (messageCallback)
			{
				messageCallback(string, LogVerbosity::Info);
			}
		}

		VT_INLINE void OnWarning(const std::string& string) const
		{
			if (messageCallback)
			{
				messageCallback(string, LogVerbosity::Warning);
			}
		}

		VT_INLINE void OnError(const std::string& string) const
		{
			if (messageCallback)
			{
				messageCallback(string, LogVerbosity::Error);
			}
		}

		VT_INLINE std::string OnPasswordRrquired() const
		{
			if (passwordRequiredCallback)
			{
				return passwordRequiredCallback();
			}

			return "";
		}
	};

	struct SourceAssetFileInfo
	{
		std::string fileVersion;
		std::string fileCreator;
		std::string fileCreatorApplication;
		std::string fileUnits;
		std::string fileAxisDirection;

		bool hasSkeleton;
		bool hasMes;
		bool hasAnimation;
	};

	class VTAS_API SourceAssetImporter
	{
	public:
		template<typename ConfigType>
		Vector<Ref<Asset>> Import(const std::filesystem::path& filepath, const ConfigType& config, const SourceAssetUserImportData& userData = {})
		{
			return ImportInternal(filepath, reinterpret_cast<const void*>(&config), userData);
		}

	protected:
		virtual Vector<Ref<Asset>> ImportInternal(const std::filesystem::path& filepath, const void* config, const SourceAssetUserImportData& userData) = 0;
	};
}
