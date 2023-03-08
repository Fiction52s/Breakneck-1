#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"
#include "Session.h"
#include "UIMouse.h"
#include <fstream>
#include "PlayerRecord.h"
#include <sstream>
#include "UIController.h"

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

bool KineticLeaderboardEntry::IsReplayReady()
{
	return action == A_READY;
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

			string userName = SteamFriends()->GetFriendPersonaName(ugcOwner);

			assert(!playerReplayManager->repVec.empty());

			playerReplayManager->repVec.front()->SetDisplayName(userName);
			

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
	panel = new Panel("leaderboard", 1920, 1080, this);
	
	panel->SetColor(Color::Transparent);

	int buttonHeight = LeaderboardDisplay::CHAR_HEIGHT;

	clearCheckedGhostsButton = panel->AddButton("clearbutton", Vector2i(), Vector2f(200, buttonHeight), "Clear Ghosts");
	raceGhostsButton = panel->AddButton("racebutton", Vector2i(), Vector2f(200, buttonHeight), "Race Ghosts");
	refreshBoardButton = panel->AddButton("refreshbutton", Vector2i(), Vector2f(100, buttonHeight), "Refresh");
	originalGhostCheckBox = panel->AddCheckBox("originalghostcheckbox", Vector2i(), true);
	originalGhostCheckBoxLabel = panel->AddLabel("originalghostcheckboxlabel", Vector2i(), LeaderboardDisplay::CHAR_HEIGHT, "Default Ghost:");

	showGhostsWithReplayCheckBox = panel->AddCheckBox("showghostswthreplaycheckbox", Vector2i(), false);

	scrollBar = panel->AddScrollBar("scroll", Vector2i(), Vector2i(30, NUM_ROWS * ROW_HEIGHT ), 1, 1);

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

void LeaderboardDisplay::Reset()
{
	action = A_HIDDEN;
	frame = 0;
	originalGhostCheckBox->checked = true;
	showGhostsWithReplayCheckBox->checked = false;
	scrollBar->SetIndex(0);

	manager.Reset();
}

void LeaderboardDisplay::Start( const std::string &boardName )
{
	Reset();

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

	Vector2i above(topLeft + Vector2f(0, -200));
	Vector2i aboveLower(topLeft + Vector2f(0, -100));

	clearCheckedGhostsButton->SetPos(above);
	raceGhostsButton->SetPos(above + Vector2i(250, 0));
	refreshBoardButton->SetPos(above + Vector2i(500, 0));

	originalGhostCheckBoxLabel->SetTopLeftPosition(aboveLower);

	originalGhostCheckBox->SetPos(Vector2i(originalGhostCheckBoxLabel->GetTopRight().x + 10, aboveLower.y));

	showGhostsWithReplayCheckBox->SetPos(originalGhostCheckBox->pos + Vector2i(200, 0));

	scrollBar->SetPos(Vector2i(topLeft + Vector2f(ROW_WIDTH, 0)));
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
		panel->extraUpdater = this;
		panel->HandleEvent(ev);
		panel->extraUpdater = NULL;
	}
}

void LeaderboardDisplay::MouseScroll(int delta)
{
	int oldTopRow = scrollBar->currIndex;
	scrollBar->SetIndex(scrollBar->currIndex - delta);

	if (scrollBar->currIndex != oldTopRow)
	{
		PopulateRows();
	}
}

void LeaderboardDisplay::PopulateRows()
{
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Clear();
	}

	int numEntries = manager.currBoard.entries.size();

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (i + scrollBar->currIndex == numEntries)
		{
			break;
		}

		rows[i].Set(manager.currBoard.entries[i + scrollBar->currIndex]);
	}
}

void LeaderboardDisplay::Update()
{
	if (action == A_HIDDEN)
		return;
	else if (action == A_LOADING)
	{
		if (manager.IsIdle())
		{
			action = A_SHOWING;
			frame = 0;

			int numEntries = manager.currBoard.entries.size();

			scrollBar->SetRows(numEntries, NUM_ROWS);
			scrollBar->SetIndex(0);

			/*if (numEntries <= NUM_ROWS)
			{
				maxTopRow = 0;
			}
			else
			{
				maxTopRow = numEntries - NUM_ROWS;
			}*/

			PopulateRows();
		}
	}
	else if (action == A_WAITING_FOR_REPLAY)
	{
		if (manager.currBoard.entries[chosenReplayIndex].IsReplayReady())
		{
			action = A_RUNNING_REPLAY;

			replayChosen = manager.currBoard.entries[chosenReplayIndex].playerReplayManager;

			return;
			//Hide();
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

		int scroll = UICONTROLLER.ConsumeScroll();
		if (scroll != 0)
		{
			MouseScroll(scroll);
		}
	}

	++frame;
}

int LeaderboardDisplay::GetNumActiveLeaderboardGhosts()
{
	return manager.GetNumActiveGhosts();
}

bool LeaderboardDisplay::IsTryingToStartReplay()
{
	return action == A_RUNNING_REPLAY;
}

bool LeaderboardDisplay::IsTryingToRaceGhosts()
{
	return action == A_RACING_GHOSTS;
}

void LeaderboardDisplay::AddGhostsToVec(std::vector<ReplayGhost*> &vec, PlayerReplayManager *ignore)
{
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn && (*it).playerReplayManager != NULL)
		{
			if ((*it).playerReplayManager == ignore)
				continue;

			(*it).playerReplayManager->AddGhostsToVec(vec);
			(*it).playerReplayManager->ghostsActive = true;
		}
	}
}

void LeaderboardDisplay::AddPlayerReplayManagersToVec(std::vector<PlayerReplayManager*> &vec, PlayerReplayManager *ignore)
{
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).playerReplayManager == ignore)
		{
			continue;
		}

		if ((*it).ghostOn && (*it).playerReplayManager != NULL)
		{
			vec.push_back((*it).playerReplayManager);
		}
	}
}

void LeaderboardDisplay::SetActive(bool replay, bool ghost, PlayerReplayManager *ignore)
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
	//clearCheckedGhostsButton->SetPos(above);
	//raceGhostsButton->SetPos(above + Vector2i(250, 0));
	//refreshBoardButton->SetPo

	if (b == clearCheckedGhostsButton)
	{
		manager.UncheckAllGhosts();
		for (int i = 0; i < NUM_ROWS; ++i)
		{
			rows[i].ghostCheckBox->checked = false;
			//rows[i].Clear();
		}
		//manager.currBoard.entries[trueIndex].ghostOn = cb->checked;
	}
	else if (b == raceGhostsButton)
	{
		if (GetNumActiveLeaderboardGhosts() > 0 || IsDefaultGhostOn() )
		{
			action = A_RACING_GHOSTS;
		}
	}
	else if (b == refreshBoardButton)
	{
		for (int i = 0; i < NUM_ROWS; ++i)
		{
			rows[i].Clear();
		}

		manager.RefreshCurrBoard();

		action = A_LOADING;
		frame = 0;
	}
	else
	{
		//watch buttons
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
			int trueIndex = scrollBar->currIndex + rowIndex;
			action = A_WAITING_FOR_REPLAY;
			manager.currBoard.entries[trueIndex].DownloadReplay();
			chosenReplayIndex = trueIndex;
			//cout << "watch replay: " << trueIndex << "\n";
		}
	}
}

bool LeaderboardDisplay::IsDefaultGhostOn()
{
	return originalGhostCheckBox->checked;
}

bool LeaderboardDisplay::ShouldShowGhostsWithReplay()
{
	return showGhostsWithReplayCheckBox->checked;
}

//showGhostsWithReplayCheckBox

void LeaderboardDisplay::CheckBoxCallback(CheckBox *cb, const std::string & e)
{
	if (cb == originalGhostCheckBox)
	{

	}
	else if (cb == showGhostsWithReplayCheckBox)
	{

	}
	else
	{
		//one of the row ghost boxes
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
			int trueIndex = scrollBar->currIndex + rowIndex;

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
	
}

void LeaderboardDisplay::ScrollBarCallback(ScrollBar *sb, const std::string &e)
{
	PopulateRows();
}