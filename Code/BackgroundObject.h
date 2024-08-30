#ifndef __BACKGROUND_OBJECT_H__
#define __BACKGROUND_OBJECT_H__


#include <SFML\Graphics.hpp>
#include <fstream>

struct Tileset;
struct TilesetManager;

struct BackgroundObject
{
	enum Type
	{
		BGO_WATERFALL,
	};

	int action;
	int frame;
	TilesetManager *tm;
	int depthLayer;
	float scrollSpeedX;
	sf::View oldView;
	sf::View newView;

	BackgroundObject(TilesetManager *p_tm);
	~BackgroundObject();
	virtual void Reset();
	virtual void Load(std::ifstream & is);
	virtual void Update();
	void Update(int numFrames);
	virtual void Draw(sf::RenderTarget *target);
	virtual void LayeredDraw(int p_depthLevel, sf::RenderTarget *target);
};

struct BackgroundWaterfall : BackgroundObject
{
	enum Action
	{
		A_IDLE,
		A_Count,
	};

	Tileset *ts;
	int actionLength[A_Count];
	int animFactor[A_Count];
	sf::Vertex quad[4];

	BackgroundWaterfall(TilesetManager *p_tm);
	void Reset();
	void Update();
	void Load(std::ifstream &is);
	void Draw(sf::RenderTarget *target);
	void LayeredDraw(int p_depthLevel, sf::RenderTarget *target);

};


#endif