#include "vtpch.h"
#include "AssetFactory.h"

#include "Volt/Asset/Mesh/Mesh.h"
#include "Volt/Asset/Mesh/Material.h"
#include "Volt/Asset/Animation/Animation.h"
#include "Volt/Asset/Animation/Skeleton.h"
#include "Volt/Asset/Animation/AnimatedCharacter.h"
#include "Volt/Asset/Animation/AnimationGraphAsset.h"
#include "Volt/Asset/ParticlePreset.h"
#include "Volt/Asset/Text/Font.h"
#include "Volt/Asset/Prefab.h"
#include "Volt/Asset/Video/Video.h"
#include "Volt/Asset/TimelinePreset.h"
#include "Volt/Asset/Rendering/PostProcessingMaterial.h"
#include "Volt/Asset/Rendering/PostProcessingStack.h"

#include "Volt/Scene/Scene.h"
#include "Volt/Animation/BlendSpace.h"

#include "Volt/Physics/PhysicsMaterial.h"

#include "Volt/Rendering/Shader/Shader.h"
#include "Volt/Rendering/Texture/Texture2D.h"

#include "Volt/BehaviorTree/BehaviorTree.hpp"
#include "Volt/Rendering/RenderPipeline/RenderPipeline.h"
#include "Volt/Net/SceneInteraction/NetContract.h"

#include <Navigation/NavMesh/VTNavMesh.h>

namespace Volt
{
	template<typename T>
	void RegisterCreateFunction(AssetType type, std::unordered_map<AssetType, AssetFactory::AssetCreateFunction>& out)
	{
		out[type] = []() { return CreateRef<T>(); }
	}

	void AssetFactory::Initialize()
	{
		RegisterCreateFunction<Mesh>(AssetType::Mesh, m_assetFactoryFunctions);
		RegisterCreateFunction<Mesh>(AssetType::MeshSource, m_assetFactoryFunctions);
		RegisterCreateFunction<Animation>(AssetType::Animation, m_assetFactoryFunctions);
		RegisterCreateFunction<Skeleton>(AssetType::Skeleton, m_assetFactoryFunctions);
		RegisterCreateFunction<Texture2D>(AssetType::Texture, m_assetFactoryFunctions);
		RegisterCreateFunction<Material>(AssetType::Material, m_assetFactoryFunctions);
		RegisterCreateFunction<Shader>(AssetType::Shader, m_assetFactoryFunctions);
		RegisterCreateFunction<Shader>(AssetType::ShaderSource, m_assetFactoryFunctions);
		RegisterCreateFunction<Scene>(AssetType::Scene, m_assetFactoryFunctions);
		RegisterCreateFunction<AnimatedCharacter>(AssetType::AnimatedCharacter, m_assetFactoryFunctions);
		RegisterCreateFunction<ParticlePreset>(AssetType::ParticlePreset, m_assetFactoryFunctions);
		RegisterCreateFunction<Prefab>(AssetType::Prefab, m_assetFactoryFunctions);
		RegisterCreateFunction<Font>(AssetType::Font, m_assetFactoryFunctions);
		RegisterCreateFunction<PhysicsMaterial>(AssetType::PhysicsMaterial, m_assetFactoryFunctions);
		RegisterCreateFunction<Video>(AssetType::Video, m_assetFactoryFunctions);
		RegisterCreateFunction<BlendSpace>(AssetType::BlendSpace, m_assetFactoryFunctions);
		RegisterCreateFunction<AI::NavMesh>(AssetType::NavMesh, m_assetFactoryFunctions);
		RegisterCreateFunction<AnimationGraphAsset>(AssetType::AnimationGraph, m_assetFactoryFunctions);
		RegisterCreateFunction<GraphKey::Graph>(AssetType::GraphKey, m_assetFactoryFunctions);
		RegisterCreateFunction<BehaviorTree::Tree>(AssetType::BehaviorGraph, m_assetFactoryFunctions);
		RegisterCreateFunction<BehaviorTree::Tree>(AssetType::GraphKey, m_assetFactoryFunctions);
		RegisterCreateFunction<RenderPipeline>(AssetType::RenderPipeline, m_assetFactoryFunctions);
		RegisterCreateFunction<TimelinePreset>(AssetType::Timeline, m_assetFactoryFunctions);
		RegisterCreateFunction<NetContract>(AssetType::NetContract, m_assetFactoryFunctions);
		RegisterCreateFunction<PostProcessingMaterial>(AssetType::PostProcessingMaterial, m_assetFactoryFunctions);
		RegisterCreateFunction<PostProcessingStack>(AssetType::PostProcessingStack, m_assetFactoryFunctions);
	}
	
	void AssetFactory::Shutdown()
	{
	}

	Ref<Asset> AssetFactory::CreateAssetOfType(AssetType type)
	{
		return Ref<Asset>();
	}
}
