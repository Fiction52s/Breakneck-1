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
	string upgradeString = upgrade["Upgrade"].get<std::string>();

	upgradeIndex = 0; //should be set from the string


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

KinStore::KinStore()
{
	MainMenu *mm = MainMenu::GetInstance();
	sess = NULL;

	CreateDescriptionTable();

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
	int xSize = 3;
	int ySize = 3;

	xSelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, xSize, 0);
	ySelector = new SingleAxisSelector(3, waitFrames, 2, waitModeThresh, ySize, 0);

	itemSelectQuads = new sf::Vertex[xSize * ySize * 4];

	currentStoreItems = new int[xSize * ySize];

	SetTopLeft(Vector2f(50, 50));
}

KinStore::~KinStore()
{
	delete xSelector;
	delete ySelector;

	delete[] itemSelectQuads;

	delete[] currentStoreItems;

	for (int i = 0; i < items.size(); ++i)
	{
		delete items[i];
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

	for (int i = 0; i < ySelector->totalItems; ++i)
	{
		for (int j = 0; j < xSelector->totalItems; ++j)
		{
			index = (i * xSelector->totalItems + j);

			if (index >= 20)
			{
				assert(0);
			}

			SetRectCenter(itemSelectQuads + index * 4, rectSize, rectSize, Vector2f(j * rectSize + xSpacing * j, i * rectSize + ySpacing * i) + gridStart);
			SetRectColor(itemSelectQuads + index * 4, Color::Green);
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

	for (int i = 0; i < xSelector->totalItems * ySelector->totalItems; ++i)
	{
		int optionIndex = 0;//(rand() % (UPGRADE_W1_BASE_DASH_1 - UPGRADE_W1_DASH_BOOST) + UPGRADE_W1_DASH_BOOST);
		currentStoreItems[i] = optionIndex;
	}
	
	//sess->SetPlayerOption(optionIndex, true);
	//sess->mainMenu->rushManager->UnlockUpgrade(optionIndex);
}

bool KinStore::IsReadyToClose()
{
	return action == A_READY_TO_CLOSE;
}

void KinStore::SetSelectedIndex(int ind)
{
	selectedIndex = ind;

	int selectedUpgradeIndex = currentStoreItems[selectedIndex];

	//eventually need to be able to display buttons in here, steal functionality from the tutbox
	auto &entry = upgradeDescriptionStringTable[selectedUpgradeIndex];
	upgradeNameText.setString( entry.first );
	upgradeDescText.setString(entry.second);

	SetRectCenter(selectedBGQuad, 192 / 2, 192 / 2,
		Vector2f((itemSelectQuads + selectedIndex * 4)->position + Vector2f(192 / 4, 192 / 4)));
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

		int index = xSelector->currIndex + ySelector->currIndex * xSelector->totalItems;

		SetSelectedIndex(index);
	}
}


void KinStore::CreateDescriptionTable()
{
	upgradeDescriptionStringTable.resize(200); //just placeholder big number

	vector<string> upgradeTypes = { "powers" };// , "power_upgrades", "speed_upgrades"

	string base = "Resources/Kin/Info/";
	stringstream ss;

	for (auto it = upgradeTypes.begin(); it != upgradeTypes.end(); ++it)
	{
		ss.clear();
		ss << base << (*it) << ".json";

		ifstream is;
		is.open(ss.str());

		json j;
		is >> j;

		auto &upgrades = j["Upgrades"];
		int numLevels = 0;
		int discSize = 0;
		for (int i = 0; i < upgrades.size(); ++i)
		{
			items.push_back(new StoreItem(upgrades[i]));
			/*cout << upgrades[i]["Upgrade"].get<std::string>() << endl;
			cout << upgrades[i]["Name"].get<std::string>() << endl;

			auto &levels = upgrades[i]["Levels"];
			numLevels = levels.size();
			for (int j = 0; j < numLevels; ++j)
			{
				cout << "level " << j + 1 << "\n";
				cout << "Cost: " << levels[j]["Cost"] << "\n";
				auto &disc = levels[j]["Description"];
				discSize = disc.size();
				cout << "Description: ";
				for (int k = 0; k < discSize; ++k)
				{
					cout << disc[k].get<std::string>() << "\n";
				}
			}*/
		}

		for (int i = 0; i < items.size(); ++i)
		{
			items[i]->Print();
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

void KinStore::SetTableEntry(int index, const std::string & s1, const std::string &s2)
{
	upgradeDescriptionStringTable[index] = std::make_pair(s1, s2);
}

void KinStore::Draw(sf::RenderTarget *target)
{
	target->draw(containerBGQuad, 4, sf::Quads );
	
	target->draw(itemSelectQuads, xSelector->totalItems * ySelector->totalItems * 4, sf::Quads);
	target->draw(selectedBGQuad, 4, sf::Quads);

	target->draw(upgradeNameText);
	target->draw(upgradeDescText);
}