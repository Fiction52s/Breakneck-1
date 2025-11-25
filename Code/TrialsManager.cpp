#include "TrialsManager.h"
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
#include "KinUpgrades.h"

#include "RushFile.h"
#include "MedalTimeForm.h"
#include "KinStore.h"
#include "RushSaveFile.h"
#include "TrialsScreen.h"

using namespace std;
using namespace sf;

TrialsManager::TrialsManager()
{
	srand(time(0));

	pauseMenu = new PauseMenu(this);
	background = NULL;
	controllerInput = NULL;
	currProfile = NULL;

	MainMenu * mm = MainMenu::GetInstance();

	mm->trialsManager = this;

	kinStore = new KinStore;

	kinUpgradeLevels = new UpgradeLevels;

	adventureHUD = new AdventureHUD(this);

	medalTimeForm = new MedalTimeForm;

	leaderboard = new LeaderboardDisplay;

	trialsScreen = NULL;
	CreateTrialsScreen();
	//world map needs to load in with the trialsmanager/trials screen
	//We should just unload the trials menu when we go into the map, and load it again when we leave.
	//I don't think we need to actually unload the world map when we go into the individual screen for the world,
	//so in that casex lets just keep the world map loaded along w/ the trialsmanager

	//okay, but we should NOT unload the trialsmanager when we go into the level because it needs to hold all of our manager related info
	//so the play would just be unload the world map and other resources each time

	currWorldDependentTilesetWorldIndex = -1;
	ts_key = NULL;
	ts_goal = NULL;
	ts_goalCrack = NULL;
	ts_goalExplode = NULL;
}

TrialsManager::~TrialsManager()
{
	delete kinUpgradeLevels;

	if (adventureHUD != NULL)
	{
		delete adventureHUD;
	}

	if (pauseMenu != NULL)
	{
		delete pauseMenu;
		pauseMenu = NULL;
	}

	delete medalTimeForm;

	delete kinStore;

	delete leaderboard;

	if (trialsScreen != NULL)
	{
		delete trialsScreen;
	}
}

void TrialsManager::Load()
{
	if (MainMenu::GetInstance()->isDemoModeOn)
	{
		LoadRushFile("demo");
	}
	else
	{
		LoadRushFile("test");
	}
}

void TrialsManager::SetToLevel(int w, int mapIndex )
{
	UpdateWorldDependentTileset(w);

	MainMenu::GetInstance()->gameRunType = MainMenu::GRT_TRIALS;

	currWorld = w;

	//int powerWorlds = min(currWorld, 6);
	//for (int i = 0; i < powerWorlds; ++i)
	//{
	//	if (kinUpgradeLevels->GetUpgradeLevel(POWER_AIR_DASH + i) == 0)
	//	{
	//		kinUpgradeLevels->SetUpgradeLevel(POWER_AIR_DASH + i, 1);
	//	}
	//}

	MatchParams mp;
	mp.mapPath = rushFile.worlds[w].maps[0].GetMapPath();
	mp.randSeed = time(0);
	mp.numPlayers = 1;
	mp.gameModeType = MatchParams::GAME_MODE_BASIC;

	mp.controllerStateVec[0] = controllerInput;
	mp.controlProfiles[0] = currProfile;
	mp.playerSkins[0] = 0;//adventureManager->currSaveFile->visualInfo.skinIndex;

	currLevel = new GameSession(&mp);
	currLevel->Load();
	//rushScoreDisplay->SetSession(firstMap);
}

void TrialsManager::LoadCurrentLevel()
{
	SetToLevel(0, 0);
}

void TrialsManager::LoadRushFile(const std::string &rushName)
{
	rushFile.Load("Resources/Rush", rushName);
}

void TrialsManager::UpdateWorldDependentTileset(int worldIndex)
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


void TrialsManager::UnlockUpgrade(int up, int lvl)
{
	kinUpgradeLevels->SetUpgradeLevel(up, lvl);
	//kinUpgradesInOrder.push_back(index);
}

std::string TrialsManager::GetLeaderboardNameAnyPowers(GameSession *game)
{
	return rushFile.GetLeaderboardName(0, 0) + "_" + game->myHash;//game->level->index) + "_" + game->myHash;
}

std::string TrialsManager::GetLeaderboardNameOriginalPowers(GameSession *game)
{
	return rushFile.GetLeaderboardName(0, 0) + "_orig_" + game->myHash;
}

std::string TrialsManager::GetLeaderboardDisplayName(GameSession *game)
{
	return rushFile.GetLeaderboardName(0, 0);//GetLeaderboardDisplayName(0, 0);//game->level->index);
}

std::string TrialsManager::GetLeaderboardNameAnyPowers(int levelIndex, const std::string &myHash)
{
	return rushFile.GetLeaderboardName(0, levelIndex) + "_" + myHash;
}

std::string TrialsManager::GetLeaderboardNameOriginalPowers(int levelIndex, const std::string &myHash)
{
	return rushFile.GetLeaderboardName(0, levelIndex) + "_orig_" + myHash;
}

std::string TrialsManager::GetLeaderboardDisplayName(int levelIndex)
{
	return rushFile.GetLeaderboardName(0, levelIndex);
}

void TrialsManager::SetBoards(GameSession *game)
{
	assert(leaderboard != NULL);

	/*leaderboard->SetBoards(GetLeaderboardDisplayName(game), GetLeaderboardNameOriginalPowers(game),
		GetLeaderboardNameAnyPowers(game));*/
}

void TrialsManager::SetBoards(int levelIndex, const std::string &myHash)
{
	assert(leaderboard != NULL);

	leaderboard->SetBoards(GetLeaderboardDisplayName(levelIndex));
}

void TrialsManager::CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze)
{
	setRecord = false;
	gotGold = false;
	gotSilver = false;
	gotBronze = false;

	Level *lev = game->level;
	int totalFrames = game->totalFramesBeforeGoal;

	//if (currSaveFile == NULL)
	//{
	//	assert(0);
	//	return;
	//}

	//if (!currSaveFile->IsCompleteLevel(lev))
	//{
	//	currSaveFile->CompleteLevel(lev);
	//}
	//else
	//{
	//	//lev.justBeaten = false;
	//}

	if (!game->usedWarp)
	{
		setRecord = false;//currSaveFile->TrySetRecordTime(totalFrames, lev);

		gotGold = false;//currSaveFile->TryUnlockGoldMedal(totalFrames, lev);
		if (gotGold)
		{
			cout << "unlocked the gold medal for this level!" << endl;
		}

		gotSilver = false;//currSaveFile->TryUnlockSilverMedal(totalFrames, lev);
		if (gotSilver)
		{
			cout << "unlocked the silver medal for this level!" << endl;
		}

		gotBronze = false;//currSaveFile->TryUnlockBronzeMedal(totalFrames, lev);
		if (gotBronze)
		{
			cout << "unlocked the bronze medal for this level!" << endl;
		}

		string tempReplayPath = string("Resources/temp_replay") + REPLAY_EXT;

		if (game->playerRecordingManager != NULL)
		{
			game->playerRecordingManager->RecordReplayFrames();
			game->playerRecordingManager->RecordGhostFrames(); //added this rn

			game->playerRecordingManager->StopRecording();
			game->playerRecordingManager->WriteToFile(tempReplayPath);

			if (setRecord)
			{
				string bestReplayPath = game->GetBestReplayPath();
				boost::filesystem::copy_file(tempReplayPath, bestReplayPath, boost::filesystem::copy_option::overwrite_if_exists);
			}

			if (MainMenu::GetInstance()->steamOn)
			{
				leaderboard->UploadScore(totalFrames, tempReplayPath);
			}
			//leaderboardMan->UploadScore(totalFrames);
			//leaderboard stuff here!
		}
	}

	Actor *a = game->GetPlayer(0);

	//currSaveFile->currency += a->currencyCounter;

	//SaveCurrFile();
}

void TrialsManager::CreateTrialsScreen()
{
	assert(trialsScreen == NULL);
	trialsScreen = new TrialsScreen;
	trialsScreen->trialsMan = this;
	cout << "create trialsscreen" << endl;
}

void TrialsManager::DestroyTrialsScreen()
{
	assert(trialsScreen != NULL);
	delete trialsScreen;
	trialsScreen = NULL;
	cout << "destroy trialsscreen" << endl;
}

void TrialsManager::UpdateButtonIconsWhenControllerIsChanged()
{
	assert(trialsScreen != NULL);
	trialsScreen->worldMap->UpdateButtonIconsWhenControllerIsChanged();
}