#ifndef __SKINMENU_H__

struct Tileset;
struct SaveMenuScreen;

#include "SFML/Graphics.hpp"
#include "Input.h"

struct SkinMenu
{
	Tileset *ts_skins;


	sf::Vertex skinQuads[64 * 4];
	sf::Vertex skinBGQuads[64 * 4];

	sf::Vector2i selectedPos;
	sf::Vector2f gridSpacing;
	sf::Vector2f gridTopLeft;

	sf::RectangleShape rect;

	SkinMenu(SaveMenuScreen *saveScreen);
	void Reset();
	void SetGridTopLeft(sf::Vector2f &pos);
	bool Update(ControllerState &currInput,
		ControllerState &prevInput);
	void Draw(sf::RenderTarget *target);


};

#define __SKINMENU_H__

#endif