#ifndef __WORLD_MAP_H__
#define __WORLD_MAP_H__

#include <SFML/Graphics.hpp>
#include <boost/filesystem.hpp>
#include "LevelSelector.h"
#include "Input.h"
#include "Tileset.h"

struct MainMenu;
struct SaveFile;

struct MapNode
{
	MapNode();
	int GetNextIndex(ControllerState &curr, 
		ControllerState &prev);
	void Draw(sf::RenderTarget *target);
	void StartMap();

	std::string mapName;
	sf::Vector2i pos;
	int leftLinkIndex;
	int rightLinkIndex;
	int upLinkIndex;
	int downLinkIndex;
	int index;
};

struct ColonyMap
{
	int numNodes;
	MapNode **nodes;
	int currNodeIndex;
	void Load(std::ifstream &is);
	void Update(ControllerState &curr, ControllerState &prev );
	void Draw(sf::RenderTarget *target);
};

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

	ColonyMap testMap;
	sf::Shader zoomShader;
	WorldMap( MainMenu *mainMenu );
	void Reset( SaveFile *sf );
	~WorldMap();
	bool Update(
		ControllerState &prevInput,
		ControllerState &currInput );
	void Draw( sf::RenderTarget *target );
	void UpdateMapList();
	const std::string & GetSelected();
	void UpdateMapList( TreeNode *parentNode, const std::string &relativePath );
	void ClearEntries();
	void UpdateColonySelect();
	int Tex( int index, int level, TreeNode *entry );
	State state;
	int frame;
	sf::Sprite extraPassSpr;
	sf::Sprite zoomedMapSpr;
	Tileset *ts_zoomedMapw1;
	float currScale;
	sf::Vector2f currCenter;
	float oldZoomCurvePos;
	float zoomCurvePos;
	sf::View zoomView;

	Tileset *ts_planetAndSpace;
	Tileset *ts_planet;
	Tileset *ts_colonySelect;
	sf::Sprite colonySelectSprite;
	//sf::Texture *planetAndSpaceTex;
	//sf::Texture *planetTex;
	int selectedColony;
	
	//sf::Texture *sectionTex[6];
	//sf::Texture *colonyTex[6];
	Tileset *ts_section[6];
	//Tileset *ts_colony[6];

	Tileset *ts_colony[7];
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

	bool moveDown;
	bool moveUp;

	//ControllerState currInput;
	//ControllerState prevInput;
};

#endif