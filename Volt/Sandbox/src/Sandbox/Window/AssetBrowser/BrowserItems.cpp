#include "sbpch.h"
#include "BrowserItems.h"

namespace AssetBrowser
{
	Item::Item(SelectionManager* selectionManager, const std::filesystem::path& aPath)
		: mySelectionManager(selectionManager), path(aPath)
	{
	}
}