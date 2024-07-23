#pragma once

#include <Volt/Core/Base.h>

#include <vector>

namespace AssetBrowser
{
	class Item;
	class SelectionManager
	{
	public:
		void Select(const Item* item);
		void Deselect(const Item* item);
		void DeselectAll();

		const bool IsSelected(const Item* item) const;

		inline const bool IsAnySelected() const { return !mySelectedItems.empty(); }
		inline const size_t GetSelectedCount() const { return mySelectedItems.size(); }
		inline const Vector<const Item*>& GetSelectedItems() const { return mySelectedItems; }

	private:
		Vector<const Item*> mySelectedItems;
	};
}
