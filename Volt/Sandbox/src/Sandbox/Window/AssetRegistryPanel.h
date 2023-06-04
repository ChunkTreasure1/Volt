#pragma once

#include "Sandbox/Window/EditorWindow.h"

class AssetRegistryPanel : public EditorWindow
{
public:
	AssetRegistryPanel();
	void UpdateMainContent() override;

private:
	void AddNewModal();

	std::string mySearchQuery;

	bool myActivateSearch = false;
};
