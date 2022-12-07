#pragma once
#include <Volt/Utility/Math.h>
#include "UIMath.h"
#include "UICanvas.h"
#include <vector>

enum class eUIElementType
{
	UNASSIGNED,
	SPRITE,
	BUTTON,
	TEXT,
	POPUP,
	SLIDER,
	CHECKBOX,
	COUNT
};

class UIElement
{
public:
	UIElement()
	{

	}

	gem::vec2 GetPosition()	{ return gem::vec2{ myTransform[3][0], myTransform[3][1] };}
	void SetPosition(gem::vec2 aPosition)
	{
		//Covert position to normalized value
		gem::vec2 normalizedPosition = UIMath::GetNormalizedPosition(aPosition);

		//Convert normalized position to Viewport position
		gem::vec2 VP_position = UIMath::GetViewportPosition(normalizedPosition, *myCanvas);

		myTransform[3][0] = VP_position.x;
		myTransform[3][1] = VP_position.y;

		for (auto child : myChildren)
		{
			child->UpdatePosition(*this);
		}
	}

	void UpdatePosition(UIElement& aParentElement)
	{
		switch (myUIType)
		{
		case eUIElementType::SPRITE:
			UpdateSpritePosition(aParentElement);
			break;
		default:
			SetPosition(GetPosition() + aParentElement.GetPosition());
			break;
		}

		for (auto child : myChildren)
		{
			child->UpdatePosition(*this);
		}
	}

	void UpdateSpritePosition(UIElement& aParentElement)
	{
		gem::vec2 newSpritePos = { aParentElement.GetPosition().x + GetPosition().x, aParentElement.GetPosition().y + GetPosition().y };
		gem::mat4 newSpriteTransform = { 1.f };
		newSpriteTransform[3][0] = newSpritePos.x;
		newSpriteTransform[3][1] = newSpritePos.y;

		//newSpriteTransform *= gem::scale(gem::mat4(1.0f), gem::vec3{ GetSize().x * GetScale().x * GetCanvas()->globalScaleX, GetSize().y * GetScale().y * GetCanvas()->globalScaleY, 1 });

		SetTransform(newSpriteTransform);

		//gem::vec2 myAnchoredPos = UIMath::GetSpriteAnchoredPos(*GetCanvas(), aParentElement.GetAnchor());
		//SetPosition(myAnchoredPos + aParentElement.GetPosition());
	}

	gem::mat4 GetTransform() { return myTransform * gem::scale(gem::mat4(1.0f), gem::vec3{mySize.x * myScale.x * myCanvas->globalScaleX, mySize.y * myScale.y * myCanvas->globalScaleY, 1}); }
	void SetTransform(gem::mat4 aTransform) { myTransform = aTransform; }

	gem::vec2 GetSize() { return mySize; }
	void SetSize(gem::vec2 aSize) { mySize = aSize; }

	gem::vec2 GetScale() { return myScale; }
	void SetScale(gem::vec2 aScale) { myScale = aScale; }

	void SetLayer(int aLayer) { myTransform[3][2] = (float)aLayer; }
	int GetLayer() { return (int)myTransform[3][2]; }

	void SetAnchor(eAnchored aAnchor) { myAchor = aAnchor; }
	eAnchored GetAnchor() { return myAchor; }

	gem::vec2 GetPivot() { return myPivot; }
	void SetPivot(gem::vec2 aPivot) { myPivot = aPivot; }

	void SetCanvas(std::shared_ptr<UICanvas> aCanvas) { myCanvas = aCanvas; SetChildCanvas(myCanvas); }
	std::shared_ptr<UICanvas> GetCanvas() { return myCanvas; }
	void SetChildCanvas(std::shared_ptr<UICanvas> aCanvas)
	{
		for (auto child : myChildren)
		{
			child->SetCanvas(aCanvas);
			child->SetChildCanvas(aCanvas);
		}
	}

	void ReciveChild(UIElement& aUIElement) { myChildren.push_back(&aUIElement); }

public:
	bool isEnabled = true;
	eUIElementType myUIType = eUIElementType::UNASSIGNED;
	std::shared_ptr<UICanvas> myCanvas;

private:
	gem::mat4 myTransform = { 1.0f };
	gem::vec2 mySize = { 1.0f, 1.0f };
	gem::vec2 myScale = { 1.0f, 1.0f };;
	gem::vec2 myPivot = { 0.0f, 0.0f };;
	eAnchored myAchor = eAnchored::C;

	std::vector<UIElement*> myChildren;

};