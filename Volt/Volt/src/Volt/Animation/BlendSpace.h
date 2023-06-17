#pragma once

#include "Volt/Asset/Asset.h"

#include <GEM/gem.h>

namespace Volt
{
	enum class BlendSpaceDimension
	{
		OneD = 0,
		TwoD
	};

	class BlendSpace : public Asset
	{
	public:
		BlendSpace() = default;
		~BlendSpace() override = default;

		inline void AddAnimation(Volt::AssetHandle animation, const gem::vec2& position) { myAnimations.emplace_back(position, animation); }
		
		inline const BlendSpaceDimension GetDimension() const { return myDimension; }
		inline const gem::vec2& GetHorizontalValues() const { return myHorizontalValues; }
		inline const gem::vec2& GetVerticalValues() const { return myVerticalValues; }
		inline const std::vector<std::pair<gem::vec2, AssetHandle>>& GetAnimations() const { return myAnimations; }

		inline void SetDimension(BlendSpaceDimension dim) { myDimension = dim; }

		static AssetType GetStaticType() { return AssetType::BlendSpace; }
		virtual AssetType GetType() override { return AssetType::BlendSpace; }

	private:
		friend class BlendSpaceImporter;

		BlendSpaceDimension myDimension = BlendSpaceDimension::OneD;

		gem::vec2 myHorizontalValues = { -1.f, 1.f };
		gem::vec2 myVerticalValues = { -1.f, 1.f };

		std::vector<std::pair<gem::vec2, AssetHandle>> myAnimations;
	};
}
