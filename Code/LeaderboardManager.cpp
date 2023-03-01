#include "Leaderboard.h"
#include <iostream>
#include "Session.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


LeaderboardManager::LeaderboardManager()
{
	action = A_IDLE;
}

LeaderboardManager::~LeaderboardManager()
{

}

void LeaderboardManager::UploadScore(const std::string &name, int score, const std::string &replayPath)
{
	scoreToUpload = score;

	string userAccountIDStr = to_string(SteamUser()->GetSteamID().GetAccountID());

	localReplayPath = replayPath;

	FindLeaderboard(name, A_UPLOAD_REPLAY);
}

void LeaderboardManager::DownloadBoard(const std::string &name)
{
	FindLeaderboard(name, A_DOWNLOADING);
}

bool LeaderboardManager::IsIdle()
{
	return action == A_IDLE;
}

void LeaderboardManager::FailureAlert()
{
	action = A_IDLE;

	Session *sess = Session::GetSession();
	if (sess != NULL)
	{
		sess->StartAlertBox("Score failed to upload. Try again later.");
	}
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

		if (action == A_UPLOAD_REPLAY)
		{
			MainMenu *mainMenu = MainMenu::GetInstance();
			bool res = mainMenu->remoteStorageManager->UploadAsync(localReplayPath, this);

			if (!res)
			{
				FailureAlert();
			}
		}
		else if (action == A_DOWNLOADING)
		{
			SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(currBoard.leaderboardID, ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobal, 0, 100);
			onLeaderboardScoresDownloadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardScoresDownloaded);
		}
	}
	else
	{
		cout << "Error: leaderboard " << searchBoardName << " not found.\n";
		FailureAlert();
	}
}

void LeaderboardManager::OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure)
{
	assert(action == A_UPLOADING_SCORE);

	if (callback->m_bSuccess)
	{
		action = A_ATTACH_REPLAY;
		SteamAPICall_t call = SteamUserStats()->AttachLeaderboardUGC(currBoard.leaderboardID, replayToUploadHandle);
		onLeaderboardUGCSetCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardUGCSet);

		cout << "leaderboard upload successful. adding UGC\n";

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
		FailureAlert();

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

void LeaderboardManager::OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *callback, bool bIOFailure)
{
	if (callback->m_eResult == k_EResultOK)
	{
		replayToUploadHandle = callback->m_hFile;
		action = A_UPLOADING_SCORE;

		SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(currBoard.leaderboardID, ELeaderboardUploadScoreMethod::k_ELeaderboardUploadScoreMethodKeepBest, scoreToUpload, NULL, 0);
		onLeaderboardScoreUploadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardScoreUploaded);
	}
	else
	{
		FailureAlert();

		cout << "remote file share failed on " << callback->m_rgchFilename << ". reason: " << callback->m_eResult << "\n";
	}
}

void LeaderboardManager::OnLeaderboardUGCSet(LeaderboardUGCSet_t *callback, bool bIOFailure)
{
	assert(action == A_ATTACH_REPLAY);

	if (callback->m_eResult == k_EResultOK)
	{
		Session *sess = Session::GetSession();
		if (sess != NULL)
		{
			sess->StartAlertBox("Leaderboard score updated successfully.");
		}

		cout << "full leaderboard submission complete and successful\n";

		action = A_IDLE;
	}
	else
	{
		FailureAlert();

		cout << "leaderboard ugc set failed on  " << localReplayPath << ". reason: " << callback->m_eResult << "\n";
	}
}

void LeaderboardManager::OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure)
{
	if (callback->m_eResult == k_EResultOK)
	{
		cout << "remote storage upload from leaderboard manager complete" << "\n";
		assert(action == A_UPLOAD_REPLAY);

		action = A_SHARE_REPLAY;
		
		MainMenu *mainMenu = MainMenu::GetInstance();
		SteamAPICall_t call = mainMenu->remoteStorageManager->FileShare(localReplayPath);
		onRemoteStorageFileShareResultCallResult.Set(call, this, &LeaderboardManager::OnRemoteStorageFileShareResult);
	}
	else
	{
		cout << "remote storage upload from leaderboad manager failed\n";
	}
}