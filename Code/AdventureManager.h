#ifndef __ADVENTURE_MANAGER_H__
#define __ADVENTURE_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"

struct WorldMap;
struct SaveMenuScreen;
struct KinBoostScreen;
struct GameSession;
struct SaveFile;
struct ControlProfile;
struct LeaderboardDisplay;

struct AdventureManager
{
	WorldMap *worldMap;
	KinBoostScreen *kinBoostScreen;
	SaveMenuScreen *saveMenu;
	SaveFile *files[6];
	SaveFile *currSaveFile;
	int currSaveFileIndex;

	AdventureFile adventureFile;
	AdventurePlanet *adventurePlanet;

	ControllerDualStateQueue *controllerInput;
	ControlProfile *currProfile;

	bool isDefaultKeyboardInputOn;

	GameSession *currLevel;

	LeaderboardDisplay *leaderboard;

	AdventureManager();
	~AdventureManager();
	void LoadAdventure( const std::string &adventureName );
	bool CompleteCurrentMap(GameSession *game );
	void DrawWorldMap(sf::RenderTarget *target);
	void SetBoards(GameSession *game );
	void CreateWorldMap();
	void CreateWorldMapOnLevel(int w, int s, int m);
	void CreateWorldMapOnCurrLevel();
	void DestroyWorldMap();
	void CreateSaveMenu();
	void DestroySaveMenu();
	bool TryToGoToNextLevel();
	void StartDefaultSaveFile( int index );
	void SaveCurrFile();
	void SetCurrSaveFile(int index);
	void FadeInSaveMenu();

	std::string GetLeaderboardNameAnyPowers(GameSession *game);
	std::string GetLeaderboardNameOriginalPowers(GameSession *game);
	
};

#endif