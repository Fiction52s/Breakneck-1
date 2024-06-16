#ifndef __ADVENTURE_MANAGER_H__
#define __ADVENTURE_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"
#include "Tileset.h"


struct WorldMap;
struct SaveMenuScreen;
struct KinBoostScreen;
struct GameSession;
struct SaveFile;
struct ControlProfile;
struct LeaderboardDisplay;
struct FeedbackForm;
struct PauseMenu;
struct AdventureHUD;
struct Background;
struct WorldTransferScreen;
struct AdventureScoreDisplay;
struct KinExperienceBar;

struct AdventureManager : TilesetManager
{
	int currWorldDependentTilesetWorldIndex;

	Tileset *ts_key;
	Tileset *ts_keyExplode;
	Tileset *ts_goal;
	Tileset *ts_goalCrack;
	Tileset *ts_goalExplode;

	int transferPlayerPowerMode;
	PauseMenu *pauseMenu;
	WorldMap *worldMap;
	KinBoostScreen *kinBoostScreen;
	WorldTransferScreen *worldTransferScreen;
	SaveMenuScreen *saveMenu;
	SaveFile *files[6];
	SaveFile *currSaveFile;
	int currSaveFileIndex;
	AdventureHUD *adventureHUD;
	AdventureScoreDisplay *adventureScoreDisplay;

	Background *background;

	AdventureFile adventureFile;
	AdventurePlanet *adventurePlanet;

	ControllerDualStateQueue *controllerInput;
	ControlProfile *currProfile;

	bool isDefaultKeyboardInputOn;

	GameSession *currLevel;

	LeaderboardDisplay *leaderboard;
	
	FeedbackForm *feedbackForm;

	KinExperienceBar *expBar;

	bool parallelPracticeMode;
	bool originalProgressionMode;

	AdventureManager();
	~AdventureManager();
	void UpdateWorldDependentTileset(int worldIndex);
	void LoadAdventure( const std::string &adventureName );
	void CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze );
	void DrawWorldMap(sf::RenderTarget *target);
	void SetBoards(GameSession *game );
	void SetBoards(int levelIndex, const std::string &myHash );
	void CreateWorldMap();
	void CreateWorldMapOnLevel(int w, int s, int m);
	void CreateWorldMapOnCurrLevel();
	void DestroyWorldMap();
	void CreateSaveMenu();
	void DestroySaveMenu();
	bool TryToGoToNextLevel();
	bool IsLastLevel();
	bool CanBoostToNextLevel();
	void StartDefaultSaveFile( int index );
	void SaveCurrFile();
	void SetCurrSaveFile(int index);
	void FadeInSaveMenu();

	std::string GetLeaderboardNameAnyPowers(GameSession *game);
	std::string GetLeaderboardNameOriginalPowers(GameSession *game);
	std::string GetLeaderboardDisplayName(GameSession *game);

	std::string GetLeaderboardNameAnyPowers(int levelIndex, const std::string &myHash );
	std::string GetLeaderboardNameOriginalPowers(int levelIndex, const std::string &myHash);
	std::string GetLeaderboardDisplayName(int levelIndex);
	
};

#endif