#ifndef __TOP_CLOUDS_H__
#define __TOP_CLOUDS_H__

#include <SFML/Graphics.hpp>

struct Tileset;
struct TopCloud;
struct Session;
struct TopClouds
{
	TopClouds();
	~TopClouds();
	void SetToHeader();
	void Update();
	void Draw(sf::RenderTarget *target);
	int numClouds;
	sf::Vertex *cloudVA;
	Tileset *ts;
	int animFactor;
	int animLength;
	Session *sess;
	int startIndex;
	int endIndex;

	int currWidth;
	int currLeft;
	int currTop;
};


#endif