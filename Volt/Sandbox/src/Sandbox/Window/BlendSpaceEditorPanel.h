#include "Sandbox/Window/EditorWindow.h"

namespace Volt
{
	class BlendSpace;
}

class BlendSpaceEditorPanel : public EditorWindow
{
public:
	BlendSpaceEditorPanel();
	~BlendSpaceEditorPanel() override;

	void UpdateContent() override;
	void UpdateMainContent() override {}
	void OnEvent(Volt::Event& e) override;

	void OpenAsset(Ref<Volt::Asset> asset) override;

private:
	void UpdateProperties();
	Volt::AssetHandle myAddHandle = 0;

	Ref<Volt::BlendSpace> myCurrentBlendSpace;
};
