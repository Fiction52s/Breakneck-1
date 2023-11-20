#ifndef __CUSTOM_CURSOR_H__
#define __CUSTOM_CURSOR_H__

#include <SFML\Graphics.hpp>

struct CustomCursor
{
	enum Mode
	{
		M_REGULAR,
		//M_SHIP,
		M_Count
	};

	CustomCursor();
	~CustomCursor();

	sf::Image normalImage[M_Count];
	sf::Image clickedImage[M_Count];
	sf::Vector2u hotspot[M_Count];
	sf::Cursor cursor;
	sf::RenderWindow *window;
	bool clicked;
	int mode;
	bool visible;
	bool grabbed;
	

	void SetClicked();
	void SetNormal();
	void SetMode(int m);
	void Hide();
	void Show();
	void Grab();
	void Release();
	void Init(sf::RenderWindow *rw);
};

#endif