#ifndef __LEADERBOARD_MANAGER_H__
#define __LEADERBOARD_MANAGER_H__

#include <SFML\Graphics.hpp>
#include "steam\steam_api.h"
#include <vector>

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

	LeaderboardManager();
	~LeaderboardManager();
	void UploadScore(int score);
private:
	CCallResult<LeaderboardManager,
		LeaderboardFindResult_t> onLeaderboardFindResultCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardScoreUploaded_t> onLeaderboardScoreUploadedCallResult;

	CCallResult<LeaderboardManager,
		LeaderboardScoresDownloaded_t> onLeaderboardScoresDownloadedCallResult;

	void OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure);
	void OnLeaderboardUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure);
	void OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure);
	void FindLeaderboard(const std::string &name, int postAction );
};

struct LeaderboardEntryRow
{
	bool set;
	sf::Text nameText;
	sf::Text scoreText;

	LeaderboardEntryRow();
	void Clear();
	void Set(KineticLeaderboardEntry &entryInfo);
	void SetTopLeft(const sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target);
};

struct LeaderboardDisplay
{
	//placing, icon, name, time

	LeaderboardManager manager;


	const static int NUM_ROWS = 10;
	const static int ROW_WIDTH = 400;
	const static int ROW_HEIGHT = 30;
	sf::Vertex rowQuads[NUM_ROWS * 4];

	LeaderboardEntryRow rows[NUM_ROWS];
	LeaderboardInfo *currBoard;

	int topIndex;
	sf::Vector2f topLeft;

	LeaderboardDisplay();
	void Start(LeaderboardInfo *currBoard);
	void SetTopLeft(const sf::Vector2f &p_pos);
	void Update();
	void Draw(sf::RenderTarget *target);
};


#endif