#include "AdventureManager.h"
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

using namespace std;
using namespace sf;

AdventureManager::AdventureManager()
{
	leaderboard = new LeaderboardDisplay;
	feedbackForm = new FeedbackForm;
	pauseMenu = new PauseMenu(this);
	worldMap = NULL;
	background = NULL;
	kinBoostScreen = NULL;
	worldTransferScreen = NULL;
	saveMenu = NULL;
	adventurePlanet = NULL;
	controllerInput = NULL;
	currProfile = NULL;

	transferPlayerPowerMode = -1;

	

	adventureHUD = new AdventureHUD(this);

	currWorldDependentTilesetWorldIndex = -1;
	ts_key = NULL;
	ts_goal = NULL;
	ts_goalCrack = NULL;
	ts_goalExplode = NULL;

	if (MainMenu::GetInstance()->steamOn)
	{
		parallelPracticeMode = true; //for testing
	}
	else
	{
		parallelPracticeMode = false;
	}

	originalProgressionMode = false;

	LoadAdventure("tadventure");

	worldTransferScreen = new WorldTransferScreen;

	kinBoostScreen = new KinBoostScreen;

	
	//worldTransferScreen = new WorldTransferScreen;

	std::vector<string> saveNames = { "blue", "green", "yellow", "orange", "red", "magenta" };
	for (int i = 0; i < 6; ++i)
	{
		files[i] = new SaveFile(saveNames[i], &adventureFile);
	}

	

	//CreateSaveMenu();

	SetCurrSaveFile(0);
}

AdventureManager::~AdventureManager()
{
	if (adventureHUD != NULL)
	{
		delete adventureHUD;
	}

	if (pauseMenu != NULL)
	{
		delete pauseMenu;
		pauseMenu = NULL;
	}

	if (leaderboard != NULL)
	{
		delete leaderboard;
		leaderboard = NULL;
	}

	if (feedbackForm != NULL)
	{
		delete feedbackForm;
		feedbackForm = NULL;
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

	if (adventurePlanet != NULL)
	{
		delete adventurePlanet;
	}

	for (int i = 0; i < 6; ++i)
	{
		delete files[i];
	}
}

void AdventureManager::LoadAdventure(const std::string &adventureName)
{
	adventureFile.Load("Resources/Adventure", adventureName);
	adventureFile.LoadMapHeaders();
	adventurePlanet = new AdventurePlanet(adventureFile);
}

void AdventureManager::UpdateWorldDependentTileset(int worldIndex)
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

bool AdventureManager::CanBoostToNextLevel()
{
	int w = 0;
	int s = 0;
	int m = 0;
	adventureFile.GetMapIndexes(currLevel->level->index, w, s, m);

	World &world = adventurePlanet->worlds[w];
	Sector &sector = world.sectors[s];
	//sector.numLevels;

	AdventureWorld &adventureWorld = adventureFile.GetWorld(w);
	AdventureSector &adventureSector = adventureFile.GetSector(w, s);

	bool keepGoingAfterSectorEnd = true;

	if (m < adventureSector.GetNumExistingMaps() - 1)
	{
		return true;
	}
	else
	{
		//this is in case I have this as an option at some point
		if (keepGoingAfterSectorEnd)
		{
			if (s < adventureWorld.GetNumExistingSectors() - 1)
			{
				return true;
			}
			else
			{
				if (w < adventureFile.GetNumExistingWorlds() - 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

bool AdventureManager::TryToGoToNextLevel()
{
	int w = 0;
	int s = 0;
	int m = 0;
	adventureFile.GetMapIndexes(currLevel->level->index, w, s, m);

	int origW = w;
	int origS = s;
	int origM = m;

	World &world = adventurePlanet->worlds[w];
	Sector &sector = world.sectors[s];
	//sector.numLevels;
	
	AdventureWorld &adventureWorld = adventureFile.GetWorld(w);
	AdventureSector &adventureSector = adventureFile.GetSector(w, s);

	bool keepGoingAfterSectorEnd = true;

	if (m < adventureSector.GetNumExistingMaps() - 1)
	{
		++m;
	}
	else
	{
		//this is in case I have this as an option at some point
		if (keepGoingAfterSectorEnd)
		{
			if (s < adventureWorld.GetNumExistingSectors() - 1)
			{
				++s;
				m = 0;
			}
			else
			{
				if (w < adventureFile.GetNumExistingWorlds() - 1)
				{
					++w;
					m = 0;
					s = 0;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	Level *lev = adventurePlanet->worlds[w].sectors[s].GetLevel(m);


	transferPlayerPowerMode = currLevel->GetPlayer(0)->currPowerMode;

	if (w == origW)
	{
		kinBoostScreen->SetLevel(lev);
		MainMenu::GetInstance()->SetModeKinBoostLoadingMap(0);
	}
	else
	{
		worldTransferScreen->SetLevel(lev);
		MainMenu::GetInstance()->SetModeWorldTransferLoadingMap(w);
	}
	

	return true;
}

bool AdventureManager::IsLastLevel()
{
	int w = 0;
	int s = 0;
	int m = 0;
	adventureFile.GetMapIndexes(currLevel->level->index, w, s, m);

	int origW = w;
	int origS = s;
	int origM = m;

	World &world = adventurePlanet->worlds[w];
	Sector &sector = world.sectors[s];

	AdventureWorld &adventureWorld = adventureFile.GetWorld(w);
	AdventureSector &adventureSector = adventureFile.GetSector(w, s);

	bool keepGoingAfterSectorEnd = true;

	if (m == adventureSector.GetNumExistingMaps() - 1
		&& s == adventureWorld.GetNumExistingSectors() - 1
		&& w == adventureFile.GetNumExistingWorlds() - 1)
	{
		return true;
	}
	
	return false;
}

void AdventureManager::CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze)
{
	setRecord = false;
	gotGold = false;
	gotSilver = false;
	gotBronze = false;

	Level *lev = game->level;
	int totalFrames = game->totalFramesBeforeGoal;

	if (currSaveFile == NULL)
	{
		assert(0);
		return;
	}

	if (!currSaveFile->IsCompleteLevel(lev))
	{
		currSaveFile->CompleteLevel(lev);
	}
	else
	{
		//lev.justBeaten = false;
	}

	if (!game->usedWarp)
	{
		setRecord = currSaveFile->TrySetRecordTime(totalFrames, lev);

		gotGold = currSaveFile->TryUnlockGoldMedal(totalFrames, lev);
		if (gotGold)
		{
			cout << "unlocked the gold medal for this level!" << endl;
			if (game->mapHeader->goldRewardShardInfo.world >= 0)
			{
				int upgradeIndex = game->mapHeader->goldRewardShardInfo.GetTrueIndex() + Actor::SHARD_START_INDEX;
				currSaveFile->UnlockUpgrade();
				cout << "unlocked upgrade: "
			}
			
			
		}

		gotSilver = currSaveFile->TryUnlockSilverMedal(totalFrames, lev);
		if (gotSilver)
		{
			cout << "unlocked the silver medal for this level!" << endl;
		}

		gotBronze = currSaveFile->TryUnlockBronzeMedal(totalFrames, lev);
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
				leaderboard->UploadScore(totalFrames, tempReplayPath, game->originalProgressionCompatible);
			}
			//leaderboardMan->UploadScore(totalFrames);
			//leaderboard stuff here!
		}
	}
	

	SaveCurrFile();
}

void AdventureManager::CreateWorldMap()
{
	assert(worldMap == NULL);
	worldMap = new WorldMap;
}

void AdventureManager::CreateWorldMapOnLevel(int w, int s, int m)
{
	assert(worldMap == NULL);
	worldMap = new WorldMap;

	worldMap->InitSelectors();
	//worldMap->SetDefaultSelections();

	worldMap->SetToLevel(w, s, m);

	

	if (worldMap->CurrSelector()->numSectors == 1)
	{
		worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();
	}
	//worldMap->selectors[worldMap->selectedColony]->ReturnFromMap();
	//SetMode(WORLDMAP_COLONY);
	//worldMap->CurrSelector()->FocusedSector()->UpdateLevelStats();
	//worldMap->CurrSelector()->FocusedSector()->UpdateStats();
	//worldMap->CurrSelector()->FocusedSector()->UpdateMapPreview();
	//worldMap->Update();

	//musicPlayer->TransitionMusic(menuMusic, 60);
}

void AdventureManager::CreateWorldMapOnCurrLevel()
{
	int w = 0;
	int s = 0;
	int m = 0;
	adventureFile.GetMapIndexes(currLevel->level->index, w, s, m);

	CreateWorldMapOnLevel(w, s, m);
}

void AdventureManager::DestroyWorldMap()
{
	assert(worldMap != NULL);
	
	delete worldMap;

	worldMap = NULL;
}

void AdventureManager::CreateSaveMenu()
{
	assert(saveMenu == NULL);

	saveMenu = new SaveMenuScreen;
}

void AdventureManager::DestroySaveMenu()
{
	assert(saveMenu != NULL);

	delete saveMenu;

	saveMenu = NULL;
}

void AdventureManager::SaveCurrFile()
{
	currSaveFile->Save();
}

void AdventureManager::StartDefaultSaveFile( int index )
{
	int savedSkin = files[index]->visualInfo.skinIndex;
	files[index]->SetAsDefault();
	files[index]->visualInfo.skinIndex = savedSkin;
	files[index]->Save();
}

void AdventureManager::SetCurrSaveFile(int index)
{
	currSaveFileIndex = index;
	currSaveFile = files[currSaveFileIndex];
}

void AdventureManager::FadeInSaveMenu()
{
	MainMenu *mainMenu = MainMenu::GetInstance();
	mainMenu->SetMode(MainMenu::SAVEMENU);
	saveMenu->Reset();
	saveMenu->action = SaveMenuScreen::FADEIN;
	saveMenu->transparency = 1.f;

	saveMenu->SetSelectedIndex(mainMenu->adventureManager->currSaveFileIndex);
}

std::string AdventureManager::GetLeaderboardNameAnyPowers(GameSession *game)
{
	return adventureFile.GetLeaderboardName(game->level->index) + "_" + game->myHash;
}

std::string AdventureManager::GetLeaderboardNameOriginalPowers(GameSession *game)
{
	return adventureFile.GetLeaderboardName(game->level->index) + "_orig_" + game->myHash;
}

std::string AdventureManager::GetLeaderboardDisplayName(GameSession *game)
{
	return adventureFile.GetLeaderboardDisplayName(game->level->index);
}

std::string AdventureManager::GetLeaderboardNameAnyPowers(int levelIndex, const std::string &myHash)
{
	return adventureFile.GetLeaderboardName(levelIndex) + "_" + myHash;
}

std::string AdventureManager::GetLeaderboardNameOriginalPowers(int levelIndex, const std::string &myHash)
{
	return adventureFile.GetLeaderboardName(levelIndex) + "_orig_" + myHash;
}

std::string AdventureManager::GetLeaderboardDisplayName(int levelIndex)
{
	return adventureFile.GetLeaderboardDisplayName(levelIndex);
}

void AdventureManager::DrawWorldMap(sf::RenderTarget *target)
{
	worldMap->Draw(target);
}

void AdventureManager::SetBoards(GameSession *game)
{
	assert(leaderboard != NULL);

	leaderboard->SetBoards( GetLeaderboardDisplayName( game ), GetLeaderboardNameOriginalPowers( game ), 
		GetLeaderboardNameAnyPowers( game ));
}

void AdventureManager::SetBoards(int levelIndex, const std::string &myHash )
{
	assert(leaderboard != NULL);

	leaderboard->SetBoards(GetLeaderboardDisplayName(levelIndex), GetLeaderboardNameOriginalPowers(levelIndex, myHash),
		GetLeaderboardNameAnyPowers(levelIndex, myHash));
}

