#ifndef __MOMENTUMBAR_H__
#define __MOMENTUMBAR_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
struct GameSession;

struct MomentumBar
{
	sf::Shader partShader;
	Tileset *ts_bar;
	sf::Vertex barQuad[4];
	int level;
	float part;
	sf::Vector2f center;

	MomentumBar(TilesetManager *tm);
	sf::Vector2f GetCenter();
	void SetCenter(sf::Vector2f &pos);
	
	void SetMomentumInfo(int level, float part);
	void Draw(sf::RenderTarget *target);
	
};

#endif