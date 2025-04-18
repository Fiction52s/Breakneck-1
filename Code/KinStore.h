#ifndef __KINSTORE_H__
#define __KINSTORE_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "nlohmann\json.hpp"

struct SingleAxisSelector;
struct Session;
struct RushManager;

struct StoreEntry
{
	int currentLevel;
	int upgradeIndex;
	std::string name;
	int numLevels;
	std::vector<std::string> descriptions;
	std::vector<int> costs;
	int quadIndex;

	StoreEntry( nlohmann::basic_json<> &j );
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
		SECTION_BASICS,
	//	SECTION_ITEMS,
		SECTION_POWERS,
		SECTION_Count
	};

	RushManager *rush;
	int action;
	int frame;
	sf::Text itemText;

	sf::Text storePointsText;
	TilesetManager *tm;
	Session *sess; //doesn't always need a session

	int gridSizeX;
	int gridSizeY;

	int *currentStoreItems;

	SingleAxisSelector *xSelector;
	SingleAxisSelector *ySelector;

	Tileset *ts_bg;
	Tileset *ts_yellowSquare;

	sf::Sprite bgSpr;
	sf::Sprite yellowSpr;

	sf::Vertex selectedBGQuad[4];
	sf::Vertex containerBGQuad[4];
	sf::Vertex *itemSelectQuads;

	sf::Text upgradeNameText;
	sf::Text upgradeDescText;
	sf::Text upgradeLevelText;

	int numTotalStoreEntries;

	std::vector<StoreEntry*> basicEntries;
	std::vector<std::vector<StoreEntry*>> allItemEntries;
	std::vector<StoreEntry*> powerEntries;

	std::vector<std::vector<StoreEntry*>*> currStoreEntries;

	//sf::Vertex shardBGQuad[4];
	//sf::Vertex descriptionBGQuad[4];
	//sf::Vertex shardTitleBGQuad[4];

	KinStore();
	~KinStore();
	void SetWorld(int w); //0 - 7
	bool IsReadyToClose();
	void SetSelected( int section, int itemIndex );
	void SetTopLeft(sf::Vector2f pos);
	void Update();
	void Open();
	void LoadStore();
	void LoadEntryFile(std::vector<StoreEntry*> &vec, const std::string &fileName, int startingQuadIndex );
	void Draw(sf::RenderTarget *target);
};

#endif

