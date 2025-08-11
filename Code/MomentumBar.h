#ifndef __MOMENTUMBAR_H__
#define __MOMENTUMBAR_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
struct GameSession;
struct Actor;

struct MomentumBar
{
	sf::Shader partShader;
	Tileset *ts_bar;
	sf::Vertex barQuad[4];
	int level;
	float part;
	sf::Vector2f center;
	Actor *actor;

	Tileset *ts_gems;
	sf::Vertex gemQuads[4 * 6];

	MomentumBar(TilesetManager *tm);
	sf::Vector2f GetCenter();
	void SetCenter(sf::Vector2f &pos);
	
	void UpdateMomentumInfo();
	void UpdateGems();
	void Draw(sf::RenderTarget *target);
	
};

#endif