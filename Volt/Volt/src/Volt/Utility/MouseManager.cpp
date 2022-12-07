#include "vtpch.h"
#include "MouseManager.h"

MouseManager::MouseManager()
{
}

MouseManager::~MouseManager()
{
}

void MouseManager::ShowCursor(bool aState)
{
	if (aState == true)
	{
		
	}
}

void MouseManager::ChangeMousePointer(MousePointerType aType)
{
	switch (aType)
	{
		case MousePointerType::Arrow:
			break;
		
		case MousePointerType::Selectable:
			break;

		case MousePointerType::Target:
			break;

		case MousePointerType::Loading:
			break;
	}
}
