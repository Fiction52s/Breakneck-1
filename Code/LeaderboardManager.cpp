#include "Leaderboard.h"
#include <iostream>

using namespace std;
using namespace sf;


LeaderboardManager::LeaderboardManager()
{
	action = A_IDLE;
}

LeaderboardManager::~LeaderboardManager()
{

}

void LeaderboardManager::UploadScore(int score)
{
	FindLeaderboard("testboard", A_UPLOADING);

	scoreToUpload = score;
}

void LeaderboardManager::FindLeaderboard(const std::string &name, int p_action)
{
	postFindAction = p_action;
	action = A_FINDING;

	searchBoardName = name;

	SteamAPICall_t call = SteamUserStats()->FindOrCreateLeaderboard(name.c_str(), ELeaderboardSortMethod::k_ELeaderboardSortMethodAscending, ELeaderboardDisplayType::k_ELeaderboardDisplayTypeNumeric);

	onLeaderboardFindResultCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardFound);
}

void LeaderboardManager::OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure)
{
	if (callback->m_bLeaderboardFound)
	{
		cout << "found leaderboard " << searchBoardName << "\n";
		action = postFindAction;

		currBoard.leaderboardID = callback->m_hSteamLeaderboard;

		if (action == A_UPLOADING)
		{
			SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(callback->m_hSteamLeaderboard, ELeaderboardUploadScoreMethod::k_ELeaderboardUploadScoreMethodKeepBest, scoreToUpload, NULL, 0);
			onLeaderboardScoreUploadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardUploaded);
		}
		else if (action == A_DOWNLOADING)
		{
			SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(currBoard.leaderboardID, ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobal, 0, 100);
			onLeaderboardScoresDownloadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardScoresDownloaded);
		}
	}
	else
	{
		action = A_IDLE;
		cout << "Error: leaderboard " << searchBoardName << " not found.\n";
	}
}

void LeaderboardManager::OnLeaderboardUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure)
{
	action = A_IDLE;
	if (callback->m_bSuccess)
	{
		cout << "leaderboard upload successful\n";

		if (callback->m_bScoreChanged)
		{
			cout << "score was changed! New score is: " << callback->m_nScore << "\n";
		}
		else
		{
			cout << "score was not changed!\n";
		}
	}
	else
	{
		cout << "leaderboard upload failed\n";
	}
}

void LeaderboardManager::OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure)
{
	action = A_IDLE;

	int numEntries = callback->m_cEntryCount;

	currBoard.entries.clear();
	currBoard.entries.resize(numEntries);

	for (int i = 0; i < numEntries; ++i)
	{
		SteamUserStats()->GetDownloadedLeaderboardEntry(callback->m_hSteamLeaderboardEntries, i, &(currBoard.entries[i].steamEntry), NULL, 0);
		currBoard.entries[i].Init();
	}
	
}