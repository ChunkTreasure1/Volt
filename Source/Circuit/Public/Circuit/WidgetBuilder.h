#pragma once

enum class RectRoundingSides : unsigned char;

class CircuitColor;

class Widget;
class RectanglePrimitiveWidget;
class CirclePrimitiveWidget;

class WidgetBuilder
{
public: 
	WidgetBuilder();
	~WidgetBuilder();
	
private:
	std::vector<std::shared_ptr<Widget>> Widgets;
};
