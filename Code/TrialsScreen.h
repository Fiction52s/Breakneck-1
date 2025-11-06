#ifndef __TRIALS_SCREEN_H__
#define __TRIALS_SCREEN_H__

#include <SFML/Graphics.hpp>
#include "Tileset.h"

struct TrialsManager;
struct WorldMap;

struct TrialsScreen : TilesetManager
{
	enum Action
	{
		A_WORLD_MAP,
		A_LEVEL_SELECT,
		A_RUN_LEVEL,
		A_DONE,
	};

	int action;
	int frame;

	TrialsManager *trialsMan;
	sf::Vertex quad[4];
	sf::Text closedBetaText;
	Tileset *ts_closedBeta;
	sf::Sprite closedBetaSpr;

	sf::Text splashText;
	sf::Sprite splashSpr;
	Tileset *ts_splash;

	WorldMap *worldMap;

	int currMapIndex; //the issue is that we need to integrate the world map.
	//when you select trials, you should go to the world map screen. 
	//world map can be its own thing within the trialsmanager

	TrialsScreen();
	~TrialsScreen();
	void Update();
	bool IsRunningMap();
	void Reset();
	void Draw(sf::RenderTarget *target);
};

#endif