#ifndef __EDITORMODES_H__
#define __EDITORMODES_H__

#include <SFML/Graphics.hpp>

struct EditorMode
{
	virtual void HandleEvent(sf::Event e) = 0;
};

struct CreateTerrainMode : EditorMode
{
	virtual void HandleEvent(sf::Event e);
};

#endif