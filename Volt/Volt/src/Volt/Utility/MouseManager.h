#pragma once

enum class MousePointerType
{
	Arrow,
	Selectable,
	Target,
	Loading
};

class MouseManager
{
	public:
	MouseManager();
	~MouseManager();

	void ShowCursor(bool aState);
	void ChangeMousePointer(MousePointerType aType);


	private:
};
