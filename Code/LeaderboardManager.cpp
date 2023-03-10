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

SteamLeaderboardManager::SteamLeaderboardManager()
{
	display = NULL;
	Reset();
}

void SteamLeaderboardManager::Reset()
{
	action = A_IDLE;
	onLeaderboardFindResultCallResult.Cancel();
	onLeaderboardScoreUploadedCallResult.Cancel();
	onLeaderboardScoresDownloadedCallResult.Cancel();
	onRemoteStorageFileShareResultCallResult.Cancel();
	onLeaderboardUGCSetCallResult.Cancel();

	replayToUploadHandle = 0;
	postFindAction = -1;
	myEntryIndex = -1;
	localReplayPath = "";
	cloudReplayPath = "";
	searchBoardName = "";
	scoreToUpload = -1;

	currBoard.Clear();
}

SteamLeaderboardManager::~SteamLeaderboardManager()
{

}

int SteamLeaderboardManager::GetNumActiveGhosts()
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

void SteamLeaderboardManager::UncheckAllGhosts()
{
	for (auto it = currBoard.entries.begin(); it != currBoard.entries.end(); ++it)
	{
		(*it).ghostOn = false;
	}
}

void SteamLeaderboardManager::UploadScore(const std::string &name, int score, const std::string &replayPath)
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

void SteamLeaderboardManager::DownloadBoard(const std::string &name)
{
	FindLeaderboard(name, A_DOWNLOADING);
}

bool SteamLeaderboardManager::IsIdle()
{
	return action == A_IDLE;
}

void SteamLeaderboardManager::UploadingScoreFailed()
{
	action = A_IDLE;//A_UPLOAD_SCORE_FAILURE;

	if (display != NULL)
	{
		display->OnManagerUploadingScoreFailed();
	}
}

void SteamLeaderboardManager::UploadingScoreSucceeded()
{
	action = A_IDLE;//A_UPLOAD_SCORE_SUCCESS;

	if (display != NULL)
	{
		display->OnManagerUploadingScoreSucceeded();
	}
}

void SteamLeaderboardManager::FindLeaderboard(const std::string &name, int p_action)
{
	postFindAction = p_action;
	action = A_FINDING;

	currBoard.Clear();

	searchBoardName = name;

	SteamAPICall_t call = SteamUserStats()->FindOrCreateLeaderboard(name.c_str(), ELeaderboardSortMethod::k_ELeaderboardSortMethodAscending, ELeaderboardDisplayType::k_ELeaderboardDisplayTypeNumeric);

	onLeaderboardFindResultCallResult.Set(call, this, &SteamLeaderboardManager::OnLeaderboardFound);
}

void SteamLeaderboardManager::RefreshCurrBoard()
{
	cout << "refreshing board: " << currBoard.name << "\n";
	myEntryIndex = -1;
	action = A_DOWNLOADING;
	currBoard.ClearEntries();
	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntries(currBoard.leaderboardID, ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobal, 0, 100);
	onLeaderboardScoresDownloadedCallResult.Set(call, this, &SteamLeaderboardManager::OnLeaderboardScoresDownloaded);
}


void SteamLeaderboardManager::OnLeaderboardFound(LeaderboardFindResult_t *callback, bool bIOFailure)
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
		UploadingScoreFailed();
	}
}



void SteamLeaderboardManager::OnLeaderboardScoreUploaded(LeaderboardScoreUploaded_t *callback, bool bIOFailure)
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
		UploadingScoreFailed();

		cout << "leaderboard upload failed\n";
	}
}

void SteamLeaderboardManager::OnLeaderboardScoresDownloaded(LeaderboardScoresDownloaded_t *callback, bool bIOFailure)
{
	if (action == A_DOWNLOADING)
	{
		action = A_IDLE;

		int numEntries = callback->m_cEntryCount;

		myEntryIndex = -1;
		currBoard.entries.resize(numEntries);

		for (int i = 0; i < numEntries; ++i)
		{
			SteamUserStats()->GetDownloadedLeaderboardEntry(callback->m_hSteamLeaderboardEntries, i, &(currBoard.entries[i].steamEntry), NULL, 0);
			currBoard.entries[i].Init();


			if (currBoard.entries[i].steamEntry.m_steamIDUser == SteamUser()->GetSteamID())
			{
				myEntryIndex = i;
			}
		}
	}
	else if( action == A_DOWNLOADING_MY_SCORE )
	{
		if (callback->m_cEntryCount == 0)
		{
			cout << "no old score was found." << "\n";
			tempMyEntry.Clear(); //can this just be a local var?
			StartUploadingReplay();
			//action = A_IDLE; //gets changed on success
		}
		else
		{
			action = A_IDLE; //gets changed on success
			tempMyEntry.Clear();
			SteamUserStats()->GetDownloadedLeaderboardEntry(callback->m_hSteamLeaderboardEntries, 0, &(tempMyEntry.steamEntry), NULL, 0);
			tempMyEntry.Init();

			if (scoreToUpload < tempMyEntry.steamEntry.m_nScore)
			{
				cout << "new time of " << scoreToUpload << " is better than the old time of " << tempMyEntry.steamEntry.m_nScore << "\n";
				StartUploadingReplay();
				//StartUploadingScore();

			}
			else
			{
				cout << "new time of " << scoreToUpload << " is worse than the old time of " << tempMyEntry.steamEntry.m_nScore << "\n";
				if (display != NULL)
					display->OnManagerScoreWasNotGoodEnoughToUpload();
			}
		}
	}
}



void SteamLeaderboardManager::OnLeaderboardUGCSet(LeaderboardUGCSet_t *callback, bool bIOFailure)
{
	assert(action == A_ATTACH_REPLAY);

	if (callback->m_eResult == k_EResultOK)
	{
		bool res = SteamRemoteStorage()->FileDelete(cloudReplayPath.c_str());
		if (res)
		{
			cout << "file deleted post-leaderboard update" << "\n";
		}
		else
		{
			cout << "file tried to delete after leaderboard update but wasn't found\n";
		}


		UploadingScoreSucceeded();

	}
	else
	{
		UploadingScoreFailed();

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


void SteamLeaderboardManager::OnRemoteStorageFileShareResult(RemoteStorageFileShareResult_t *callback, bool bIOFailure)
{
	if (callback->m_eResult == k_EResultOK)
	{
		replayToUploadHandle = callback->m_hFile;
		cout << "file shared successfully" << "\n";

		StartUploadingScore();
	}
	else
	{
		UploadingScoreFailed();

		cout << "remote file share failed on " << callback->m_rgchFilename << ". reason: " << callback->m_eResult << "\n";
	}
}

void SteamLeaderboardManager::StartDownloadingMyScore()
{
	cout << "Start downloading my score" << "\n";

	action = A_DOWNLOADING_MY_SCORE;

	CSteamID myID = SteamUser()->GetSteamID();

	SteamAPICall_t call = SteamUserStats()->DownloadLeaderboardEntriesForUsers(currBoard.leaderboardID, &myID, 1);

	onLeaderboardScoresDownloadedCallResult.Set(call, this, &SteamLeaderboardManager::OnLeaderboardScoresDownloaded);
}

void SteamLeaderboardManager::StartUploadingScore()
{
	cout << "Start uploading score" << "\n";

	action = A_UPLOADING_SCORE;
	SteamAPICall_t call = SteamUserStats()->UploadLeaderboardScore(currBoard.leaderboardID, ELeaderboardUploadScoreMethod::k_ELeaderboardUploadScoreMethodForceUpdate, scoreToUpload, NULL, 0);
	onLeaderboardScoreUploadedCallResult.Set(call, this, &SteamLeaderboardManager::OnLeaderboardScoreUploaded);
}

void SteamLeaderboardManager::StartUploadingReplay()
{
	cout << "Start uploading replay:" << cloudReplayPath << "\n";

	MainMenu *mainMenu = MainMenu::GetInstance();
	bool res = mainMenu->remoteStorageManager->UploadAsync(localReplayPath, cloudReplayPath, this);
	if (!res)
	{
		UploadingScoreFailed();
	}
}

void SteamLeaderboardManager::StartSharingReplay()
{
	cout << "start sharing replay" << "\n";

	action = A_SHARE_REPLAY;

	MainMenu *mainMenu = MainMenu::GetInstance();
	SteamAPICall_t call = mainMenu->remoteStorageManager->FileShare(cloudReplayPath);
	onRemoteStorageFileShareResultCallResult.Set(call, this, &SteamLeaderboardManager::OnRemoteStorageFileShareResult);
}

void SteamLeaderboardManager::StartAttachingReplay()
{
	cout << "Start attaching replay" << "\n";

	action = A_ATTACH_REPLAY;
	SteamAPICall_t call = SteamUserStats()->AttachLeaderboardUGC(currBoard.leaderboardID, replayToUploadHandle);
	onLeaderboardUGCSetCallResult.Set(call, this, &SteamLeaderboardManager::OnLeaderboardUGCSet);
}

//from RemoteStorageManager
void SteamLeaderboardManager::OnRemoteStorageFileWriteAsyncComplete(RemoteStorageFileWriteAsyncComplete_t *callback, bool bIOFailure)
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
