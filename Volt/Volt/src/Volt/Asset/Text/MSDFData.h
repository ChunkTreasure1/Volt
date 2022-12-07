#pragma once

#include <msdf-atlas-gen.h>

#include <vector>

namespace Volt
{
	struct MSDFData
	{
		msdf_atlas::FontGeometry fontGeometry;
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
	};
}