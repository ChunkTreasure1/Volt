#include "vtpch.h"
#include "AssetDependencyGraph.h"

#include "Volt/Asset/AssetManager.h"

namespace Volt
{
	AssetDependencyGraph::AssetDependencyGraph()
	{
	}
	AssetDependencyGraph::~AssetDependencyGraph()
	{
	}

	UUID64 AssetDependencyGraph::AddAssetToGraph(AssetHandle handle)
	{
		WriteLock lock{ m_mutex };

		if (m_assetNodeIds.contains(handle))
		{
			return m_assetNodeIds.at(handle);
		}

		UUID64 newId = m_graph.AddNode({ handle });
		m_assetNodeIds[handle] = newId;

		return newId;
	}

	void AssetDependencyGraph::RemoveAssetFromGraph(AssetHandle handle)
	{
		WriteLock lock{ m_mutex };
		if (!m_assetNodeIds.contains(handle))
		{
			return;
		}

		m_graph.RemoveNode(m_assetNodeIds.at(handle));
		m_assetNodeIds.erase(handle);
	}

	void AssetDependencyGraph::AddDependencyToAsset(AssetHandle handle, AssetHandle dependency)
	{
		// #TODO_Ivar: Check if asset already is a dependency, so we don't add it twice
		WriteLock lock{ m_mutex };
		if (!DoAssetExistInGraph(handle))
		{
			VT_CORE_WARN("[AssetDependencyGraph]: Trying to add dependency to asset not in the graph!");
			return;
		}

		if (!DoAssetExistInGraph(dependency))
		{
			VT_CORE_WARN("[AssetDependencyGraph]: Trying to add dependency not in the graph to asset!");
			return;
		}

		m_graph.LinkNodes(m_assetNodeIds.at(handle), m_assetNodeIds.at(dependency));
	}

	void AssetDependencyGraph::OnAssetChanged(AssetHandle handle, AssetChangedState state)
	{
		ReadLock lock{ m_mutex };
		const auto dependants = GetAssetsDependentOn(handle);

		for (size_t i = 1; i < dependants.size(); i++)
		{
			const auto isLoaded = Volt::AssetManager::IsLoaded(dependants.at(i));
			if (isLoaded)
			{
				const auto rawAsset = Volt::AssetManager::Get().GetAssetRaw(dependants.at(i));
				if (rawAsset && rawAsset->IsValid())
				{
					rawAsset->OnDependencyChanged(handle, state);
				}
			}
		}
	}

	const Vector<AssetHandle> AssetDependencyGraph::GetAssetDependencyChain(AssetHandle handle) const
	{
		if (!DoAssetExistInGraph(handle))
		{
			VT_CORE_WARN("[AssetDependencyGraph]: Trying to get chain of asset not in the graph!");
			return {};
		}

		Vector<AssetHandle> result;
		result.emplace_back(handle);

		const auto& node = m_graph.GetNodeFromID(m_assetNodeIds.at(handle));

		for (const auto& output : node.GetInputEdges())
		{
			const auto& edge = m_graph.GetEdgeFromID(output);
			auto nodeRes = GetAssetDependencyChain(m_graph.GetNodeFromID(edge.startNode).nodeData.handle);

			for (const auto& res : nodeRes)
			{
				result.emplace_back(res);
			}
		}

		return result;
	}

	const Vector<AssetHandle> AssetDependencyGraph::GetAssetsDependentOn(AssetHandle handle) const
	{
		if (!DoAssetExistInGraph(handle))
		{
			VT_CORE_WARN("[AssetDependencyGraph]: Trying to get chain of asset not in the graph!");
			return {};
		}

		Vector<AssetHandle> result;
		result.emplace_back(handle);

		const auto& node = m_graph.GetNodeFromID(m_assetNodeIds.at(handle));

		for (const auto& output : node.GetInputEdges())
		{
			const auto& edge = m_graph.GetEdgeFromID(output);
			auto nodeRes = GetAssetDependencyChain(m_graph.GetNodeFromID(edge.startNode).nodeData.handle);

			for (const auto& res : nodeRes)
			{
				result.emplace_back(res);
			}
		}

		return result;
	}
}
