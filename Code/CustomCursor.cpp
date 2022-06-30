#include "CustomCursor.h"

using namespace std;
using namespace sf;

CustomCursor::CustomCursor()
{
	window = NULL;
	normalImage.loadFromFile("Resources/Menu/arrow_editor_1_36x36.png");
	clickedImage.loadFromFile("Resources/Menu/arrow_editor_2_36x36.png");
	clicked = false;

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
		cursor.loadFromPixels(normalImage.getPixelsPtr(), sf::Vector2u(36, 36), Vector2u(2, 2));
		window->setMouseCursor(cursor);

		clicked = false;
	}
	
}

void CustomCursor::SetClicked()
{
	if (window != NULL && !clicked)
	{
		cursor.loadFromPixels(clickedImage.getPixelsPtr(), sf::Vector2u(36, 36), Vector2u(2, 2));
		window->setMouseCursor(cursor);

		clicked = true;
	}
}

