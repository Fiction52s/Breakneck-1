#ifndef __RECORDED_ANIMATION_H__
#define __RECORDED_ANIMATION_H__

#include "Tileset.h"
#include <vector>

struct RecordedAnimation
{
	int frame;
	int totalFrames;
	int numTilesets;
	std::vector<Tileset*> tilesetVec;
	sf::Vector2f recordSize;
	float scale;
	sf::Vertex quad[4];
	Tileset *ts_curr;
	sf::Vector2f topLeft;
	float displayScale;

	int animFactor;


	RecordedAnimation(TilesetManager *tm, const std::string &folder, const std::string &name);
	~RecordedAnimation();
	void SetCenter(sf::Vector2f p);
	void SetTopLeft(sf::Vector2f p);
	void SetScale(float s);
	void Reset();
	void Update();
	void UpdateQuad();
	void Draw(sf::RenderTarget *target);
};

#endif