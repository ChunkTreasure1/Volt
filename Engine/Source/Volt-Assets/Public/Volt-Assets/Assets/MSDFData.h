#pragma once

// #TODO_Ivar: This really shouldn't be exposed...

#include <msdf-atlas-gen.h>

namespace Volt
{
	struct MSDFData
	{
		msdf_atlas::FontGeometry fontGeometry;
		std::vector<msdf_atlas::GlyphGeometry> glyphs;
	};
}
