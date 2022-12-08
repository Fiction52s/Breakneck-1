#include "CustomCursor.h"

using namespace std;
using namespace sf;

CustomCursor::CustomCursor()
{
	window = NULL;

	normalImage[M_REGULAR].loadFromFile("Resources/Menu/arrow_editor_1_36x36.png");
	clickedImage[M_REGULAR].loadFromFile("Resources/Menu/arrow_editor_2_36x36.png");
	hotspot[M_REGULAR] = Vector2u(2,2);

	normalImage[M_SHIP].loadFromFile("Resources/Menu/ship_mouse_icon_1_172x80.png");
	clickedImage[M_SHIP].loadFromFile("Resources/Menu/ship_mouse_icon_2_172x80.png");
	hotspot[M_SHIP] = Vector2u(2, 2);
	clicked = false;

	mode = M_REGULAR;
}

CustomCursor::~CustomCursor()
{
}

void CustomCursor::SetMode(int m)
{
	if (m == mode)
		return;

	mode = m;

	if (clicked)
	{
		
		cursor.loadFromPixels(clickedImage[mode].getPixelsPtr(), clickedImage[mode].getSize(), hotspot[mode]);
		window->setMouseCursor(cursor);
	}
	else
	{
		cursor.loadFromPixels(normalImage[mode].getPixelsPtr(), normalImage[mode].getSize(), hotspot[mode]);
		window->setMouseCursor(cursor);
	}
}

void CustomCursor::Init(sf::RenderWindow *rw)
{
	window = rw;
	clicked = true;
	SetNormal();
}

void CustomCursor::SetNormal()
{
	if (window != NULL && clicked)
	{
		cursor.loadFromPixels(normalImage[mode].getPixelsPtr(), normalImage[mode].getSize(), hotspot[mode]);
		window->setMouseCursor(cursor);

		clicked = false;
	}
}

void CustomCursor::SetClicked()
{
	if (window != NULL && !clicked)
	{
		cursor.loadFromPixels(clickedImage[mode].getPixelsPtr(), clickedImage[mode].getSize(), hotspot[mode]);
		window->setMouseCursor(cursor);

		clicked = true;
	}
}

