#ifndef __RUSH_MANAGER_H__
#define __RUSH_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"
#include "Tileset.h"
#include "RushFile.h"
#include <vector>
#include "KinUpgrades.h"

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
struct RushSaveFile;



struct RushManager : TilesetManager
{
	const static int NUM_SAVE_FILES = 3;

	RushFile rushFile;
	GameSession *firstMap;

	GameSession *shipGame;

	int storePoints;

	int currWorldSection;
	int currWorld;
	int currRushMapIndex;
	std::vector<GameSession*> bonusVec;

	int currWorldDependentTilesetWorldIndex;

	UpgradeLevels *kinUpgradeLevels;

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
	RushScoreDisplay *rushScoreDisplay;
	//SaveFile *files[6];
	//SaveFile *currSaveFile;
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

	std::vector<RushSaveFile*> saveFileVec;
	int currSaveFileIndex;
	RushSaveFile *currSaveFile;

	RushManager();
	~RushManager();
	void Load();
	void LoadShip();
	void SetWorld(int w, int section);
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