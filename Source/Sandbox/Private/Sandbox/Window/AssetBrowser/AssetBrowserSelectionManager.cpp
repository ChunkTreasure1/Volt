#include "sbpch.h"
#include "Window/AssetBrowser/AssetBrowserSelectionManager.h"

namespace AssetBrowser
{
	void SelectionManager::Select(const Item* item)
	{
		if (std::find(mySelectedItems.begin(), mySelectedItems.end(), item) != mySelectedItems.end())
		{
			return;
		}

		mySelectedItems.emplace_back(item);
	}

	void SelectionManager::Deselect(const Item* item)
	{
		auto it = std::find(mySelectedItems.begin(), mySelectedItems.end(), item);

		if (it == mySelectedItems.end())
		{
			return;
		}

		mySelectedItems.erase(it);
	}

	const bool SelectionManager::IsSelected(const Item* item) const
	{
		return std::find(mySelectedItems.begin(), mySelectedItems.end(), item) != mySelectedItems.end();
	}

	void SelectionManager::DeselectAll()
	{
		mySelectedItems.clear();
	}
}

