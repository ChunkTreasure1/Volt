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
		const Vector<Volt::Animation::TRS> TrySampleAnimation(Volt::AssetHandle animationHandle);
		const Vector<std::pair<float, Volt::AssetHandle>> GetSortedAnimationWeights(Ref<Volt::BlendSpace> blendSpace, const glm::vec2& blendValue);
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

		void Serialize(YAMLStreamWriter& out) override;
		void Deserialize(YAMLStreamReader& node) override;

		inline Vector<BlendFilter>& GetBlendIncludeFilters() { return myIncludeFilters; }
		inline Vector<BlendFilter>& GetBlendExcludeFilters() { return myExcludeFilters; }

		Ref<Node> CreateCopy(Graph* ownerGraph, Volt::EntityID entity = Volt::EntityID(0)) override;

	private:
		void TryApplyLayeredBlendPerBone();

		Vector<BlendFilter> myIncludeFilters;
		Vector<BlendFilter> myExcludeFilters;
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
