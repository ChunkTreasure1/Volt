#pragma once
#include "Volt/UI/UIButton.hpp"

class UICheckbox : public UIButton
{
//FUNCTIONS
public:
	bool OnButtonPressed() override
	{
		if (myButtonFunction == "") { return false; }
		ChangeState();
		return Volt::UIFunctionRegistry::Execute(myButtonFunction);
	}

	void SetState(bool aBool) {
		isChecked = aBool; ChangeCheckedTexture();
	}

	void SetUncheckedTexture(std::string aTexturePath) { myUncheckedTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aTexturePath); }
	void SetCheckedTexture(std::string aTexturePath) { myCheckedTexture = Volt::AssetManager::GetAsset<Volt::Texture2D>(aTexturePath); }

private:
	void ChangeState() 
	{
		isChecked = !isChecked;
		ChangeCheckedTexture();
	}

	void ChangeCheckedTexture() 
	{
		if (isChecked)
		{
			GetSprite().SetTexture(myCheckedTexture);
		}
		else
		{
			GetSprite().SetTexture(myUncheckedTexture);
		}
	}

//VARIABLES
public:

private:
	bool isChecked = false;

	std::shared_ptr<Volt::Texture2D> myUncheckedTexture;
	std::shared_ptr<Volt::Texture2D> myCheckedTexture; 
};