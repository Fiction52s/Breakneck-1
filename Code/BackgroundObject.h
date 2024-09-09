#ifndef __BACKGROUND_OBJECT_H__
#define __BACKGROUND_OBJECT_H__


#include <SFML\Graphics.hpp>
#include <fstream>
#include "nlohmann\json.hpp"

struct Tileset;
struct TilesetManager;

struct BackgroundObject
{
	enum Type
	{
		BGO_WATERFALL,
	};

	Tileset *ts;
	sf::Vector2f myPos;
	sf::Vertex *quads;

	int loopWidth;
	int numQuads;
	int action;
	int frame;
	TilesetManager *tm;
	int depthLayer;
	float scrollSpeedX;
	sf::View oldView;
	sf::View newView;
	int repetitionFactor; //how often it repeats
	int scrollOffset;

	BackgroundObject(TilesetManager *p_tm, int loopWidth, int p_layer );
	~BackgroundObject();
	virtual void Reset();
	virtual void Load(std::ifstream & is);
	virtual void DrawObject(sf::RenderTarget *target);
	virtual sf::IntRect GetSubRect();
	virtual void ProcessAction();

	virtual void UpdateQuads( float realX );
	void Update(const sf::Vector2f &camPos);
	void Update(const sf::Vector2f &camPos, const int numFrames);
	void Draw(sf::RenderTarget *target);
	void LayeredDraw(int p_depthLevel, sf::RenderTarget *target);
};

struct BackgroundTile : BackgroundObject
{
	sf::IntRect subRect;
	std::string folder;
	

	BackgroundTile(TilesetManager *p_tm, const std::string &p_folder, int p_loopWidth, int p_layer );
	void Load(nlohmann::basic_json<> &jobj);
	sf::IntRect GetSubRect();
};

struct BackgroundWideSpread : BackgroundObject
{
	sf::IntRect subRect;
	std::string folder;
	int extraWidth;

	BackgroundWideSpread(TilesetManager *p_tm, const std::string &p_folder, int p_loopWidth, int p_layer );
	void Load(nlohmann::basic_json<> &jobj);
	void UpdateQuads(float realX);
	sf::IntRect GetSubRect();
};

struct BackgroundWaterfall : BackgroundObject
{
	enum Action
	{
		A_IDLE,
		A_Count,
	};

	int actionLength[A_Count];
	int animFactor[A_Count];

	BackgroundWaterfall(TilesetManager *p_tm, int p_loopWidth, int p_layer );
	~BackgroundWaterfall();
	void Load(nlohmann::basic_json<> &jobj);

	void ProcessAction();
	sf::IntRect GetSubRect();

};


#endif