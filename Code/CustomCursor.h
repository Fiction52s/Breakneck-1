#ifndef __CUSTOM_CURSOR_H__
#define __CUSTOM_CURSOR_H__

#include <SFML\Graphics.hpp>

struct CustomCursor
{
	CustomCursor();

	sf::Image normalImage;
	sf::Image clickedImage;
	sf::Cursor cursor;
	sf::RenderWindow *window;
	bool clicked;

	void SetClicked();
	void SetNormal();
	void Init(sf::RenderWindow *rw);
};

#endif