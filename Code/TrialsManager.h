#ifndef __TRIALS_MANAGER_H__
#define __TRIALS_MANAGER_H__

#include <SFML/Graphics.hpp>
#include "SaveFile.h"
#include "Input.h"
#include "Tileset.h"
#include "RushFile.h"
#include <vector>
#include "KinUpgrades.h"

struct UpgradeLevels;
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
struct TrialsScreen;

struct TrialsManager : TilesetManager
{
	TrialsScreen *trialsScreen;

	LeaderboardDisplay *leaderboard;

	RushFile rushFile; //use to load th maps until we think of something new

	int currWorldDependentTilesetWorldIndex;

	UpgradeLevels *kinUpgradeLevels;

	Tileset *ts_key;
	Tileset *ts_keyExplode;
	Tileset *ts_goal;
	Tileset *ts_goalCrack;
	Tileset *ts_goalExplode;

	//int transferPlayerPowerMode;
	PauseMenu *pauseMenu;

	AdventureHUD *adventureHUD;

	Background *background;

	XINPUT_VIBRATION vibration;
	ControllerDualStateQueue *controllerInput;
	ControlProfile *currProfile;

	bool isDefaultKeyboardInputOn;

	GameSession *currLevel;

	MedalTimeForm *medalTimeForm;

	KinStore *kinStore;

	int currWorld;

	bool parallelPracticeMode;
	

	TrialsManager();
	~TrialsManager();
	void Load();
	void SetToLevel(int w, int levelIndex);
	void LoadCurrentLevel();
	void UpdateWorldDependentTileset(int worldIndex);
	void LoadRushFile(const std::string &rushName);
	void UnlockUpgrade(int up, int lvl);
	void CreateTrialsScreen();
	void DestroyTrialsScreen();

	std::string GetLeaderboardNameAnyPowers(GameSession *game);
	std::string GetLeaderboardNameOriginalPowers(GameSession *game);
	std::string GetLeaderboardDisplayName(GameSession *game);
	std::string GetLeaderboardNameAnyPowers(int levelIndex, const std::string &myHash);
	std::string GetLeaderboardNameOriginalPowers(int levelIndex, const std::string &myHash);
	std::string GetLeaderboardDisplayName(int levelIndex);
	void CompleteCurrentMap(GameSession *game, bool &setRecord, bool &gotGold, bool &gotSilver, bool &gotBronze);

	void UpdateButtonIconsWhenControllerIsChanged();

	void SetBoards(GameSession *game);
	void SetBoards(int levelIndex, const std::string &myHash);
};

#endif