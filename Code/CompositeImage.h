#ifndef __COMPOSITE_IMAGE_H__
#define __COMPOSITE_IMAGE_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct CompositeImage
{
	sf::Vertex *quads;
	Tileset **tilesets;
	sf::Vector2f gridReps;
	sf::Vector2f tileSize;
	int numImages;
	CompositeImage( TilesetManager *tm, const std::string &baseName, int numImages, sf::Vector2f tileSize, sf::Vector2f gridReps );
	~CompositeImage();
	void SetTopLeft(sf::Vector2f p_topLeft);
	void SetCenter(sf::Vector2f p_center);
	void Draw(sf::RenderTarget *target);
};

#endif