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

struct LeaderboardManager : RemoteStorageResultHandler
{
	enum Action
	{
		A_IDLE,
		A_FINDING,
		A_UPLOAD_REPLAY,
		A_SHARE_REPLAY,
		A_UPLOADING_SCORE,
		A_ATTACH_REPLAY,
		A_DOWNLOADING,
		A_DOWNLOADING_MY_SCORE,
	};

	int action;
	LeaderboardInfo currBoard;
	int scoreToUpload;
	std::string localReplayPath;
	std::string cloudReplayPath;
	std::string searchBoardName;
	int postFindAction;

	KineticLeaderboardEntry myEntry;

	UGCHandle_t replayToUploadHandle;

	LeaderboardManager();
	~LeaderboardManager();
	void FailureAlert();
	bool IsIdle();
	void UploadScore(const std::string &name, int score, const std::string &replayPath );
	int GetNumActiveGhosts();
	void DownloadBoard(const std::string &name);
	void UncheckAllGhosts();
	void RefreshCurrBoard();
private:
	CCallResult<LeaderboardManager,
		LeaderboardFindResult_t> onLeaderboardFindResultCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardScoreUploaded_t> onLeaderboardScoreUploadedCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardScoresDownloaded_t> onLeaderboardScoresDownloadedCallResult;

	CCallResult<LeaderboardManager,
		RemoteStorageFileShareResult_t> onRemoteStorageFileShareResultCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardUGCSet_t> onLeaderboardUGCSetCallResult;

	void OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure);
	void OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure);
	void OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure);
	void OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *callback, bool bIOFailure);
	void OnLeaderboardUGCSet(LeaderboardUGCSet_t *callback, bool bIOFailure);
	void FindLeaderboard(const std::string &name, int postAction );

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

	LeaderboardEntryRow();
	void Init( int index, Panel *p);
	void Clear();
	void Set(KineticLeaderboardEntry &entryInfo);
	void SetTopLeft(const sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
};

struct LeaderboardDisplay : GUIHandler
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

	MessagePopup messagePop;
	LeaderboardManager manager;
	int action;
	int frame;

	Panel *panel;

	Button *clearCheckedGhostsButton;
	Button *raceGhostsButton;
	Button *refreshBoardButton;
	CheckBox *originalGhostCheckBox;
	Label *originalGhostCheckBoxLabel;
	

	sf::Vertex bgQuad[4];

	const static int NUM_ROWS = 10;
	const static int ROW_WIDTH = 700;
	const static int ROW_HEIGHT = 50;
	const static int CHAR_HEIGHT = ROW_HEIGHT - 20;
	sf::Vertex rowQuads[NUM_ROWS * 4];

	LeaderboardEntryRow rows[NUM_ROWS];

	PlayerReplayManager *replayChosen;
	int chosenReplayIndex;


	int topIndex;
	sf::Vector2f topLeft;

	LeaderboardDisplay();
	~LeaderboardDisplay();
	void Start( const std::string &boardName );
	void SetTopLeft(const sf::Vector2f &p_pos);
	void HandleEvent(sf::Event ev);
	void Update();
	void Show();
	void Hide();
	bool IsHidden();
	void AddGhostsToVec(std::vector<ReplayGhost*> &vec);
	void AddPlayerReplayManagersToVec(std::vector<PlayerReplayManager*> &vec);
	void SetActive(bool replay, bool ghost);
	void Draw(sf::RenderTarget *target);
	void ButtonCallback(Button *b, const std::string & e);
	void CheckBoxCallback(CheckBox *cb, const std::string & e);
	int GetNumActiveLeaderboardGhosts();
	bool IsTryingToStartReplay();
	bool IsTryingToRaceGhosts();
	bool IsDefaultGhostOn();
};


#endif