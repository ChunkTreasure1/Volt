#pragma once

#include "GraphKey/Node.h"

#include <Volt/Asset/Asset.h>
#include <Volt/Asset/Animation/Animation.h>

namespace Volt
{
	class BlendSpace;
}

namespace GraphKey
{
	struct CrossfadeNode : public Node
	{
		CrossfadeNode();
		~CrossfadeNode() override = default;

		inline const std::string GetName() override { return "Crossfade"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void TryApplyCrossfade();
	};

	struct AdditiveNode : public Node
	{
		AdditiveNode();
		~AdditiveNode() override = default;

		inline const std::string GetName() override { return "Additive"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void TryApplyAdditive();
	};

	struct BlendSpaceNode : public Node
	{
		BlendSpaceNode();
		~BlendSpaceNode() override = default;
	
		inline const std::string GetName() override { return "Blend Space"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void Sample();

		const std::array<std::pair<float, Volt::AssetHandle>, 2> FindClosestKeys1D(Ref<Volt::BlendSpace> blendSpace, float blendValue);
		const std::vector<Volt::Animation::TRS> TrySampleAnimation(Volt::AssetHandle animationHandle);
		const std::vector<std::pair<float, Volt::AssetHandle>> GetSortedAnimationWeights(Ref<Volt::BlendSpace> blendSpace, const glm::vec2& blendValue);
		const size_t GetSkeletonJointCount() const;
	};

	struct LayeredBlendPerBoneNode : public Node
	{
		struct BlendFilter
		{
			std::string boneName;
		};

		LayeredBlendPerBoneNode();
		~LayeredBlendPerBoneNode() override = default;

		inline const std::string GetName() override { return "Layered blend per bone"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

		void Serialize(YAML::Emitter& out) override;
		void Deserialize(const YAML::Node& node) override;

		inline std::vector<BlendFilter>& GetBlendIncludeFilters() { return myIncludeFilters; }
		inline std::vector<BlendFilter>& GetBlendExcludeFilters() { return myExcludeFilters; }

		Ref<Node> CreateCopy(Graph* ownerGraph, Wire::EntityId entity = 0) override;

	private:
		void TryApplyLayeredBlendPerBone();

		std::vector<BlendFilter> myIncludeFilters;
		std::vector<BlendFilter> myExcludeFilters;
	};

	struct RotateBoneNode : public Node
	{
		RotateBoneNode();
		~RotateBoneNode() override = default;

		inline const std::string GetName() override { return "Rotate bone"; }
		inline const glm::vec4 GetColor() override { return { 1.f }; }

	private:
		void RotateBone();
	};
}
