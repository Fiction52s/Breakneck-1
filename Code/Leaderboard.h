#ifndef __LEADERBOARD_MANAGER_H__
#define __LEADERBOARD_MANAGER_H__

#include <SFML\Graphics.hpp>
#include "steam\steam_api.h"
#include <vector>
#include "GUI.h"
#include "RemoteStorageManager.h"
#include <vector>

struct PlayerReplayManager;
struct ReplayGhost;
struct LeaderboardDisplay;

struct KineticLeaderboardEntry
{
	enum Action
	{
		A_CLEAR,
		A_INITIALIZED,
		A_DOWNLOADING,
		A_SAVING,
		A_READY,
		A_Count
	};

	LeaderboardEntry_t steamEntry;
	std::string name;
	std::string timeStr;
	std::string replayPath;
	int action;

	bool ghostOn;

	PlayerReplayManager *playerReplayManager;

	KineticLeaderboardEntry();
	KineticLeaderboardEntry(const KineticLeaderboardEntry &);
	~KineticLeaderboardEntry();
	void DownloadReplay();
	bool IsReplayReady();
	void Init();
	void Clear();
private:
	CCallResult<KineticLeaderboardEntry,
		RemoteStorageDownloadUGCResult_t> onRemoteStorageDownloadUGCResultCallResult;

	void OnRemoteStorageDownloadUGCResult(RemoteStorageDownloadUGCResult_t *callback, bool bIOFailure);
};

struct LeaderboardInfo
{
	std::string name;
	SteamLeaderboard_t leaderboardID;
	std::vector<KineticLeaderboardEntry> entries;
	
	LeaderboardInfo();
	void Clear();
	void ClearEntries();
};

struct SteamLeaderboardManager : RemoteStorageResultHandler
{
	enum Action
	{
		A_IDLE,
		A_UPLOAD_SCORE_SUCCESS,
		A_UPLOAD_SCORE_FAILURE,
		A_FINDING,
		A_UPLOAD_REPLAY,
		A_SHARE_REPLAY,
		A_UPLOADING_SCORE,
		A_ATTACH_REPLAY,
		A_DOWNLOADING,
		A_DOWNLOADING_MY_SCORE,
	};

	int action;
	LeaderboardInfo *boardBeingProcessed;
	LeaderboardInfo currBoard;
	LeaderboardInfo uploadBoard;
	int scoreToUpload;
	std::string localReplayPath;
	std::string cloudReplayPath;
	std::string searchBoardName;
	int postFindAction;

	LeaderboardDisplay *display;

	int myEntryIndex;
	KineticLeaderboardEntry tempMyEntry;

	UGCHandle_t replayToUploadHandle;

	SteamLeaderboardManager();
	~SteamLeaderboardManager();
	void Reset();
	//void FailureAlert();
	void UploadingScoreFailed();
	void UploadingScoreSucceeded();
	bool IsIdle();
	void UploadScore(const std::string &name, int score, const std::string &replayPath );
	int GetNumActiveGhosts();
	void DownloadBoard(const std::string &name);
	void DeleteCloudReplay();
	void UncheckAllGhosts();
	void RefreshCurrBoard();
private:
	CCallResult<SteamLeaderboardManager,
		LeaderboardFindResult_t> onLeaderboardFindResultCallResult;

	CCallResult<SteamLeaderboardManager,
		LeaderboardScoreUploaded_t> onLeaderboardScoreUploadedCallResult;

	CCallResult<SteamLeaderboardManager,
		LeaderboardScoresDownloaded_t> onLeaderboardScoresDownloadedCallResult;

	CCallResult<SteamLeaderboardManager,
		RemoteStorageFileShareResult_t> onRemoteStorageFileShareResultCallResult;

	CCallResult<SteamLeaderboardManager,
		LeaderboardUGCSet_t> onLeaderboardUGCSetCallResult;

	void OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure);
	void OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure);
	void OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure);
	void OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *callback, bool bIOFailure);
	void OnLeaderboardUGCSet(LeaderboardUGCSet_t *callback, bool bIOFailure);
	void FindLeaderboard(const std::string &name, int postAction, bool forUpload = false );

	//from the storage handler, doesnt directly handle the call result
	void OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure);

	void StartUploadingScore();
	void StartUploadingReplay();
	void StartSharingReplay();
	void StartAttachingReplay();
	void StartDownloadingMyScore();
};

struct LeaderboardEntryRow
{
	bool set;
	HyperLink *nameLink;
	sf::Text scoreText;
	sf::Text rankText;
	CheckBox *ghostCheckBox;
	Button *watchButton;
	int index;

	float rankSpacing;
	float nameSpacing;
	float scoreSpacing;
	float ghostSpacing;
	float watchSpacing;

	LeaderboardEntryRow();
	void Init( int index, Panel *p);
	void Clear();
	void Set(KineticLeaderboardEntry &entryInfo);
	void SetTopLeft(const sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
};

struct LeaderboardDisplay : GUIHandler, PanelUpdater
{
	//placing, icon, name, time

	enum Action
	{
		A_HIDDEN,
		A_LOADING,
		A_SHOWING,
		A_UPLOAD_FAILED_POPUP,
		A_RUNNING_REPLAY,
		A_WAITING_FOR_REPLAY,
		A_RACING_GHOSTS,
	};

	enum UploadType
	{
		UPLOAD_TYPE_ANY_POWERS,
		UPLOAD_TYPE_BOTH_BOARDS,
	};

	enum UploadState
	{
		UPLOAD_STATE_NONE,
		UPLOAD_STATE_ANY_POWERS,
		UPLOAD_STATE_ORIG_POWERS,
	};

	int uploadType;

	int uploadState;
	bool successfulScoreChange;

	MessagePopup messagePop;
	SteamLeaderboardManager manager;
	int action;
	int frame;

	Panel *panel;

	Label *titleLabel;

	Button *clearCheckedGhostsButton;
	Button *raceGhostsButton;
	Button *refreshBoardButton;
	CheckBox *originalGhostCheckBox;
	Label *originalGhostCheckBoxLabel;

	CheckBox *showGhostsWithReplayCheckBox;
	Label *showGhostsWithReplayLabel;

	CheckBox *friendsOnlyCheckBox;
	Label *friendsOnlyLabel;
	ScrollBar *scrollBar;
	TabGroup *tabs;

	CheckBox *ghostsUseOriginalSkinsCheckBox;
	Label *ghostsUseOriginalSkinsLabel;

	int tabWhenDownloadingBoard;
	std::string origPowersBoardName;
	std::string anyPowersBoardName;

	int leaderboardBGBorder;

	Label *topRowRankLabel;
	Label *topRowNameLabel;
	Label *topRowTimeLabel;
	Label *topRowGhostLabel;
	Label *topRowReplayLabel;

	sf::Color evenColor;
	sf::Color oddColor;
	sf::Color myColor;

	sf::Vertex topRowQuad[4];

	sf::Vertex dividerQuads[5 * 4];

	std::vector<CSteamID> storedCheckedGhosts;

	sf::Vertex fullscreenBGQuad[4];
	sf::Vertex leaderboardBGQuad[4];
	sf::Vertex titleQuad[4];

	const static int NUM_ROWS = 10;
	const static int ROW_WIDTH = 800;//700;
	const static int ROW_HEIGHT = 50;
	const static int CHAR_HEIGHT = ROW_HEIGHT - 20;
	sf::Vertex rowQuads[NUM_ROWS * 4];

	LeaderboardEntryRow rows[NUM_ROWS];

	PlayerReplayManager *replayChosen;
	int chosenReplayIndex;


	//int topRow;
	//int maxTopRow;
	sf::Vector2f topLeft;

	LeaderboardDisplay();
	~LeaderboardDisplay();

	void Clear();
	bool IsFriendsOnlyMode();
	void SetBoards(const std::string &leaderboardDisplayName, const std::string &origPowers, const std::string &anyPowers);
	bool IsAnyPowersMode();
	void SetAnyPowersMode(bool on);
	void Reset();
	void Start();
	void SetTopLeft(const sf::Vector2f &p_pos);
	void HandleEvent(sf::Event ev);
	void Update();
	void UploadScore(int score, const std::string &replayPath, bool origCompatible);
	void Show();
	void Hide();
	bool IsHidden();
	void AddGhostsToVec(std::vector<ReplayGhost*> &vec, PlayerReplayManager *ignore = NULL);
	void AddPlayerReplayManagersToVec(std::vector<PlayerReplayManager*> &vec, PlayerReplayManager *ignore = NULL);
	void SetActive(bool replay, bool ghost, PlayerReplayManager *ignore = NULL);
	void Draw(sf::RenderTarget *target);
	
	int GetNumActiveLeaderboardGhosts();
	bool IsTryingToStartReplay();
	bool IsTryingToRaceGhosts();
	bool IsUsingPlayerGhostSkins();
	bool IsDefaultGhostOn();
	bool ShouldShowGhostsWithReplay();
	void DownloadCurrBoard();
	
	void MouseScroll(int delta);
	void PopulateRows();

	void OnManagerUploadingScoreFailed();
	void OnManagerUploadingScoreSucceeded();
	void OnManagerScoreWasNotGoodEnoughToUpload();
	
	void RefreshCurrBoard();

	void ButtonCallback(Button *b, const std::string & e);
	void CheckBoxCallback(CheckBox *cb, const std::string & e);
	void ScrollBarCallback(ScrollBar *sb, const std::string &e);
	void TabGroupCallback(TabGroup *tg, const std::string &e);
};


#endif