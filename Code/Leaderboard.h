#ifndef __LEADERBOARD_MANAGER_H__
#define __LEADERBOARD_MANAGER_H__

#include <SFML\Graphics.hpp>
#include "steam\steam_api.h"
#include <vector>
#include "GUI.h"

struct KineticLeaderboardEntry
{
	LeaderboardEntry_t steamEntry;
	std::string name;
	std::string timeStr;

	KineticLeaderboardEntry();
	void Init();
	void Clear();
};

struct LeaderboardInfo
{
	SteamLeaderboard_t leaderboardID;
	std::vector<KineticLeaderboardEntry> entries;
};

struct LeaderboardManager
{
	enum Action
	{
		A_IDLE,
		A_FINDING,
		A_UPLOADING,
		A_DOWNLOADING,
	};

	int action;
	LeaderboardInfo currBoard;
	int scoreToUpload;
	std::string searchBoardName;
	int postFindAction;
	bool lastUploadSuccess;

	LeaderboardManager();
	~LeaderboardManager();
	bool IsIdle();
	void UploadScore(const std::string &name, int score);
	void DownloadBoard(const std::string &name);
private:
	CCallResult<LeaderboardManager,
		LeaderboardFindResult_t> onLeaderboardFindResultCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardScoreUploaded_t> onLeaderboardScoreUploadedCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardScoresDownloaded_t> onLeaderboardScoresDownloadedCallResult;

	void OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure);
	void OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure);
	void OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure);
	void FindLeaderboard(const std::string &name, int postAction );
};

struct LeaderboardEntryRow
{
	bool set;
	HyperLink *nameLink;
	sf::Text scoreText;
	sf::Text rankText;
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
	};

	MessagePopup messagePop;
	LeaderboardManager manager;
	int action;
	int frame;

	Panel *panel;

	sf::Vertex bgQuad[4];

	const static int NUM_ROWS = 10;
	const static int ROW_WIDTH = 500;
	const static int ROW_HEIGHT = 50;
	const static int CHAR_HEIGHT = ROW_HEIGHT - 20;
	sf::Vertex rowQuads[NUM_ROWS * 4];

	LeaderboardEntryRow rows[NUM_ROWS];
	LeaderboardInfo *currBoard;

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
	void Draw(sf::RenderTarget *target);
};


#endif