#ifndef __TOP_CLOUDS_H__
#define __TOP_CLOUDS_H__

#include <SFML/Graphics.hpp>

struct Tileset;
struct TopCloud;
struct GameSession;
struct TopClouds
{
	TopClouds( GameSession *owner );
	void Update();
	void Draw(sf::RenderTarget *target);
	int numClouds;
	sf::Vertex *cloudVA;
	Tileset *ts;
	int animFactor;
	int animLength;
	GameSession *owner;
	int startIndex;
	int endIndex;
};


#endif