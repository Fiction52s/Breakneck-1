#include "Tileset.h"
#include "KinStore.h"
#include "MainMenu.h"
#include "Session.h"
#include "KinUpgrades.h"
#include "nlohmann\json.hpp"
#include "RushManager.h"

using namespace sf;
using namespace std;

using json = nlohmann::json;

StoreEntry::StoreEntry( nlohmann::basic_json<> &upgrade)
{
	currentLevel = 0;

	string upgradeString = upgrade["Upgrade"].get<std::string>();

	upgradeIndex = 0; //should be set from the string

	if (upgradeString == "POWER_AIRDASH")
	{
		upgradeIndex = POWER_AIRDASH;
	}
	else if (upgradeString == "POWER_GRAV")
	{
		upgradeIndex = POWER_GRAV;
	}
	else if (upgradeString == "POWER_BOUNCE")
	{
		upgradeIndex = POWER_BOUNCE;
	}
	else if (upgradeString == "POWER_GRIND")
	{
		upgradeIndex = POWER_GRIND;
	}
	else if (upgradeString == "POWER_TIME")
	{
		upgradeIndex = POWER_TIME;
	}
	else if (upgradeString == "POWER_DOUBLE_WIRES")
	{
		upgradeIndex = POWER_DOUBLE_WIRES;
	}
	else if (upgradeString == "UPGRADE_SPEED")
	{
		upgradeIndex = UPGRADE_SPEED;
	}
	else if (upgradeString == "UPGRADE_ENERGY")
	{
		upgradeIndex = UPGRADE_ENERGY;
	}
	else if (upgradeString == "UPGRADE_COMBAT")
	{
		upgradeIndex = UPGRADE_COMBAT;
	}
	else if (upgradeString == "UPGRADE_DASH")
	{
		upgradeIndex = UPGRADE_DASH;
	}

	name = upgrade["Name"].get<std::string>();

	auto &levels = upgrade["Levels"];
	numLevels = levels.size();
	descriptions.resize(numLevels);
	costs.resize(numLevels);

	stringstream ss;

	int descSize = 0;
	for (int i = 0; i < numLevels; ++i)
	{
		ss.clear();
		ss.str("");

		auto &desc = levels[i]["Description"];
		descSize = desc.size();
		for (int j = 0; j < descSize; ++j)
		{
			ss << desc[j].get<std::string>();
			if (j < descSize - 1)
			{
				ss << "\n";
			}
			descriptions[i] = ss.str();
		}

		costs[i] = levels[i]["Cost"];
	}
}

void StoreEntry::Print()
{
	cout << "name: " << name << "\n";
	cout << "upgradeIndex: " << upgradeIndex << "\n";
	cout << "numLevels: " << numLevels << "\n";
	for (int i = 0; i < numLevels; ++i)
	{
		cout << "Description: " << descriptions[i] << "\n";
		cout << "Cost: " << costs[i] << "\n";
	}
}

const std::string &StoreEntry::GetCurrentDescription()
{
	return descriptions[currentLevel];
}

int StoreEntry::GetCurrentCost()
{
	return costs[currentLevel];
}

KinStore::KinStore()
{
	MainMenu *mm = MainMenu::GetInstance();
	sess = NULL;

	

	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));

	RushManager *rm = mm->rushManager;
	assert(rm != NULL);
	ts_bg = rm->GetSizedTileset("Menu/Store/store_bg_placeholder_1920x1080.png");//rm->GetSizedTileset( "Menu/Store/Store_1920x1080.png");
	ts_yellowSquare = rm->GetSizedTileset( "Menu/Store/Yellow_Square_145x145.png");
	
	bgSpr.setTexture(*ts_bg->texture);
	yellowSpr.setTexture(*ts_yellowSquare->texture);
	sf::Sprite yellowSpr;

	action = -1;
	frame = -1;


	upgradeNameText.setCharacterSize(60);
	upgradeNameText.setFont(mm->arial);
	upgradeNameText.setFillColor(Color::White);

	upgradeLevelText.setCharacterSize(20);
	upgradeLevelText.setFont(mm->arial);
	upgradeLevelText.setFillColor(Color::White);

	storePointsText.setCharacterSize(40);
	storePointsText.setFont(mm->arial);
	storePointsText.setFillColor(Color::Red);

	
	

	upgradeDescText.setCharacterSize(27);
	upgradeDescText.setFont(mm->arial);
	upgradeDescText.setFillColor(Color::White);

	int waitFrames[3] = { 60, 20, 10 };
	int waitModeThresh[2] = { 2, 4 };
	int maxXSize = 2;
	int ySize = 4;

	//basics, powers, armor items, use items
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, maxXSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	LoadStore();

	int numBasics = 4;
	int numItems = 4;
	int numPowers = 6;

	numTotalStoreEntries = numBasics + numItems + numPowers;

	itemSelectQuads = new sf::Vertex[numTotalStoreEntries * 4];

	//currentStoreItems = new int[maxXSize * ySize];

	
}

KinStore::~KinStore()
{

	delete xSelector;
	delete ySelector;

	delete[] itemSelectQuads;

	//delete[] currentStoreItems;

	for (auto it = basicEntries.begin(); it != basicEntries.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = allItemEntries.begin(); it != allItemEntries.end(); ++it)
	{
		for (auto it2 = (*it).begin(); it2 != (*it).end(); ++it2)
		{
			delete (*it2);
		}
	}

	for (auto it = powerEntries.begin(); it != powerEntries.end(); ++it)
	{
		delete (*it);
	}
}

void KinStore::SetTopLeft(sf::Vector2f pos)
{
	Vector2f testSize(1820, 980);

	//Vector2f shardBGQuadTopLeft = Vector2f(edgeMargin, edgeMargin) + pos;
	SetRectTopLeft(containerBGQuad, testSize.x, testSize.y, pos);

	int index = 0;
	int rectSize = 192 / 2;
	int xSpacing = 20 * 2;
	int ySpacing = 12 * 2;

	Vector2f gridStart(150, 200);

	gridStart += pos;

	int currIndex = 0;
	for (int y = 0; y < SECTION_Count; ++y)
	{
		for (int j = 0; j < currStoreEntries[y]->size(); ++j)
		{
			//index = (i * xSelector->totalItems + j);

			/*if (index >= 20)
			{
				assert(0);
			}*/

			SetRectCenter(itemSelectQuads + currIndex * 4, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, y * rectSize + ySpacing * y) + gridStart);
			SetRectColor(itemSelectQuads + currIndex * 4, Color::Green);

			++currIndex;
		}
	}

	storePointsText.setPosition(20, 20);
	//upgradeNameText.setPosition(500 + pos.x, pos.y + 50);
	//upgradeDescText.setPosition(500 + pos.x, pos.y + 200 );
	//upgradeLevelText.setPosition(500 + pos.x, pos.y + 400);
}

#include "RushManager.h"
//assuming I'm in rush manager
void KinStore::Open()
{
	rush = sess->mainMenu->rushManager;

	

	SetWorld(rush->currWorld); //0 - 7

	for (auto it = basicEntries.begin(); it != basicEntries.end(); ++it)
	{
		(*it)->currentLevel = rush->kinUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
	}

	for (auto it = powerEntries.begin(); it != powerEntries.end(); ++it)
	{
		(*it)->currentLevel = rush->kinUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
	}

	SetTopLeft(Vector2f(0, 0));

	//all items remain at level 0 for now

	action = A_OPEN;
	frame = 0;

	ySelector->SetIndex(0);
	xSelector->SetIndex(0);
	xSelector->SetTotalSize(4); //4 basics
	//currStoreEntries[ySelector->currIndex].size());

	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));

	SetSelected(0, 0);

	storePointsText.setString(to_string(rush->storePoints));
	//for (int i = 0; i < xSelector->totalItems * ySelector->totalItems; ++i)
	//{
	//	int optionIndex = 0;//(rand() % (UPGRADE_W1_BASE_DASH_1 - UPGRADE_W1_DASH_BOOST) + UPGRADE_W1_DASH_BOOST);
	//	//currentStoreItems[i] = optionIndex;
	//}
	
	//sess->SetPlayerOption(optionIndex, true);
	//sess->mainMenu->rushManager->UnlockUpgrade(optionIndex);
}

void KinStore::SetWorld(int w) //0-7
{
	//currStoreEntries[SECTION_ITEMS] = &allItemEntries[w];
}

bool KinStore::IsReadyToClose()
{
	return action == A_READY_TO_CLOSE;
}

void KinStore::SetSelected(int section, int itemIndex)
{
	//eventually need to be able to display buttons in here, steal functionality from the tutbox
	StoreEntry *si = currStoreEntries[section]->at(itemIndex);

	int upgradeLevel = sess->mainMenu->rushManager->kinUpgradeLevels->GetUpgradeLevel(si->upgradeIndex);


	if (upgradeLevel == si->numLevels)
	{
		upgradeNameText.setString("Max Leveled Already");
		upgradeDescText.setString("N/A");
	}
	else
	{
		upgradeNameText.setString(si->name + " Level " + to_string(upgradeLevel + 1));
		//upgradeLevelText.setString("Level: " + to_string(upgradeLevel));
		upgradeDescText.setString( "Cost: " + to_string(si->GetCurrentCost()) + " desc: " + si->GetCurrentDescription());
	}

	upgradeNameText.setOrigin(upgradeNameText.getLocalBounds().left
			+ upgradeNameText.getLocalBounds().width / 2,
			upgradeNameText.getLocalBounds().top);

	//upgradeNameText.setPosition(1088,303);
	upgradeNameText.setPosition(1200,303);

	
	upgradeDescText.setPosition(745, 412);
	

	SetRectCenter(selectedBGQuad, 192 / 2, 192 / 2,
		Vector2f((itemSelectQuads + si->quadIndex * 4)->position + Vector2f(192 / 4, 192 / 4)));
	SetRectColor(selectedBGQuad, Color::White);

	Vector2f selectTopLeft;
	switch (section)
	{
	case 0:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(224, 229);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(423, 228);
		}
		break;
	case 1:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(221, 482);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(421, 482);
		}
		break;
	case 2:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(221, 640);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(421, 640);
		}
		break;
	case 3:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(221, 795);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(421, 795);
		}
		break;
	}

	selectTopLeft += Vector2f(-10, -10);

	if (si->GetCurrentCost() > rush->storePoints)
	{
		storePointsText.setFillColor(Color::Red);
		return;
	}
	else
	{
		storePointsText.setFillColor(Color::Blue);
		return;
	}

	yellowSpr.setPosition(selectTopLeft);
}

void KinStore::Update()
{
	if (action == A_OPEN)
	{
		auto *inputStates = sess->controllerStates[0];
		bool aPressed = inputStates->ButtonPressed_A();
		bool startPressed = inputStates->ButtonPressed_Start();

		if (startPressed || rush->storePoints == 0 )
		{
			action = A_READY_TO_CLOSE;
			frame = 0;
			return;
		}

		if (aPressed)
		{
			//int optionIndex = (rand() % (UPGRADE_W1_BASE_DASH_1 - UPGRADE_W1_DASH_BOOST) + UPGRADE_W1_DASH_BOOST);

			//sess->SetPlayerOption(optionIndex, true);
			//sess->mainMenu->rushManager->UnlockUpgrade(optionIndex);

			//if (optionIndex == 5)
			//{
			//	//left wire also
			//	sess->SetPlayerOption(optionIndex + 1, true);
			//	sess->mainMenu->rushManager->kinOptionField.SetBit(optionIndex + 1, true);
			//}
			StoreEntry *se = currStoreEntries[ySelector->currIndex]->at(xSelector->currIndex);
			int optionIndex = se->upgradeIndex;

			int cost = se->GetCurrentCost();
			if ( cost > rush->storePoints)
			{
				return;
			}

			if (se->currentLevel < se->numLevels)
			{
				rush->storePoints -= se->GetCurrentCost();
				storePointsText.setString(to_string(rush->storePoints));

				sess->SetPlayerUpgradeLevel(optionIndex, se->currentLevel + 1);
				sess->mainMenu->rushManager->UnlockUpgrade(optionIndex, se->currentLevel + 1);
				se->currentLevel++;

				//action = A_READY_TO_CLOSE;
				//frame = 0;

				SetRectColor(containerBGQuad, Color(255, 0, 0, 128));

				
			}

			return;
		}

		int xchanged = xSelector->UpdateIndex(inputStates->DirHold_Left() || inputStates->PadDirHold_Left(), inputStates->DirHold_Right() || inputStates->PadDirHold_Right());
		int ychanged = ySelector->UpdateIndex(inputStates->DirHold_Up() || inputStates->PadDirHold_Up(), inputStates->DirHold_Down() || inputStates->PadDirHold_Down());

		if (ychanged != 0)
		{
			//xSelector->SetIndex(0);
			xSelector->SetTotalSize(currStoreEntries[ySelector->currIndex]->size());
		}

		if (xchanged != 0 || ychanged != 0)
		{
			SetSelected(ySelector->currIndex, xSelector->currIndex );
		}
	}
}

void KinStore::LoadEntryFile(std::vector<StoreEntry*> &vec, const std::string &fileName, int startingQuadIndex )
{
	string base = "Resources/Menu/Store/Info/";
	stringstream ss;
	ss << base << fileName << ".json";

	ifstream is;
	is.open(ss.str());

	json j;
	is >> j;

	auto &upgrades = j["Upgrades"];
	int numLevels = 0;
	int discSize = 0;
	StoreEntry *si = NULL;

	assert(vec.empty());
	//vec.clear();
	vec.reserve(upgrades.size());

	int quadIndex = startingQuadIndex;

	for (int j = 0; j < upgrades.size(); ++j)
	{
		si = new StoreEntry(upgrades[j]);
		si->quadIndex = quadIndex;
		vec.push_back(si);
		++quadIndex;
	}
}

void KinStore::LoadStore()
{
	currStoreEntries.resize(SECTION_Count);
	ySelector->SetTotalSize(SECTION_Count);
	allItemEntries.resize(8);
	
	int currQuadIndex = 0;

	//load basics
	LoadEntryFile(basicEntries, "basic_upgrades", currQuadIndex);

	currQuadIndex += basicEntries.size();	

	//load items
	/*for (int i = 0; i < 8; ++i)
	{
		LoadEntryFile(allItemEntries[i], "items_w" + to_string( i + 1 ), currQuadIndex);
	}*/

	//currQuadIndex += 4; //4 is the max number of items per world, its set even if the entries are empty
	
	//load powers
	LoadEntryFile(powerEntries, "powers", currQuadIndex);

	//currQuadIndex += powerEntries.size();

	currStoreEntries[SECTION_BASICS] = &basicEntries;
	//currStoreEntries[SECTION_ITEMS] = NULL;
	currStoreEntries[SECTION_POWERS] = &powerEntries;

}

void KinStore::Draw(sf::RenderTarget *target)
{
	//target->draw(containerBGQuad, 4, sf::Quads );
	target->draw(bgSpr);
	
	target->draw(itemSelectQuads, numTotalStoreEntries * 4, sf::Quads);
	target->draw(selectedBGQuad, 4, sf::Quads);

	target->draw(storePointsText);
	//target->draw(yellowSpr);

	target->draw(upgradeNameText);
	target->draw(upgradeDescText);
	//target->draw(upgradeLevelText);
}