#pragma once

#include "Volt/Asset/Asset.h"

#include <glm/glm.hpp>

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

		inline void AddAnimation(Volt::AssetHandle animation, const glm::vec2& position) { myAnimations.emplace_back(position, animation); }
		
		inline const BlendSpaceDimension GetDimension() const { return myDimension; }
		inline const glm::vec2& GetHorizontalValues() const { return myHorizontalValues; }
		inline const glm::vec2& GetVerticalValues() const { return myVerticalValues; }
		inline const std::vector<std::pair<glm::vec2, AssetHandle>>& GetAnimations() const { return myAnimations; }

		inline void SetDimension(BlendSpaceDimension dim) { myDimension = dim; }

		static AssetType GetStaticType() { return AssetType::BlendSpace; }
		virtual AssetType GetType() override { return AssetType::BlendSpace; }

	private:
		friend class BlendSpaceImporter;

		BlendSpaceDimension myDimension = BlendSpaceDimension::OneD;

		glm::vec2 myHorizontalValues = { -1.f, 1.f };
		glm::vec2 myVerticalValues = { -1.f, 1.f };

		std::vector<std::pair<glm::vec2, AssetHandle>> myAnimations;
	};
}
