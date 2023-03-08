#include "Leaderboard.h"
#include <iostream>
#include "Session.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;


LeaderboardInfo::LeaderboardInfo()
{
	Clear();
}

void LeaderboardInfo::Clear()
{
	leaderboardID = 0;
	name = "";
	ClearEntries();
}

void LeaderboardInfo::ClearEntries()
{
	entries.clear();
}

LeaderboardManager::LeaderboardManager()
{
	Reset();
}

void LeaderboardManager::Reset()
{
	action = A_IDLE;
	onLeaderboardFindResultCallResult.Cancel();
	onLeaderboardScoreUploadedCallResult.Cancel();
	onLeaderboardScoresDownloadedCallResult.Cancel();
	onRemoteStorageFileShareResultCallResult.Cancel();
	onLeaderboardUGCSetCallResult.Cancel();

	replayToUploadHandle = 0;
	postFindAction = -1;
	localReplayPath = "";
	cloudReplayPath = "";
	searchBoardName = "";
	scoreToUpload = -1;

	currBoard.Clear();
}

LeaderboardManager::~LeaderboardManager()
{

}

int LeaderboardManager::GetNumActiveGhosts()
{
	int numActiveGhosts = 0;
	for (auto it = currBoard.entries.begin(); it != currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn)
		{
			++numActiveGhosts;
		}
	}

	return numActiveGhosts;
}

void LeaderboardManager::UncheckAllGhosts()
{
	for (auto it = currBoard.entries.begin(); it != currBoard.entries.end(); ++it)
	{
		(*it).ghostOn = false;
	}
}

void LeaderboardManager::UploadScore(const std::string &name, int score, const std::string &replayPath)
{
	scoreToUpload = score;

	string userAccountIDStr = to_string(SteamUser()->GetSteamID().GetAccountID());

	localReplayPath = replayPath;

	boost::filesystem::path path = localReplayPath;
	string file = path.filename().string();

	cloudReplayPath = userAccountIDStr + "_" + file;//RemoteStorageManager::GetRemotePath(file);

	FindLeaderboard(name, A_DOWNLOADING_MY_SCORE);

	//1. download my score
	//2. if I have the better score, upload my replay
	//3. once replay has uploaded, share it.
	//4. once replay has been shared, upload score
	//5. once score is up, add replay
	//6. once replay is added, report sucess.
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

	currBoard.Clear();

	searchBoardName = name;

	SteamAPICall_t call = SteamUserStats()->FindOrCreateLeaderboard(name.c_str(), ELeaderboardSortMethod::k_ELeaderboardSortMethodAscending, ELeaderboardDisplayType::k_ELeaderboardDisplayTypeNumeric);

	onLeaderboardFindResultCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardFound);
}

void LeaderboardManager::RefreshCurrBoard()
{
	cout << "refreshing board: " << currBoard.name << "\n";
	action = A_DOWNLOADING;
	currBoard.ClearEntries();
	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(currBoard.leaderboardID, ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobal, 0, 100);
	onLeaderboardScoresDownloadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardScoresDownloaded);
}


void LeaderboardManager::OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure)
{
	assert(action == A_FINDING);

	if (callback->m_bLeaderboardFound)
	{
		cout << "found leaderboard " << searchBoardName << "\n";
		action = postFindAction;

		currBoard.name = searchBoardName;
		currBoard.leaderboardID = callback->m_hSteamLeaderboard;

		if (action == A_DOWNLOADING_MY_SCORE )
		{
			StartDownloadingMyScore();
		}
		else if (action == A_DOWNLOADING)
		{
			RefreshCurrBoard();
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
		cout << "leaderboard score upload successful. uploading UGC\n";

		//always replaces the score because we are doing forced replacement
		StartAttachingReplay();
	}
	else
	{
		FailureAlert();

		cout << "leaderboard upload failed\n";
	}
}

void LeaderboardManager::OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure)
{
	if (action == A_DOWNLOADING)
	{
		action = A_IDLE;

		int numEntries = callback->m_cEntryCount;

		
		currBoard.entries.resize(numEntries);

		for (int i = 0; i < numEntries; ++i)
		{
			SteamUserStats()->GetDownloadedLeaderboardEntry(callback->m_hSteamLeaderboardEntries, i, &(currBoard.entries[i].steamEntry), NULL, 0);
			currBoard.entries[i].Init();
		}
	}
	else if( action == A_DOWNLOADING_MY_SCORE )
	{
		if (callback->m_cEntryCount == 0)
		{
			cout << "no old score was found." << "\n";
			myEntry.Clear(); //can this just be a local var?
			StartUploadingReplay();
			//action = A_IDLE; //gets changed on success
		}
		else
		{
			action = A_IDLE; //gets changed on success
			myEntry.Clear();
			SteamUserStats()->GetDownloadedLeaderboardEntry(callback->m_hSteamLeaderboardEntries, 0, &(myEntry.steamEntry), NULL, 0);
			myEntry.Init();

			if (scoreToUpload < myEntry.steamEntry.m_nScore)
			{
				cout << "new time of " << scoreToUpload << " is better than the old time of " << myEntry.steamEntry.m_nScore << "\n";
				StartUploadingReplay();
				//StartUploadingScore();

			}
			else
			{
				cout << "new time of " << scoreToUpload << " is worse than the old time of " << myEntry.steamEntry.m_nScore << "\n";
			}
		}
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

		bool res = SteamRemoteStorage()->FileDelete(cloudReplayPath.c_str());
		if (res)
		{
			cout << "file deleted post-leaderboard update" << "\n";
		}
		else
		{
			cout << "file tried to delete after leaderboard update but wasn't found\n";
		}
	}
	else
	{
		FailureAlert();

		cout << "leaderboard ugc set failed on  " << cloudReplayPath << ". reason: " << callback->m_eResult << "\n";

		bool res = SteamRemoteStorage()->FileDelete(cloudReplayPath.c_str());

		if (res)
		{
			cout << "failure file deleted post-leaderboard update" << "\n";
		}
		else
		{
			cout << "failure file tried to delete after leaderboard update but wasn't found\n";
		}
	}
}


void LeaderboardManager::OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *callback, bool bIOFailure)
{
	if (callback->m_eResult == k_EResultOK)
	{
		replayToUploadHandle = callback->m_hFile;
		cout << "file shared successfully" << "\n";

		StartUploadingScore();
	}
	else
	{
		FailureAlert();

		cout << "remote file share failed on " << callback->m_rgchFilename << ". reason: " << callback->m_eResult << "\n";
	}
}

void LeaderboardManager::StartDownloadingMyScore()
{
	cout << "Start downloading my score" << "\n";

	action = A_DOWNLOADING_MY_SCORE;

	CSteamID myID = SteamUser()->GetSteamID();

	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntriesForUsers(currBoard.leaderboardID, &myID, 1);

	onLeaderboardScoresDownloadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardScoresDownloaded);
}

void LeaderboardManager::StartUploadingScore()
{
	cout << "Start uploading score" << "\n";

	action = A_UPLOADING_SCORE;
	SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(currBoard.leaderboardID, ELeaderboardUploadScoreMethod::k_ELeaderboardUploadScoreMethodForceUpdate, scoreToUpload, NULL, 0);
	onLeaderboardScoreUploadedCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardScoreUploaded);
}

void LeaderboardManager::StartUploadingReplay()
{
	cout << "Start uploading replay:" << cloudReplayPath << "\n";

	MainMenu *mainMenu = MainMenu::GetInstance();
	bool res = mainMenu->remoteStorageManager->UploadAsync(localReplayPath, cloudReplayPath, this);
	if (!res)
	{
		FailureAlert();
	}
}

void LeaderboardManager::StartSharingReplay()
{
	cout << "start sharing replay" << "\n";

	action = A_SHARE_REPLAY;

	MainMenu *mainMenu = MainMenu::GetInstance();
	SteamAPICall_t call = mainMenu->remoteStorageManager->FileShare(cloudReplayPath);
	onRemoteStorageFileShareResultCallResult.Set(call, this, &LeaderboardManager::OnRemoteStorageFileShareResult);
}

void LeaderboardManager::StartAttachingReplay()
{
	cout << "Start attaching replay" << "\n";

	action = A_ATTACH_REPLAY;
	SteamAPICall_t call = SteamUserStats()->AttachLeaderboardUGC(currBoard.leaderboardID, replayToUploadHandle);
	onLeaderboardUGCSetCallResult.Set(call, this, &LeaderboardManager::OnLeaderboardUGCSet);
}

//from RemoteStorageManager
void LeaderboardManager::OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure)
{
	if (callback->m_eResult == k_EResultOK)
	{
		cout << "remote storage upload from leaderboard manager complete" << "\n";
		StartSharingReplay();
	}
	else
	{
		cout << "remote storage upload from leaderboad manager failed\n";
	}
}
