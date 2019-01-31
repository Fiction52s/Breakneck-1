#ifndef __KINBOOSTSCREEN_H__
#define __KINBOOSTSCREEN_H__

#include <SFML\Graphics.hpp>

struct KinBoostScreen
{
	KinBoostScreen();
	void Draw(sf::RenderTarget *target);
	sf::RectangleShape rect;
};

#endif