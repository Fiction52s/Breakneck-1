#ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include "LevelSelector.h"
#include "Input.h"

struct WorldMap
{
	enum State
	{
		OFF,
		PLANET_AND_SPACE,
		PLANET_TRANSITION,
		PLANET,
		SECTION_TRANSITION,
		SECTION,
		COLONY_TRANSITION,
		COLONY
	};

	WorldMap( sf::Font &p_font );
	void Reset();
	~WorldMap();
	void Update();
	void Draw( sf::RenderTarget *target );
	void UpdateMapList();
	void UpdateMapList( TreeNode *parentNode, const std::string &relativePath );
	void ClearEntries();
	int Tex( int index, int level, TreeNode *entry );
	State state;
	int frame;
	sf::Texture *planetAndSpaceTex;
	sf::Texture *planetTex;
	int selectedColony;
	
	sf::Texture *sectionTex[6];
	sf::Texture *colonyTex[6];
	sf::Sprite back;
	sf::Sprite front;

	TreeNode **dirNode;
	TreeNode *entries;
	int selectedLevel;
	int numTotalEntries;
	sf::Text * text;
	sf::Font font;
	std::string * localPaths;
	int fontHeight;
	int leftBorder;
	//int xspacing;
	int yspacing;
	sf::Vector2f menuPos;
	sf::RectangleShape bgRect;
	sf::RectangleShape selectedRect;

	ControllerState currInput;
	ControllerState prevInput;
};

#endif