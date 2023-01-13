#ifndef __WORLDMAPSHIP_H__
#define __WORLDMAPSHIP_H__

#include "Input.h"
#include <SFML/Graphics.hpp>

struct WorldMap;
struct Tileset;

struct WorldMapShip
{
	sf::Vertex shipQuad[4];
	Tileset *ts;

	WorldMapShip( WorldMap *wm );
	void Update(ControllerDualStateQueue *controllerInput);
	void Draw(sf::RenderTarget *target);
	void SetPosition(sf::Vector2f &pos);
	sf::Vector2f GetPosition();
	void SetAlpha(float alpha);
private:
	sf::Vector2f position;
	void UpdateQuads();
};

#endif