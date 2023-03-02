#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"
#include "Session.h"
#include "UIMouse.h"
#include <fstream>
#include "PlayerRecord.h"
#include <sstream>

using namespace std;
using namespace sf;

KineticLeaderboardEntry::KineticLeaderboardEntry()
{
	playerReplayManager = NULL;
	Clear();
}

KineticLeaderboardEntry::KineticLeaderboardEntry(const KineticLeaderboardEntry &k)
{
	steamEntry = k.steamEntry;
	name = k.name;
	timeStr = k.timeStr;
	replayPath = k.replayPath;;
	onRemoteStorageDownloadUGCResultCallResult.Cancel();
	action = k.action;
}

void KineticLeaderboardEntry::Init()
{
	name = SteamFriends()->GetFriendPersonaName(steamEntry.m_steamIDUser);
	timeStr = GetTimeStr(steamEntry.m_nScore);
	action = A_INITIALIZED;
}

void KineticLeaderboardEntry::DownloadReplay()
{
	if (action == A_INITIALIZED)
	{
		cout << "starting download " << name << "'s replay" << endl;

		action = A_DOWNLOADING;

		//string downloadDest = boost::filesystem::current_path().string() + "\\" + "test.kinreplay";//"test.kinreplay";//boost::filesystem::current_path().string();// +"\\LeaderboardReplays\\test.kinreplay";// +//replayPath;//+ name + ".kinreplay";
		//SteamAPICall_t call = SteamRemoteStorage()->UGCDownloadToLocation(steamEntry.m_hUGC, downloadDest.c_str(), 0);
		SteamAPICall_t call = SteamRemoteStorage()->UGCDownload(steamEntry.m_hUGC, 0);
		onRemoteStorageDownloadUGCResultCallResult.Set(call, this, &KineticLeaderboardEntry::OnRemoteStorageDownloadUGCResult);
	}
}

void KineticLeaderboardEntry::Clear()
{
	ghostOn = false;
	name = "";
	timeStr = "";
	replayPath = "";
	onRemoteStorageDownloadUGCResultCallResult.Cancel();
	memset(&steamEntry, 0, sizeof(steamEntry));
	action = A_CLEAR;
	if (playerReplayManager != NULL)
		delete playerReplayManager;
	playerReplayManager = NULL;
}

void KineticLeaderboardEntry::OnRemoteStorageDownloadUGCResult(RemoteStorageDownloadUGCResult_t *callback, bool bIOFailure)
{
	if (callback->m_eResult == k_EResultOK)
	{
		AppId_t ugcAppID;
		char *ugcNameArr;
		int32 ugcFileSize;
		CSteamID ugcOwner;

		SteamRemoteStorage()->GetUGCDetails(callback->m_hFile, &ugcAppID, (&ugcNameArr), &ugcFileSize, &ugcOwner);

		string ugcName = ugcNameArr;

		replayPath = "Resources\\LeaderboardGhosts\\" + ugcName;

		cout << "remote storage download ugc success: " << callback->m_pchFileName << "\n";

		action = A_SAVING;

		char *fileBytes = new char[ugcFileSize];

		int numBytesRead = SteamRemoteStorage()->UGCRead(callback->m_hFile, fileBytes, ugcFileSize, 0, EUGCReadAction::k_EUGCRead_Close);

		if (numBytesRead == ugcFileSize)
		{
			stringstream ss;
			ss.write(fileBytes, ugcFileSize);

			assert(playerReplayManager == NULL);
			playerReplayManager = new PlayerReplayManager;
			playerReplayManager->LoadFromStream(ss);

			action = A_READY;
			cout << "replay is ready\n";
			/*ofstream of;
			of.open(replayPath, ios::binary | ios::out);

			if (of.is_open())
			{
				stringstream ss;
				ss.write( fileBytes, ugcFileSize);

				assert(playerReplayManager == NULL);
				playerReplayManager = new PlayerReplayManager;
				playerReplayManager->LoadFromStream(ss);

				action = A_READY;
				cout << "replay is ready\n";
			}
			else
			{
				cout << "could not write file after reading\n";
				action = A_INITIALIZED;
			}*/
		}
		else
		{
			cout << "could not read all the bytes from the ugc file" << "\n";
			action = A_INITIALIZED;
		}


		delete[]fileBytes;
	}
	else
	{
		action = A_INITIALIZED; //assumes its been initialized before downloading anyway
		cout << "remote storage download ugc failed. error: " << callback->m_eResult << "\n";
	}
}

LeaderboardDisplay::LeaderboardDisplay()
{
	topIndex = 0;

	panel = new Panel("leaderboard", 1920, 1080, this);
	panel->SetColor(Color::Transparent);

	Hide();

	Color evenColor = Color(200, 200, 200, 200);
	Color oddColor = Color(150, 150, 150, 200);
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (i % 2 == 0)
		{
			SetRectColor(rowQuads + i * 4, evenColor);
		}
		else
		{
			SetRectColor(rowQuads + i * 4, oddColor );
		}
		
	}

	SetRectColor(bgQuad, Color(100, 100, 200, 150));
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Init(i, panel);
	}

	SetTopLeft(Vector2f(960 - ROW_WIDTH / 2, 540 - (NUM_ROWS / 2 * ROW_HEIGHT)));
}

LeaderboardDisplay::~LeaderboardDisplay()
{
	delete panel;
}

bool LeaderboardDisplay::IsHidden()
{
	return action == A_HIDDEN;
}

void LeaderboardDisplay::Show()
{
	action = A_LOADING;
	frame = 0;

	MOUSE.Show();
	MOUSE.SetControllersOn(true);
}

void LeaderboardDisplay::Hide()
{
	action = A_HIDDEN;
	frame = 0;

	MOUSE.Hide();
	MOUSE.SetControllersOn(false);
}

void LeaderboardDisplay::Start( const std::string &boardName )
{
	Show();

	manager.DownloadBoard(boardName);
}

void LeaderboardDisplay::SetTopLeft(const sf::Vector2f &p_pos)
{
	topLeft = p_pos;

	Vector2f rowTopLeft;
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rowTopLeft = topLeft + Vector2f(0, i * ROW_HEIGHT);
		SetRectTopLeft(rowQuads + i * 4, ROW_WIDTH, ROW_HEIGHT, rowTopLeft);
		rows[i].SetTopLeft(rowTopLeft);
	}
}

void LeaderboardDisplay::HandleEvent(sf::Event ev)
{
	if (IsHidden())
		return;

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.panel->HandleEvent(ev);
	}
	else
	{
		panel->HandleEvent(ev);
	}
}

void LeaderboardDisplay::Update()
{
	if (action == A_HIDDEN)
		return;

	if (action == A_LOADING)
	{
		if (manager.IsIdle())
		{
			action = A_SHOWING;
			frame = 0;

			int numEntries = manager.currBoard.entries.size();

			for (int i = 0; i < NUM_ROWS; ++i)
			{
				rows[i].Clear();
			}

			for (int i = 0; i < NUM_ROWS; ++i)
			{
				if (i == numEntries)
				{
					break;
				}

				rows[i].Set(manager.currBoard.entries[i]);
			}
			//if (false)//manager.lastUploadSuccess)
			//{
			//	
			//}
			//else
			//{
			//	messagePop.Pop("Score upload failed. Try again later");
			//	action = A_UPLOAD_FAILED_POPUP;
			//}
			
		}
	}

	Session *sess = Session::GetSession();

	ControllerState currState = sess->GetCurrInput(0);
	ControllerState prevState = sess->GetPrevInput(0);

	if (currState.start && !prevState.start)
	{
		Hide();
	}

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.Update();

		if (!messagePop.IsActive())
		{
			action = A_LOADING;
		}
	}
	else
	{
		panel->MouseUpdate();
	}

	++frame;
}

int LeaderboardDisplay::GetNumActiveGhosts()
{
	return manager.GetNumActiveGhosts();
}

void LeaderboardDisplay::AddGhostsToVec(std::vector<ReplayGhost*> &vec)
{
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn && (*it).playerReplayManager != NULL)
		{
			(*it).playerReplayManager->AddGhostsToVec(vec);
			(*it).playerReplayManager->ghostsActive = true;
		}
	}
}

void LeaderboardDisplay::AddPlayerReplayManagersToVec(std::vector<PlayerReplayManager*> &vec)
{
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn && (*it).playerReplayManager != NULL)
		{
			vec.push_back((*it).playerReplayManager);
		}
	}
}

void LeaderboardDisplay::SetActive(bool replay, bool ghost)
{
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn && (*it).playerReplayManager != NULL)
		{
			(*it).playerReplayManager->replaysActive = replay;
			(*it).playerReplayManager->ghostsActive = ghost;
		}
	}
}

void LeaderboardDisplay::Draw(sf::RenderTarget *target)
{
	if (action == A_HIDDEN)
	{
		return;
	}

	target->draw(bgQuad, 4, sf::Quads);

	target->draw(rowQuads, NUM_ROWS * 4, sf::Quads);

	if (action == A_SHOWING)
	{
		for (int i = 0; i < NUM_ROWS; ++i)
		{
			rows[i].Draw(target);
		}

		panel->Draw(target);
	}

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.Draw(target);
	}
}

void LeaderboardDisplay::ButtonCallback(Button *b, const std::string & e)
{
	int rowIndex = -1;
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (b == rows[i].watchButton)
		{
			rowIndex = i;
			break;
		}
	}

	if (rowIndex >= 0)
	{
		int trueIndex = topIndex + rowIndex;

		cout << "watch replay: " << trueIndex << "\n";
	}
}

void LeaderboardDisplay::CheckBoxCallback(CheckBox *cb, const std::string & e)
{
	int rowIndex = -1;
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (cb == rows[i].ghostCheckBox)
		{
			rowIndex = i;
			break;
		}
	}

	if (rowIndex >= 0)
	{
		int trueIndex = topIndex + rowIndex;

		manager.currBoard.entries[trueIndex].ghostOn = cb->checked;

		if (cb->checked)
		{
			
			manager.currBoard.entries[trueIndex].DownloadReplay();
			cout << "downloading replay because you checked ghost: " << trueIndex << "\n";
			
		}
		else
		{
			//don't really need to delete the ghost file, nbd for now. Just deleting it when you leave the level anyway
		}
	}
}

