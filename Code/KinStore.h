#ifndef __KINSTORE_H__
#define __KINSTORE_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"

struct SingleAxisSelector;
struct Session;

struct StoreItem
{
	int upgradeIndex;
	std::string description;
	std::string name;
};

struct KinStore
{
	enum Action
	{
		A_OPEN,
		A_READY_TO_CLOSE,
		A_Count
	};

	int action;
	int frame;
	sf::Text itemText;
	TilesetManager *tm;
	Session *sess; //doesn't always need a session

	int gridSizeX;
	int gridSizeY;

	int *currentStoreItems;

	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;

	int selectedIndex;

	sf::Vertex selectedBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex *itemSelectQuads;

	std::vector<std::pair<std::string, std::string>> upgradeDescriptionStringTable;

	sf::Text upgradeNameText;
	sf::Text upgradeDescText;

	//sf::Vertex shardBGQuad[4];
	//sf::Vertex descriptionBGQuad[4];
	//sf::Vertex shardTitleBGQuad[4];

	KinStore();
	~KinStore();
	bool IsReadyToClose();
	void SetSelectedIndex(int ind);
	void SetTopLeft(sf::Vector2f pos);
	void Update();
	void Open();
	void CreateDescriptionTable();
	void SetTableEntry(int index, const std::string & s1, const std::string &s2);
	void Draw(sf::RenderTarget *target);
};

#endif

