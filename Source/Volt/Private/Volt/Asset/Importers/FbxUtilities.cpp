#include "vtpch.h"
#include "Volt/Asset/Importers/FbxUtilities.h"

#include <ufbx.h>

namespace Volt::FbxUtilities
{
	FbxInformation GetFbxInformation(const std::filesystem::path& fbxFilePath)
	{
		ufbx_load_opts options =
		{
			.load_external_files = true,
			.ignore_missing_external_files = true,
			.generate_missing_normals = true,

			.target_axes =
			{
				.right = UFBX_COORDINATE_AXIS_POSITIVE_X,
				.up = UFBX_COORDINATE_AXIS_POSITIVE_Y,
				.front = UFBX_COORDINATE_AXIS_POSITIVE_Z,
			},
			.target_unit_meters = 0.01f,
		};

		options.space_conversion = UFBX_SPACE_CONVERSION_MODIFY_GEOMETRY;

		ufbx_error error;
		ufbx_scene* scene = ufbx_load_file(fbxFilePath.string().c_str(), &options, &error);

		if (!scene)
		{
			return {};
		}

		FbxInformation result{};
		result.fileVersion = std::to_string(scene->metadata.version);
		result.fileCreator = scene->metadata.creator.data;
		result.fileCreatorApplication = scene->metadata.original_application.name.data;
		result.fileUnits = "";

		result.hasSkeleton = scene->bones.count > 0;
		result.hasMesh = scene->meshes.count > 0;
		result.hasAnimation = scene->anim_stacks.count > 0;

		return result;
	}
}
