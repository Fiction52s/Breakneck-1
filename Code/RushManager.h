#ifndef __RUSH_MANAGER_H__
#define __RUSH_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"
#include "Tileset.h"
#include "RushFile.h"
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


struct RushFile;

struct RushManager : TilesetManager
{
	RushFile rushFile;
	GameSession *firstMap;

	int currRushMapIndex;
	std::vector<GameSession*> bonusVec;


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
	//SaveFile *files[6];
	//SaveFile *currSaveFile;
	int currSaveFileIndex;
	AdventureHUD *adventureHUD;

	Background *background;

	ControllerDualStateQueue *controllerInput;
	ControlProfile *currProfile;

	bool isDefaultKeyboardInputOn;

	GameSession *currLevel;

	RushManager();
	~RushManager();
	void Load();
	void UpdateWorldDependentTileset(int worldIndex);
	void LoadRush(const std::string &rushName);
	void CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze);
	void CreateSaveMenu();
	void DestroySaveMenu();
	bool TryToGoToNextLevel(GameSession *game);
	bool IsLastLevel();
	void StartDefaultSaveFile(int index);
	void SaveCurrFile();
	void SetCurrSaveFile(int index);
	void FadeInSaveMenu();

};

#endif