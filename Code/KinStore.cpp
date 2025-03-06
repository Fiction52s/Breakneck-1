#include "Tileset.h"
#include "KinStore.h"
#include "MainMenu.h"
#include "Session.h"
#include "KinUpgrades.h"
#include "nlohmann\json.hpp"

using namespace sf;
using namespace std;

using json = nlohmann::json;

StoreItem::StoreItem( nlohmann::basic_json<> &upgrade)
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

void StoreItem::Print()
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

const std::string &StoreItem::GetCurrentDescription()
{
	return descriptions[currentLevel];
}

int StoreItem::GetCurrentCost()
{
	return costs[currentLevel];
}

KinStore::KinStore()
{
	MainMenu *mm = MainMenu::GetInstance();
	sess = NULL;

	LoadStore();

	SetRectColor(containerBGQuad, Color(0, 0, 0, 128));
	
	action = -1;
	frame = -1;


	upgradeNameText.setCharacterSize(20);
	upgradeNameText.setFont(mm->arial);
	upgradeNameText.setFillColor(Color::White);

	upgradeDescText.setCharacterSize(20);
	upgradeDescText.setFont(mm->arial);
	upgradeDescText.setFillColor(Color::White);

	int waitFrames[3] = { 60, 20, 10 };
	int waitModeThresh[2] = { 2, 4 };
	int maxXSize = 6;
	int ySize = 4;

	//basics, powers, armor items, use items
	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, maxXSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	numTotalStoreItems = 0;
	for (auto it = storeItems.begin(); it != storeItems.end(); ++it)
	{
		for (auto it2 = (*it).begin(); it2 != (*it).end(); ++it2)
		{
			++numTotalStoreItems;
		}
	}

	itemSelectQuads = new sf::Vertex[numTotalStoreItems * 4];

	//currentStoreItems = new int[maxXSize * ySize];

	SetTopLeft(Vector2f(50, 50));
}

KinStore::~KinStore()
{
	delete xSelector;
	delete ySelector;

	delete[] itemSelectQuads;

	//delete[] currentStoreItems;

	for (int i = 0; i < storeItems.size(); ++i)
	{
		for (int j = 0; j < storeItems[i].size(); ++j)
		{
			delete storeItems[i][j];
		}
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
	for (int y = 0; y < SS_Count; ++y)
	{
		for (int j = 0; j < storeItems[y].size(); ++j)
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

	upgradeNameText.setPosition(500 + pos.x, pos.y + 50);
	upgradeDescText.setPosition(500 + pos.x, pos.y + 200 );
}

#include "RushManager.h"
//assuming I'm in rush manager
void KinStore::Open()
{
	action = A_OPEN;
	frame = 0;

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

bool KinStore::IsReadyToClose()
{
	return action == A_READY_TO_CLOSE;
}

void KinStore::SetSelected(int section, int itemIndex)
{
	//eventually need to be able to display buttons in here, steal functionality from the tutbox
	StoreItem *si = storeItems[section][itemIndex];
	upgradeNameText.setString( si->name );
	upgradeDescText.setString(si->GetCurrentDescription());

	SetRectCenter(selectedBGQuad, 192 / 2, 192 / 2,
		Vector2f((itemSelectQuads + si->quadIndex * 4)->position + Vector2f(192 / 4, 192 / 4)));
	SetRectColor(selectedBGQuad, Color::White);
}

void KinStore::Update()
{
	if (action == A_OPEN)
	{
		auto *inputStates = sess->controllerStates[0];
		bool aPressed = inputStates->ButtonPressed_A();

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

			action = A_READY_TO_CLOSE;
			frame = 0;

			SetRectColor(containerBGQuad, Color(255, 0, 0, 128));

			return;
		}

		int xchanged = xSelector->UpdateIndex(inputStates->DirHold_Left() || inputStates->PadDirHold_Left(), inputStates->DirHold_Right() || inputStates->PadDirHold_Right());
		int ychanged = ySelector->UpdateIndex(inputStates->DirHold_Up() || inputStates->PadDirHold_Up(), inputStates->DirHold_Down() || inputStates->PadDirHold_Down());

		if (ychanged != 0)
		{
			xSelector->SetIndex(0);
			xSelector->SetTotalSize(storeItems[ySelector->currIndex].size());
		}

		if (xchanged != 0 || ychanged != 0)
		{
			SetSelected(ySelector->currIndex, xSelector->currIndex );
		}
	}
}


void KinStore::LoadStore()
{
	vector<string> upgradeTypes = { "basic_upgrades", "powers", "armor", "items" };

	string base = "Resources/Kin/Info/";
	stringstream ss;

	storeItems.resize(SS_Count);

	int totalStoreItemCounter = 0;
	for (int sectionIndex = 0; sectionIndex < upgradeTypes.size(); ++sectionIndex)
	{
		ss.clear();
		ss.str("");
		ss << base << upgradeTypes[sectionIndex] << ".json";

		ifstream is;
		is.open(ss.str());

		json j;
		is >> j;

		auto &upgrades = j["Upgrades"];
		int numLevels = 0;
		int discSize = 0;
		StoreItem *si = NULL;
		for (int j = 0; j < upgrades.size(); ++j)
		{
			si = new StoreItem(upgrades[j]);
			si->quadIndex = totalStoreItemCounter;
			storeItems[sectionIndex].push_back(si);
			++totalStoreItemCounter;
		}

		for (int j = 0; j < storeItems[sectionIndex].size(); ++j)
		{
			storeItems[sectionIndex][j]->Print();
			cout << "\n";
		}
	}

	/*if (is.is_open())
	{
		
		
	}
	else
	{
		cout << "could not open upgrades json file" << endl;
		assert(0);
		return;
	}*/


	/*SetTableEntry(POWER_AIRDASH, "Airdash",
		"-Hold DASH in the air to hover!\n"
		"-Hold DASH and a direction to airdash in any of the 8 directions!\n"
		"-Press ATTACK while Airdashing diagonally for a special attack!");

	SetTableEntry(POWER_GRAV, "Gravity Reverse",
		"-Hold DASH and up while touching a ceiling to reverse your gravity!\n"
		"-Gravity will remain reversed until you leave the ceiling you are on.");

	SetTableEntry(POWER_BOUNCE, "Bounce Scorpion",
		"-Use RLEFT to enter scorpion mode, and hold SHIELD to activate.\n"
		"-While on, you will bounce off of any surface you collide with!");

	SetTableEntry(POWER_GRIND, "Grind Wheel",
		"-Use RRIGHT to enter grind mode, and hold SHIELD to activate!\n"
		"-While on, you will move along your current surface regardless of slope.\n"
		"-Press ATTACK while grinding for a grind attack!");

	SetTableEntry(POWER_TIME, "Time Slow Bubble",
		"-Use RDOWN to enter time slow mode, and press SHIELD to create a bubble!\n"
		"-Enemies and bullets are slowed down while in a bubble.\n"
		"-Hold SHIELD while in a bubble to slow yourself down too!");

	SetTableEntry(POWER_DOUBLE_WIRES, "Double Wires",
		"-Use the double wires to swing and move around with total freedom!\n"
		"-Use LEFTWIRE to use the blue wire, and RIGHTWIRE to use the red wire!\n"
		"-Press a direction when launching the wire to aim it!\n"
		"-Keep holding the wire button after it is attached to swing from it!");

	SetTableEntry(UPGRADE_W1_DASH_BOOST, "Unlock Dash Boost",
		"Let go of dash near the end to get a boost of speed!");

	SetTableEntry(UPGRADE_W1_STEEP_CLIMB_1, "Upgrade Steep Climb 1/3",
		"Climb steep slopes faster!");

	SetTableEntry(UPGRADE_W1_STEEP_SLIDE_1, "Upgrade Steep Slide 1/3",
		"Slide down steep slopes faster!");

	SetTableEntry(UPGRADE_W1_PASSIVE_GROUND_1, "Upgrade Ground Acceleration 1/3",
		"Increased passive acceleration during grounded movement!");

	SetTableEntry(UPGRADE_W1_SPRINT_1, "Upgrade Sprint 1/3",
		"Increased acceleration from sprinting on slopes!");

	SetTableEntry(UPGRADE_W1_BASE_DASH_1, "Upgrade Base Dash Speed 1/3",
		"Dash speed increased!");*/


	//leftwire entry left blank for now, since right wire is double
}

void KinStore::Draw(sf::RenderTarget *target)
{
	target->draw(containerBGQuad, 4, sf::Quads );
	
	target->draw(itemSelectQuads, numTotalStoreItems * 4, sf::Quads);
	target->draw(selectedBGQuad, 4, sf::Quads);

	target->draw(upgradeNameText);
	target->draw(upgradeDescText);
}