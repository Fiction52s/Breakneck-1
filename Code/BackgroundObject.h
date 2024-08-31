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
	int repetitionFactor; //how often it repeats
	int scrollOffset;

	BackgroundObject(TilesetManager *p_tm);
	~BackgroundObject();
	virtual void Reset();
	virtual void Load(std::ifstream & is);
	virtual void Update(const sf::Vector2f &camPos);
	void Update(const sf::Vector2f &camPos, const int numFrames);
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
	sf::Vertex *quads;
	sf::Vector2f myPos;

	BackgroundWaterfall(TilesetManager *p_tm);
	~BackgroundWaterfall();
	void Reset();
	void Update(const sf::Vector2f &camPos);
	void Load(std::ifstream &is);
	void Draw(sf::RenderTarget *target);
	void LayeredDraw(int p_depthLevel, sf::RenderTarget *target);

};


#endif