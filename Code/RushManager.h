#ifndef __RUSH_MANAGER_H__
#define __RUSH_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"
#include "Tileset.h"
#include "RushFile.h"
#include <vector>

struct UpgradeLevels;
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
struct MedalTimeForm;
struct KinStore;
struct RushScoreDisplay;
struct RushFile;
struct KinExperienceBar;

struct RushManager : TilesetManager
{
	RushFile rushFile;
	GameSession *firstMap;

	GameSession *shipGame;

	int storePoints;

	int currWorld;
	int currRushMapIndex;
	std::vector<GameSession*> bonusVec;

	int currWorldDependentTilesetWorldIndex;

	UpgradeLevels *kinUpgradeLevels;

	std::vector<int> kinUpgradesInOrder;

	Tileset *ts_key;
	Tileset *ts_keyExplode;
	Tileset *ts_goal;
	Tileset *ts_goalCrack;
	Tileset *ts_goalExplode;

	int transferPlayerPowerMode;
	int transferPlayerHotkeyedPowerMode;
	PauseMenu *pauseMenu;
	WorldMap *worldMap;
	KinBoostScreen *kinBoostScreen;
	WorldTransferScreen *worldTransferScreen;
	SaveMenuScreen *saveMenu;
	RushScoreDisplay *rushScoreDisplay;
	//SaveFile *files[6];
	//SaveFile *currSaveFile;
	int currSaveFileIndex;
	AdventureHUD *adventureHUD;

	Background *background;

	XINPUT_VIBRATION vibration;
	ControllerDualStateQueue *controllerInput;
	ControlProfile *currProfile;

	int startWorld;

	bool isDefaultKeyboardInputOn;

	GameSession *currLevel;

	MedalTimeForm *medalTimeForm;

	KinStore *kinStore;

	int trueLevelIndex; //for randomized build

	KinExperienceBar *expBar;

	RushManager();
	~RushManager();
	void Load();
	void LoadShip();
	void SetWorld(int w);
	void UpdateWorldDependentTileset(int worldIndex);
	void LoadRush(const std::string &rushName);
	//void CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze);
	void CreateSaveMenu();
	void DestroySaveMenu();
	bool TryToGoToNextLevel(GameSession *game);
	bool CanGoToNextLevel();
	bool TryToGoToNextWorld();
	bool TryToGoToNextWorldShip();
	bool IsLastLevel();
	void StartDefaultSaveFile(int index);
	void SaveCurrFile();
	void SetCurrSaveFile(int index);
	void FadeInSaveMenu();
	void UnlockUpgrade(int up, int lvl);

};

#endif