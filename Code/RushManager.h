#ifndef __RUSH_MANAGER_H__
#define __RUSH_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"
#include "Tileset.h"
#include "RushFile.h"
#include <vector>
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

struct RushScoreDisplay;
struct RushFile;

struct RushManager : TilesetManager
{
	RushFile rushFile;
	GameSession *firstMap;

	GameSession *shipGame;

	int currWorld;
	int currRushMapIndex;
	std::vector<GameSession*> bonusVec;

	int currWorldDependentTilesetWorldIndex;

	BitField kinOptionField; //store powers, store other variables. upgrades are not stored here. They are availabled based on your level/exp
	std::vector<int> kinUpgradesInOrder;

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
	void LoadShip();
	void SetWorld(int w);
	void UpdateWorldDependentTileset(int worldIndex);
	void LoadRush(const std::string &rushName);
	void CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze);
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
	void UnlockUpgrade(int index);

};

#endif