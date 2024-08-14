#pragma once

#include "Volt/Asset/Asset.h"

#include <CoreUtilities/Containers/Graph.h>

#include <shared_mutex>

namespace Volt
{
	struct AssetDependencyInfo
	{
		AssetHandle handle;
	};

	struct EdgeInfo
	{
	};

	class AssetDependencyGraph
	{
	public:
		using WriteLock = std::unique_lock<std::shared_mutex>;
		using ReadLock = std::shared_lock<std::shared_mutex>;

		AssetDependencyGraph();
		~AssetDependencyGraph();

		// #TODO_Ivar: Implement removing nodes

		UUID64 AddAssetToGraph(AssetHandle handle);
		void RemoveAssetFromGraph(AssetHandle handle);

		void AddDependencyToAsset(AssetHandle handle, AssetHandle dependency);
		void RemoveDependencyToAsset(AssetHandle handle, AssetHandle dependency);

		void OnAssetChanged(AssetHandle handle, AssetChangedState state);

		const Vector<AssetHandle> GetAssetDependencyChain(AssetHandle handle) const;
		const Vector<AssetHandle> GetAssetsDependentOn(AssetHandle handle) const;

		inline const bool DoAssetExistInGraph(AssetHandle handle) const { return m_assetNodeIds.contains(handle); }

	private:
		mutable std::shared_mutex m_mutex;
		
		std::unordered_map<AssetHandle, UUID64> m_assetNodeIds;
		Graph<AssetDependencyInfo, EdgeInfo> m_graph;
	};
}
