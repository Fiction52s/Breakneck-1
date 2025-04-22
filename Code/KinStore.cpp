#include "Tileset.h"
#include "KinStore.h"
#include "MainMenu.h"
#include "Session.h"
#include "KinUpgrades.h"
#include "nlohmann\json.hpp"
#include "RushManager.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

using json = nlohmann::json;

StoreEntry::StoreEntry( nlohmann::basic_json<> &upgrade)
{
	currentLevel = 0;

	string upgradeString = upgrade["Upgrade"].get<std::string>();

	upgradeIndex = 0; //should be set from the string

	if (upgradeString == "POWER_AIR_DASH")
	{
		upgradeIndex = POWER_AIR_DASH;
	}
	else if (upgradeString == "POWER_GRAVITY_CLING")
	{
		upgradeIndex = POWER_GRAVITY_CLING;
	}
	else if (upgradeString == "POWER_BOUNCE_SCORPION")
	{
		upgradeIndex = POWER_BOUNCE_SCORPION;
	}
	else if (upgradeString == "POWER_GRIND_BALL")
	{
		upgradeIndex = POWER_GRIND_BALL;
	}
	else if (upgradeString == "POWER_HOMING_RUSH")
	{
		upgradeIndex = POWER_HOMING_RUSH;
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
	int effectsSize = 0;
	string effectStr;

	upgradeEffects.resize(numLevels);

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

		ss.clear();
		ss.str("");

		auto &effects = levels[i]["Effect"];
		effectsSize = effects.size();
		for (int j = 0; j < effectsSize; ++j)
		{
			effectStr = effects[j].get<std::string>();
			upgradeEffects[i].push_back(GetUpgradeEffect(effectStr));
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

int StoreEntry::GetUpgradeEffect(const std::string &str)
{
	if (str == "SPEED")
	{
		return UE_SPEED;
	}
	else if (str == "REDUCED_DAMAGE")
	{
		return UE_REDUCED_DAMAGE;
	}
	else if( str == "COUNTERHIT")
	{
		return UE_COUNTERHIT;
	}
	else if( str == "CHARGEATTACK")
	{
		return UE_CHARGEATTACK;
	}
	else if( str == "FASTER_KILL")
	{
		return UE_FASTER_KILL;
	}
	else if( str == "SWORD_BEAMS")
	{
		return UE_SWORD_BEAMS;
	}
	else if (str == "DASH_SPEED")
	{
		return UE_DASH_SPEED;
	}
	else if (str == "DASH_BOOST")
	{
		return UE_DASH_BOOST;
	}
	else if( str == "ATTACK_THROUGH_BULLETS")
	{
		return UE_ATTACK_THROUGH_BULLETS;
	}
	else if( str == "INCREASE_HP_GAIN_ON_HIT")
	{
		return UE_INCREASE_HP_GAIN_ON_HIT;
	}
	else if( str == "INCREASE_STARTING_MOMENTUM")
	{
		return UE_INCREASE_STARTING_MOMENTUM;
	}
	else if( str == "INCREASE_STARTING_HEALTH")
	{
		return UE_INCREASE_STARTING_HEALTH;
	}
	else if( str == "INCREASE_HEALTH_REGEN_FROM_CURRENCY")
	{
		return UE_INCREASE_HEALTH_REGEN_FROM_CURRENCY;
	}
	else if( str == "INCREASE_MOMENTUM_METER_FROM_ENEMIES")
	{
		return UE_INCREASE_MOMENTUM_METER_FROM_ENEMIES;
	}
	else if( str == "LONGER_SURVIVAL_MODE")
	{
		return UE_LONGER_SURVIVAL_MODE;
	}
	else if( str == "AIR_DASH_UNLOCK")
	{
		return UE_AIR_DASH_UNLOCK;
	}
	else if( str == "AIR_DASH_BOOST")
	{
		return UE_AIR_DASH_BOOST;
	}
	else if( str == "AIR_DASH_SPEED")
	{
		return UE_AIR_DASH_SPEED;
	}
	else if( str == "AIR_DASH_BOOST_2")
	{
		return UE_AIR_DASH_BOOST_2;
	}
	else if( str == "GRAVITY_CLING_UNLOCK")
	{
		return UE_GRAVITY_CLING_UNLOCK;
	}
	else if( str == "GRAVITY_CEILING_SPEED")
	{
		return UE_GRAVITY_CEILING_SPEED;
	}
	else if( str == "GRAVITY_CEILING_CHARGE_ATTACK")
	{
		return UE_GRAVITY_CEILING_CHARGE_ATTACK;
	}
	else if( str == "BOUNCE_SCORPION_UNLOCK")
	{
		return UE_BOUNCE_SCORPION_UNLOCK;
	}
	else if( str == "BOUNCE_BOOST_INCREASE")
	{
		return UE_BOUNCE_BOOST_INCREASE;
	}
	else if( str == "BOUNCE_JUMP_INCREASE")
	{
		return UE_BOUNCE_JUMP_INCREASE;
	}
	else if( str == "BOUNCE_DOUBLE_JUMP_INCREASE")
	{
		return UE_BOUNCE_DOUBLE_JUMP_INCREASE;
	}
	else if( str == "BOUNCE_TRIPLE_JUMP")
	{
		return UE_BOUNCE_TRIPLE_JUMP;
	}
	else if( str == "BOUNCE_ATTACK")
	{
		return UE_BOUNCE_ATTACK;
	}
	else if( str == "GRIND_BALL_UNLOCK")
	{
		return UE_GRIND_BALL_UNLOCK;
	}
	else if( str == "GRIND_EXTEND_LENGTH")
	{
		return UE_GRIND_EXTEND_LENGTH;
	}
	else if( str == "GRIND_SPEED_BOOST")
	{
		return UE_GRIND_SPEED_BOOST;
	}
	else if( str == "GRIND_ATTACK")
	{
		return UE_GRIND_ATTACK;
	}
	else if( str == "GRIND_LUNGE")
	{
		return UE_GRIND_LUNGE;
	}
	else if( str == "HOMING_RUSH_UNLOCK")
	{
		return UE_HOMING_RUSH_UNLOCK;
	}
	else if( str == "HOMING_INCREASE_RADIUS")
	{
		return UE_HOMING_INCREASE_RADIUS;
	}
	else if( str == "HOMING_BOOST_ATTACK")
	{
		return UE_HOMING_BOOST_ATTACK;
	}
	else if( str == "HOMING_CURRENCY_ACCELERATION_BOOST")
	{
		return UE_HOMING_CURRENCY_ACCELERATION_BOOST;
	}
	else if( str == "DOUBLE_WIRES_UNLOCK")
	{
		return UE_DOUBLE_WIRES_UNLOCK;
	}
	else if( str == "WIRES_INCREASED_RANGE")
	{
		return UE_WIRES_INCREASED_RANGE;
	}
	else if( str == "WIRES_ATTACH_TO_ENEMIES")
	{
		return UE_WIRES_ATTACH_TO_ENEMIES;
	}
	else if( str == "WIRE_STUN")
	{
		return UE_WIRES_STUN;
	}
	else
	{
		cout << "bad string detected for upgrades: " << str << endl;
		assert(0);
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

	TilesetManager *tm = NULL;



	edit = EditSession::GetSession();

	if ( edit != NULL)
	{
		tm = edit;
		storePanel = new Panel("store", 1920, 1080, this, true);
		storePanel->SetColor(Color::Transparent);
	}
	else
	{
		tm = mm->rushManager;
		storePanel = NULL;
	}

	ts_bg = tm->GetSizedTileset("Menu/Store/new_store_1920x1080.png");//rm->GetSizedTileset( "Menu/Store/Store_1920x1080.png");
	ts_yellowSquare = tm->GetSizedTileset( "Menu/Store/Yellow_Square_145x145.png");
	
	bgSpr.setTexture(*ts_bg->texture);
	yellowSpr.setTexture(*ts_yellowSquare->texture);

	action = -1;
	frame = -1;


	upgradeNameText.setCharacterSize(60);
	upgradeNameText.setFont(mm->arial);
	upgradeNameText.setFillColor(Color::White);

	upgradeLevelText.setCharacterSize(20);
	upgradeLevelText.setFont(mm->arial);
	upgradeLevelText.setFillColor(Color::White);

	storePointsText.setCharacterSize(60);
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
	int numPowers = 6;

	numTotalStoreEntries = numBasics + numPowers;

	itemSelectQuads = new sf::Vertex[numTotalStoreEntries * 4];


	if (storePanel != NULL)
	{
		storeRects.resize(numTotalStoreEntries);
		storePanel->ReserveImageRects(numTotalStoreEntries);

		for (int i = 0; i < numTotalStoreEntries; ++i)
		{
			storeRects[i] = storePanel->AddImageRect(
				ChooseRect::ChooseRectIdentity::I_STORE_UPGRADE,Vector2f( 0, 0 ),NULL, 0,100);
		}
	}
	

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

	for (auto it = power1Entries.begin(); it != power1Entries.end(); ++it)
	{
		delete (*it);
	}

	for (auto it = power2Entries.begin(); it != power2Entries.end(); ++it)
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
	Vector2f rectPos;

	for (int y = 0; y < SECTION_Count; ++y)
	{
		for (int j = 0; j < currStoreEntries[y]->size(); ++j)
		{
			//index = (i * xSelector->totalItems + j);

			/*if (index >= 20)
			{
				assert(0);
			}*/

			rectPos = Vector2f(j * rectSize + xSpacing * j, y * rectSize + ySpacing * y) + gridStart;

			SetRectCenter(itemSelectQuads + currIndex * 4, rectSize, rectSize, rectPos);
			SetRectColor(itemSelectQuads + currIndex * 4, Color::Green);

			storeRects[currIndex]->SetSize( Vector2f(rectSize, rectSize));
			//storeRects[currIndex]->SetPosition(rectPos);
			
			storeRects[currIndex]->SetInfo((void*)currIndex);
			++currIndex;
		}
	}

	storeRects[0]->SetPosition(Vector2f(256, 262));
	storeRects[1]->SetPosition(Vector2f(423, 262));
	storeRects[2]->SetPosition(Vector2f(591, 263));
	storeRects[3]->SetPosition(Vector2f(758, 263));

	storeRects[4]->SetPosition(Vector2f(254, 488));
	storeRects[5]->SetPosition(Vector2f(490, 488));
	storeRects[6]->SetPosition(Vector2f(723, 488));

	storeRects[7]->SetPosition(Vector2f(253, 697));
	storeRects[8]->SetPosition(Vector2f(488, 697));
	storeRects[9]->SetPosition(Vector2f(723, 697));

	for (int i = 0; i < numTotalStoreEntries; ++i)
	{
		storeRects[i]->Init();
		storeRects[i]->SetShown(true);
	}

	storePointsText.setPosition(1152, 742);
	//upgradeNameText.setPosition(500 + pos.x, pos.y + 50);
	//upgradeDescText.setPosition(500 + pos.x, pos.y + 200 );
	//upgradeLevelText.setPosition(500 + pos.x, pos.y + 400);
}

#include "RushManager.h"
//assuming I'm in rush manager
void KinStore::Open()
{
	if (edit != NULL)
	{
		rush = NULL;
		SetWorld(0);

		for (auto it = basicEntries.begin(); it != basicEntries.end(); ++it)
		{
			(*it)->currentLevel = edit->defaultStartingPlayerUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
		}

		for (auto it = power1Entries.begin(); it != power1Entries.end(); ++it)
		{
			(*it)->currentLevel = edit->defaultStartingPlayerUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
		}

		for (auto it = power2Entries.begin(); it != power2Entries.end(); ++it)
		{
			(*it)->currentLevel = edit->defaultStartingPlayerUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
		}

		storePointsText.setString("Infinite Upgrade points");
	}
	else
	{
		rush = sess->mainMenu->rushManager;

		assert(rush != NULL);
		SetWorld(rush->currWorld); //0 - 7

		for (auto it = basicEntries.begin(); it != basicEntries.end(); ++it)
		{
			(*it)->currentLevel = rush->kinUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
		}

		for (auto it = power1Entries.begin(); it != power1Entries.end(); ++it)
		{
			(*it)->currentLevel = rush->kinUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
		}

		for (auto it = power2Entries.begin(); it != power2Entries.end(); ++it)
		{
			(*it)->currentLevel = rush->kinUpgradeLevels->GetUpgradeLevel((*it)->upgradeIndex);
		}

		storePointsText.setString(to_string(rush->storePoints) + " Upgrade points");
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

	ySelector->currIndex = section;

	if (section == 0)
	{
		xSelector->SetTotalSize(4);
	}
	else
	{
		xSelector->SetTotalSize(3);
	}

	xSelector->currIndex = itemIndex;

	int upgradeLevel = 0;
	if (edit != NULL)
	{
		upgradeLevel = edit->defaultStartingPlayerUpgradeLevels->GetUpgradeLevel(si->upgradeIndex);
	}
	else
	{
		upgradeLevel = rush->kinUpgradeLevels->GetUpgradeLevel(si->upgradeIndex);
	}

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
			upgradeNameText.getLocalBounds().top + upgradeNameText.getLocalBounds().height / 2);

	//upgradeNameText.setPosition(1088,303);
	upgradeNameText.setPosition(1369,240);

	
	upgradeDescText.setPosition(1051, 374);
	

	SetRectCenter(selectedBGQuad, 192 / 2, 192 / 2,
		Vector2f((itemSelectQuads + si->quadIndex * 4)->position + Vector2f(192 / 4, 192 / 4)));
	SetRectColor(selectedBGQuad, Color::White);

	Vector2f selectTopLeft;
	switch (section)
	{
	case 0:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(256, 262);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(423, 262);
		}
		else if (itemIndex == 2)
		{
			selectTopLeft = Vector2f(591, 263);
		}
		else if (itemIndex == 3)
		{
			selectTopLeft = Vector2f(758, 263);
		}
		break;
	case 1:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(254, 488);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(490, 488);
		}
		else if (itemIndex == 2)
		{
			selectTopLeft = Vector2f(723, 488);
		}
		break;
	case 2:
		if (itemIndex == 0)
		{
			selectTopLeft = Vector2f(253, 697);
		}
		else if (itemIndex == 1)
		{
			selectTopLeft = Vector2f(488, 697);
		}
		else if (itemIndex == 2)
		{
			selectTopLeft = Vector2f(723, 697);
		}
		break;
	}

	selectTopLeft += Vector2f(-10, -10);

	if (edit != NULL)
	{
		storePointsText.setFillColor(Color::Blue);
	}
	else if (si->GetCurrentCost() > rush->storePoints)
	{
		storePointsText.setFillColor(Color::Red);
	}
	else
	{
		storePointsText.setFillColor(Color::Blue);
	}

	yellowSpr.setPosition(selectTopLeft);
}

void KinStore::TryUnlockCurrentUpgrade()
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
	if (rush != NULL && cost > rush->storePoints)
	{
		return;
	}

	if (se->currentLevel < se->numLevels)
	{
		if (rush != NULL)
		{
			rush->storePoints -= se->GetCurrentCost();
			storePointsText.setString(to_string(rush->storePoints) + " Upgrade points");
			sess->SetPlayerUpgradeLevel(optionIndex, se->currentLevel + 1);
			sess->mainMenu->rushManager->UnlockUpgrade(optionIndex, se->currentLevel + 1);
		}
		else
		{
			edit->defaultStartingPlayerUpgradeLevels->SetUpgradeLevel(optionIndex, se->currentLevel + 1);
		}

		se->currentLevel++;

		SetSelected(ySelector->currIndex, xSelector->currIndex);

		//action = A_READY_TO_CLOSE;
		//frame = 0;

		SetRectColor(containerBGQuad, Color(255, 0, 0, 128));


	}
}

void KinStore::Update()
{
	if (action == A_OPEN)
	{
		auto *inputStates = sess->controllerStates[0];
		bool aPressed = inputStates->ButtonPressed_A();
		bool startPressed = inputStates->ButtonPressed_Start();

		if (startPressed || ( rush != NULL && rush->storePoints == 0 ) )
		{
			action = A_READY_TO_CLOSE;
			frame = 0;
			return;
		}

		if (aPressed)
		{
			TryUnlockCurrentUpgrade();
			return;
		}

		int ychanged = ySelector->UpdateIndex(inputStates->DirHold_Up() || inputStates->PadDirHold_Up(), inputStates->DirHold_Down() || inputStates->PadDirHold_Down());

		if (ychanged != 0)
		{
			//xSelector->SetIndex(0);
			xSelector->SetTotalSize(currStoreEntries[ySelector->currIndex]->size());
		}

		int xchanged = xSelector->UpdateIndex(inputStates->DirHold_Left() || inputStates->PadDirHold_Left(), inputStates->DirHold_Right() || inputStates->PadDirHold_Right());

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

		for (int k = 0; k < si->upgradeEffects.size(); ++k)
		{
			for (auto it = si->upgradeEffects[k].begin(); it != si->upgradeEffects[k].end(); ++it)
			{
				upgradeEffectMap[(*it)].push_back(make_pair(si->upgradeIndex, k+1));
			}
		}
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

	currQuadIndex += 4;//basicEntries.size();	

	//load items
	/*for (int i = 0; i < 8; ++i)
	{
		LoadEntryFile(allItemEntries[i], "items_w" + to_string( i + 1 ), currQuadIndex);
	}*/

	//currQuadIndex += 4; //4 is the max number of items per world, its set even if the entries are empty
	
	//load powers
	LoadEntryFile(power1Entries, "powers1", currQuadIndex);

	currQuadIndex += 3;//power1Entries.size();

	LoadEntryFile(power2Entries, "powers2", currQuadIndex);

	//currQuadIndex += powerEntries.size();

	currStoreEntries[SECTION_BASICS] = &basicEntries;
	//currStoreEntries[SECTION_ITEMS] = NULL;
	currStoreEntries[SECTION_POWERS1] = &power1Entries;
	currStoreEntries[SECTION_POWERS2] = &power2Entries;

}

void KinStore::Draw(sf::RenderTarget *target)
{
	//target->draw(containerBGQuad, 4, sf::Quads );
	target->draw(bgSpr);
	
	//target->draw(itemSelectQuads, numTotalStoreEntries * 4, sf::Quads);
	//target->draw(selectedBGQuad, 4, sf::Quads);

	target->draw(storePointsText);
	target->draw(yellowSpr);

	target->draw(upgradeNameText);
	target->draw(upgradeDescText);
	//target->draw(upgradeLevelText);
}

void KinStore::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::E_LEFTCLICKED) //||
//		eventType == ChooseRect::E_LEFTRELEASED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_STORE_UPGRADE)
			{
				int ind = (int)cr->info;
				TryUnlockCurrentUpgrade();
			}
		}
	}
	else if (eventType == ChooseRect::E_FOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();

		if (icRect->rectIdentity == ChooseRect::I_STORE_UPGRADE)
		{
			int ind = (int)cr->info;
			int col = 0;
			int row = 0;
			if (ind < 4)
			{
				row = 0;
				col = ind;
			}
			else if (ind < 4 + 3)
			{
				row = 1;
				col = ind - 4;
			}
			else
			{
				row = 2;
				col = ind - (4 + 3);
			}

			SetSelected(row, col);
		}
	}
	else if (eventType == ChooseRect::E_UNFOCUSED)
	{
		ImageChooseRect *icRect = cr->GetAsImageChooseRect();
		if (icRect != NULL)
		{
			if (icRect->rectIdentity == ChooseRect::I_STORE_UPGRADE)
			{
				//edit->showLogNameText = false;
			}
		}
	}
}