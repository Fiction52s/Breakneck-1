#ifndef __MOMENTUMBAR_H__
#define __MOMENTUMBAR_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"
struct GameSession;

struct MomentumBar
{
	MomentumBar(GameSession *owner);
	sf::Sprite teal;
	sf::Sprite blue;
	sf::Sprite purp;
	sf::Sprite container;
	sf::Sprite levelNumSpr;
	sf::Vector2f GetTopLeft();
	void SetTopLeft(sf::Vector2f &pos);
	int level;
	float part;
	void SetMomentumInfo(int level, float part);
	Tileset *ts_bar;
	Tileset *ts_container;
	Tileset *ts_num;
	void Draw(sf::RenderTarget *target);
	sf::Shader partShader;
};

#endif