#ifndef __KIN_EXPERIENCE_BAR_H__
#define __KIN_EXPERIENCE_BAR_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct ExperienceAdder
{
	int numExp;
	std::string reasonStr;
	sf::Text fullText;
	sf::Vector2f myPos;

	ExperienceAdder();
	void SetTopLeft(sf::Vector2f pos);
	void Draw(sf::RenderTarget *target);
};

struct KinExperienceBar
{
	Tileset *ts_bar;
	sf::Vertex barQuad[4];

	KinExperienceBar( TilesetManager *tm );
	void Draw(sf::RenderTarget *target);
};

#endif