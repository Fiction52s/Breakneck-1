#ifndef __BOSSHEALTH_H__
#define __BOSSHEALTH_H__

#include <SFML/Graphics.hpp>

struct Boss;
struct BossHealth
{
	sf::Vertex healthQuad[4];
	sf::Vertex bgQuad[4];
	sf::Vertex portraitQuad[4];
	Boss *boss;

	float bgWidth;
	float bgHeight;
	float healthPadding;

	BossHealth(Boss *p_boss);
	void SetTopLeft(sf::Vector2f &pos);
	void Reset();
	void Draw(sf::RenderTarget *target);
	void Update();
};

#endif