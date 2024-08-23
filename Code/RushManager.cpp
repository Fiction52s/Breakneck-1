#include "RushManager.h"
#include "KinBoostScreen.h"
#include "SaveMenuScreen.h"
#include "WorldMap.h"
#include "MainMenu.h"
#include "GameSession.h"
#include "PlayerRecord.h"
#include "Leaderboard.h"
#include "md5.h"
#include "FeedbackForm.h"
#include "PauseMenu.h"
#include "HUD.h"
#include "WorldTransferScreen.h"
#include "globals.h"
#include "MapHeader.h"
#include "ShardMenu.h"
#include "KinExperienceBar.h"
#include "RushScoreDisplay.h"

#include "RushFile.h"

using namespace std;
using namespace sf;

RushManager::RushManager()
	:kinOptionField(Session::PLAYER_OPTION_BIT_COUNT)
{
	pauseMenu = new PauseMenu(this);

	firstMap = NULL;
	shipGame = NULL;

	kinUpgradesInOrder.reserve(128);

	worldMap = NULL;
	background = NULL;
	kinBoostScreen = NULL;
	worldTransferScreen = NULL;
	saveMenu = NULL;
	controllerInput = NULL;
	currProfile = NULL;

	transferPlayerPowerMode = -1;

	MainMenu * mm = MainMenu::GetInstance();

	adventureHUD = new AdventureHUD(this);

	currWorldDependentTilesetWorldIndex = -1;
	ts_key = NULL;
	ts_goal = NULL;
	ts_goalCrack = NULL;
	ts_goalExplode = NULL;

	currRushMapIndex = 0;
	currWorld = 0;
	
	rushScoreDisplay = new RushScoreDisplay(this, mm->arial);

	worldTransferScreen = new WorldTransferScreen;

	kinBoostScreen = new KinBoostScreen;

	SetCurrSaveFile(0);
}

RushManager::~RushManager()
{
	if (firstMap != NULL)
	{
		delete firstMap;
	}

	if (shipGame != NULL)
	{
		delete shipGame;
	}

	if (adventureHUD != NULL)
	{
		delete adventureHUD;
	}

	if (rushScoreDisplay != NULL)
	{
		delete rushScoreDisplay;
	}

	if (pauseMenu != NULL)
	{
		delete pauseMenu;
		pauseMenu = NULL;
	}

	if (worldMap != NULL)
	{
		delete worldMap;
	}

	if (kinBoostScreen != NULL)
	{
		delete kinBoostScreen;
	}

	if (worldTransferScreen != NULL)
	{
		delete worldTransferScreen;
	}

	if (saveMenu != NULL)
	{
		delete saveMenu;
	}
}

void RushManager::Load()
{
	LoadRush("test");
}

void RushManager::SetWorld(int w)
{
	UpdateWorldDependentTileset(w);

	MainMenu::GetInstance()->gameRunType = MainMenu::GRT_RUSH;

	for (int i = 0; i < bonusVec.size(); ++i )
	{
		delete bonusVec[i];
	}
	bonusVec.clear();

	if (firstMap != NULL)
	{
		delete firstMap;
		firstMap = NULL;
	}

	currWorld = w;

	currRushMapIndex = 0;

	if (currWorld == 0)
	{
		kinOptionField.Reset();
		kinUpgradesInOrder.clear();
	}

	MatchParams mp;
	mp.mapPath = rushFile.worlds[w].maps[0].GetMapPath();
	mp.randSeed = time(0);
	mp.numPlayers = 1;
	mp.gameModeType = MatchParams::GAME_MODE_BASIC;

	mp.controllerStateVec[0] = controllerInput;
	mp.controlProfiles[0] = currProfile;
	mp.playerSkins[0] = 0;//adventureManager->currSaveFile->visualInfo.skinIndex;

	firstMap = new GameSession(&mp);
	firstMap->Load();

	rushScoreDisplay->SetSession(firstMap);

	//GameSession *lastMap = firstMap;
	int numMapsInWorld = rushFile.worlds[w].maps.size();
	bonusVec.resize(numMapsInWorld - 1);
	for (int i = 0; i < numMapsInWorld - 1; ++i)
	{
		bonusVec[i] = firstMap->CreateBonus(rushFile.worlds[w].maps[i + 1].GetFilePath());
		//lastMap = bonusVec[i];
	}
}

void RushManager::LoadRush(const std::string &rushName)
{
	rushFile.Load("Resources/Rush", rushName);
	SetWorld(0);
}

void RushManager::LoadShip()
{
	if (shipGame != NULL)
	{
		delete shipGame;
		shipGame = NULL;
	}

	MatchParams mp;
	mp.mapPath = string("Resources/Maps/test/ashiptest") + MAP_EXT;
	mp.randSeed = time(0);
	mp.numPlayers = 1;
	mp.gameModeType = MatchParams::GAME_MODE_BASIC;

	mp.controllerStateVec[0] = controllerInput;
	mp.controlProfiles[0] = currProfile;
	mp.playerSkins[0] = 0;//adventureManager->currSaveFile->visualInfo.skinIndex;

	shipGame = new GameSession(&mp);
	shipGame->Load();
	//shipGame = new GameSession()
}

void RushManager::UpdateWorldDependentTileset(int worldIndex)
{
	if (currWorldDependentTilesetWorldIndex == worldIndex)
		return;

	currWorldDependentTilesetWorldIndex = worldIndex;
	if (ts_key != NULL)
	{
		DestroyTileset(ts_key);
		ts_key = NULL;

		DestroyTileset(ts_keyExplode);
		ts_keyExplode = NULL;
	}

	int w = worldIndex + 1;

	if (worldIndex < 8)
	{
		stringstream ss;
		ss << "Enemies/General/Keys/key_w" << w << "_128x128.png";
		stringstream ssExplode;
		ssExplode << "Enemies/General/Keys/keyexplode_w" << w << "_128x128.png";
		ts_key = GetSizedTileset(ss.str());
		ts_keyExplode = GetSizedTileset(ssExplode.str());
	}
	else
	{
		ts_key = GetSizedTileset("Enemies/General/Keys/key_w1_128x128.png");
		ts_keyExplode = GetSizedTileset("Enemies/General/Keys/keyexplode_w1_128x128.png");
	}

	if (ts_goal != NULL)
	{
		DestroyTileset(ts_goal);
		ts_goal = NULL;
	}

	if (ts_goalCrack != NULL)
	{
		DestroyTileset(ts_goalCrack);
		ts_goalCrack = NULL;
	}

	if (ts_goalExplode != NULL)
	{
		DestroyTileset(ts_goalExplode);
		ts_goalExplode = NULL;
	}


	//if (worldIndex < 1)
	{
		stringstream ss;
		ss << "Enemies/General/Goal/goal_w" << w << "_a_512x512.png";
		stringstream ssCrack;
		ssCrack << "Enemies/General/Goal/goal_w" << w << "_b_512x512.png";
		stringstream ssExplode;
		ssExplode << "Enemies/General/Goal/goal_w" << w << "_c_512x512.png";
		ts_goal = GetSizedTileset(ss.str());
		ts_goalCrack = GetSizedTileset(ssCrack.str());
		ts_goalExplode = GetSizedTileset(ssExplode.str());
	}
}

bool RushManager::TryToGoToNextLevel(GameSession *game)
{
	if (CanGoToNextLevel())
	{
		//int r = rand() % (rushFile.numMaps - 1);
		//game->SetBonus(bonusVec[r], V2d(0, 0));
		game->SetBonus(bonusVec[currRushMapIndex], V2d(0,0));
		currRushMapIndex++;
		return true;
	}

	return false;
}

bool RushManager::CanGoToNextLevel()
{
	if (currRushMapIndex < rushFile.worlds[currWorld].maps.size() - 1)
		return true;

	return false;
}

bool RushManager::TryToGoToNextWorld()
{
	if (currWorld < rushFile.numWorlds)
	{
		MainMenu::GetInstance()->SetModeRushLoadingWorld(currWorld + 1);
		currRushMapIndex = 0;
		currWorld++;
		return true;
	}
	else
	{
		return false;
	}
}

bool RushManager::TryToGoToNextWorldShip()
{
	if (currWorld < rushFile.numWorlds)
	{
		MainMenu::GetInstance()->SetModeRushShip(currWorld + 1);
		//currRushMapIndex = 0;
		//currWorld++;
		return true;
	}
	else
	{
		return false;
	}
}

bool RushManager::IsLastLevel()
{
	return false;
}

void RushManager::CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze)
{
}


void RushManager::CreateSaveMenu()
{
	assert(saveMenu == NULL);

	saveMenu = new SaveMenuScreen;
}

void RushManager::DestroySaveMenu()
{
	assert(saveMenu != NULL);

	delete saveMenu;

	saveMenu = NULL;
}

void RushManager::SaveCurrFile()
{
	//currSaveFile->Save();
}

void RushManager::StartDefaultSaveFile(int index)
{
	/*int savedSkin = files[index]->visualInfo.skinIndex;
	files[index]->SetAsDefault();
	files[index]->visualInfo.skinIndex = savedSkin;
	files[index]->Save();*/
}

void RushManager::SetCurrSaveFile(int index)
{
	/*currSaveFileIndex = index;
	currSaveFile = files[currSaveFileIndex];*/
}

void RushManager::FadeInSaveMenu()
{
	/*MainMenu *mainMenu = MainMenu::GetInstance();
	mainMenu->SetMode(MainMenu::SAVEMENU);
	saveMenu->Reset();
	saveMenu->action = SaveMenuScreen::FADEIN;
	saveMenu->transparency = 1.f;*/

	//saveMenu->SetSelectedIndex(mainMenu->RushManager->currSaveFileIndex);
}

void RushManager::UnlockUpgrade(int index)
{
	kinOptionField.SetBit(index, true);
	kinUpgradesInOrder.push_back(index);
}
