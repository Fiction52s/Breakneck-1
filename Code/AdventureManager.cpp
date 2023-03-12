#include "AdventureManager.h"
#include "KinBoostScreen.h"
#include "SaveMenuScreen.h"
#include "WorldMap.h"
#include "MainMenu.h"
#include "GameSession.h"
#include "PlayerRecord.h"
#include "Leaderboard.h"
#include "md5.h"

using namespace std;
using namespace sf;

AdventureManager::AdventureManager()
{
	leaderboard = new LeaderboardDisplay;
	worldMap = NULL;
	kinBoostScreen = NULL;
	saveMenu = NULL;
	adventurePlanet = NULL;
	controllerInput = NULL;
	currProfile = NULL;
	parallelPracticeMode = false;

	LoadAdventure("tadventure");

	kinBoostScreen = new KinBoostScreen;

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
	if (leaderboard != NULL)
	{
		delete leaderboard;
		leaderboard = NULL;
	}

	if (worldMap != NULL)
	{
		delete worldMap;
	}

	if (kinBoostScreen != NULL)
	{
		delete kinBoostScreen;
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

bool AdventureManager::TryToGoToNextLevel()
{
	int w = 0;
	int s = 0;
	int m = 0;
	adventureFile.GetMapIndexes(currLevel->level->index, w, s, m);

	adventurePlanet->worlds[w].sectors[s].numLevels;

	auto &sector = adventureFile.GetSector(w, s);

	if (m < sector.GetNumExistingMaps() - 1)
	{
		++m;

		Level *lev = adventurePlanet->worlds[w].sectors[s].GetLevel(m);
		kinBoostScreen->level = lev;
		MainMenu::GetInstance()->SetModeKinBoostLoadingMap(0);

		return true;
	}
	else
	{
		return false;

		//MainMenu::GetInstance()->fader->Clear();

		//ReturnToWorldAfterLevel();
	}
}

//returns true if you make a record!
bool AdventureManager::CompleteCurrentMap(GameSession *game)
{
	Level *lev = game->level;
	int totalFrames = game->totalFramesBeforeGoal;

	if (currSaveFile == NULL)
	{
		assert(0);
		return false;
	}

	if (!currSaveFile->IsCompleteLevel(lev))
	{
		currSaveFile->CompleteLevel(lev);
	}
	else
	{
		//lev.justBeaten = false;
	}

	bool isRecordSet = currSaveFile->TrySetRecordTime(totalFrames, lev);
	if (isRecordSet)
	{
		//if (game->recPlayer != NULL)
		//{
		//	game->recPlayer->RecordFrame();
		//	game->recPlayer->StopRecording();
		//	game->recPlayer->WriteToFile(game->GetBestReplayPath());
		//}

		//if (game->recGhost != NULL)
		//{
		//	game->recGhost->StopRecording();
		//	game->recGhost->WriteToFile(game->GetBestTimeGhostPath());
		//	game->SetupBestTimeGhost(); //only if ghost is already on
		//}

		if (game->playerRecordingManager != NULL)
		{
			game->playerRecordingManager->RecordReplayFrames();
			game->playerRecordingManager->RecordGhostFrames(); //added this rn

			string bestReplayPath = game->GetBestReplayPath();

			game->playerRecordingManager->StopRecording();
			game->playerRecordingManager->WriteToFile(bestReplayPath);

			//just turned this off
			//game->SetupPlayerReplayerManagers();

			//string currPath = boost::filesystem::current_path().string();
			//string fullReplayPath = currPath + "\\" + bestReplayPath;
			
			/*string leaderBoardStr;
			if (leaderboard->IsAnyPowersMode())
			{
				leaderBoardStr = GetLeaderboardNameAnyPowers(game);
			}
			else
			{
				leaderBoardStr = GetLeaderboardNameOriginalPowers(game);
			}*/

			//leaderboard->manager.UploadScore(leaderBoardStr,
			//	totalFrames, bestReplayPath);

			leaderboard->UploadScore(totalFrames, bestReplayPath, game->originalProgressionCompatible);




			//leaderboardMan->UploadScore(totalFrames);
			//leaderboard stuff here!
		}
		//create a flag so that you can get hype over this
	}

	SaveCurrFile();

	return isRecordSet;
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
	int savedSkin = files[index]->defaultSkinIndex;
	files[index]->SetAsDefault();
	files[index]->defaultSkinIndex = savedSkin;
	files[index]->Save();
}

void AdventureManager::SetCurrSaveFile(int index)
{
	currSaveFileIndex = index;
	currSaveFile = files[currSaveFileIndex];
}

void AdventureManager::FadeInSaveMenu()
{
	MainMenu::GetInstance()->SetMode(MainMenu::SAVEMENU);
	saveMenu->Reset();
	saveMenu->action = SaveMenuScreen::FADEIN;
	saveMenu->transparency = 1.f;
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

