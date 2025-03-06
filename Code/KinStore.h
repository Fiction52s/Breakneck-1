#ifndef __KINSTORE_H__
#define __KINSTORE_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "nlohmann\json.hpp"

struct SingleAxisSelector;
struct Session;

struct StoreItem
{
	int upgradeIndex;
	std::string name;
	int numLevels;
	std::vector<std::string> descriptions;
	std::vector<int> costs;
	int currentLevel;
	int quadIndex;

	StoreItem( nlohmann::basic_json<> &j );
	void Print();
	const std::string &GetCurrentDescription();
	int GetCurrentCost();
};

struct KinStore
{
	enum Action
	{
		A_OPEN,
		A_READY_TO_CLOSE,
		A_Count
	};

	enum StoreSection
	{
		SS_BASICS,
		SS_POWERS,
		SS_ARMORS,
		SS_ITEMS,
		SS_Count
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

	sf::Vertex selectedBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex *itemSelectQuads;

	sf::Text upgradeNameText;
	sf::Text upgradeDescText;

	int numTotalStoreItems;

	std::vector<std::vector<StoreItem*>> storeItems;

	//sf::Vertex shardBGQuad[4];
	//sf::Vertex descriptionBGQuad[4];
	//sf::Vertex shardTitleBGQuad[4];

	KinStore();
	~KinStore();
	bool IsReadyToClose();
	void SetSelected( int section, int itemIndex );
	void SetTopLeft(sf::Vector2f pos);
	void Update();
	void Open();
	void LoadStore();
	void Draw(sf::RenderTarget *target);
};

#endif

