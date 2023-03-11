#include "Leaderboard.h"
#include <iostream>
#include "MainMenu.h"
#include "Session.h"
#include "UIMouse.h"
#include <fstream>
#include "PlayerRecord.h"
#include <sstream>
#include "UIController.h"
#include "GameSession.h"

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

KineticLeaderboardEntry::~KineticLeaderboardEntry()
{
	if (playerReplayManager != NULL)
	{
		delete playerReplayManager;
		playerReplayManager = NULL;
	}
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
	manager.display = this;

	panel = new Panel("leaderboard", 1920, 1080, this);
	
	panel->SetColor(Color::Transparent);

	leaderboardBGBorder = 50;

	int buttonHeight = LeaderboardDisplay::CHAR_HEIGHT;

	titleLabel = panel->AddLabel("titlelabel", Vector2i(), 40, "");
	titleLabel->text.setFillColor(Color::White);

	friendsOnlyCheckBox = panel->AddCheckBox("friendsonlycheckbox", Vector2i(), true);
	friendsOnlyLabel = panel->AddLabel("friendsonlylabel", Vector2i(), LeaderboardDisplay::CHAR_HEIGHT, "Friends Only: ");

	refreshBoardButton = panel->AddButton("refreshbutton", Vector2i(), Vector2f(100, buttonHeight), "Refresh");


	clearCheckedGhostsButton = panel->AddButton("clearbutton", Vector2i(), Vector2f(200, buttonHeight), "Clear Ghosts");
	raceGhostsButton = panel->AddButton("racebutton", Vector2i(), Vector2f(200, buttonHeight), "Race Ghosts");
	
	int ghostOptionCharHeight = LeaderboardDisplay::CHAR_HEIGHT - 2;

	originalGhostCheckBox = panel->AddCheckBox("originalghostcheckbox", Vector2i(), true);
	originalGhostCheckBoxLabel = panel->AddLabel("originalghostcheckboxlabel", Vector2i(), ghostOptionCharHeight, "Default ghost on:");

	showGhostsWithReplayCheckBox = panel->AddCheckBox("showghostswthreplaycheckbox", Vector2i(), false);
	showGhostsWithReplayLabel = panel->AddLabel("showghostswithreplaylabel", Vector2i(), ghostOptionCharHeight, "Show ghosts while watching replay:");

	ghostsUseOriginalSkinsCheckBox = panel->AddCheckBox("ghostsuseoriginalskinscheckbox", Vector2i(), false);
	ghostsUseOriginalSkinsLabel = panel->AddLabel("ghostsuseoriginalskinslabel", Vector2i(), ghostOptionCharHeight, "Ghosts use player skins:");

	scrollBar = panel->AddScrollBar("scroll", Vector2i(), Vector2i(30, NUM_ROWS * ROW_HEIGHT ), 1, 1);

	std::vector<string> tabStrings = { "Original Powers", "Any Powers" };

	int tabWidth = ROW_WIDTH / 2 - 40;
	int tabHeight = 45;

	tabs = panel->AddTabGroup("tabs", Vector2i(), tabStrings, tabWidth, tabHeight);//LeaderboardDisplay::CHAR_HEIGHT);
	tabs->SelectTab(0);

	evenColor = Color(200, 200, 200, 200);
	oddColor = Color(150, 150, 150, 200);
	myColor = Color::Green;

	int topRowCharHeight = LeaderboardDisplay::CHAR_HEIGHT - 4;
	topRowRankLabel = panel->AddLabel( "toprowranklabel", Vector2i(), topRowCharHeight, "Rank" );
	topRowNameLabel = panel->AddLabel("toprownamelabel", Vector2i(), topRowCharHeight, "User");
	topRowTimeLabel = panel->AddLabel("toprowtimelabel", Vector2i(), topRowCharHeight, "Time");
	topRowGhostLabel = panel->AddLabel("toprowghostlabel", Vector2i(), topRowCharHeight, "Ghost");
	topRowReplayLabel = panel->AddLabel("toprowreplaylabel", Vector2i(), topRowCharHeight, "Replay");

	SetRectColor(topRowQuad, Color( 120, 120, 200, 255 ));

	for (int i = 0; i < 5; ++i)
	{
		SetRectColor(dividerQuads + i * 4, Color(100, 100, 100));//Color::Black);
	}


	Hide();

	SetRectColor(titleQuad, Color(0, 0, 0, 255));
	SetRectColor(leaderboardBGQuad, Color(100, 100, 100, 150));

	SetRectColor(fullscreenBGQuad, Color(100, 100, 200, 150));
	SetRectTopLeft(fullscreenBGQuad, 1920, 1080, Vector2f(0, 0));

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Init(i, panel);
	}

	SetTopLeft(Vector2f(960 - ( ROW_WIDTH / 2 + leaderboardBGBorder ), 100));//540 - (NUM_ROWS / 2 * ROW_HEIGHT)));
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

	storedCheckedGhosts.clear();
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn)
		{
			storedCheckedGhosts.push_back((*it).steamEntry.m_steamIDUser);
		}
	}
}

void LeaderboardDisplay::Reset()
{
	action = A_HIDDEN;
	frame = 0;

	//some of these should reset w/ the map choice, some should remain for the session

	//friendsOnlyCheckBox->checked = true;
	originalGhostCheckBox->checked = true;
	showGhostsWithReplayCheckBox->checked = false;
	//ghostsUseOriginalSkinsCheckBox->checked = false;
	scrollBar->SetIndex(0);
	storedCheckedGhosts.clear();
	origPowersBoardName = "";
	anyPowersBoardName = "";
	tabs->SelectTab(0);

	manager.Reset();
}

void LeaderboardDisplay::Clear()
{
	scrollBar->SetIndex(0);
	//storedCheckedGhosts.clear();
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Clear();
	}
}

bool LeaderboardDisplay::IsAnyPowersMode()
{
	return tabs->currTabIndex == 1;
}

void LeaderboardDisplay::SetBoards( const std::string &leaderboardDisplayName, const std::string &origPowers, const std::string &anyPowers)
{
	titleLabel->text.setString(leaderboardDisplayName);
	titleLabel->SetCenterPosition(Vector2i(titleLabel->text.getPosition()));

	origPowersBoardName = origPowers;
	anyPowersBoardName = anyPowers;
}

void LeaderboardDisplay::SetAnyPowersMode(bool on)
{
	if (on)
	{
		tabs->SelectTab(1);

	}
	else
	{
		tabs->SelectTab(0);
	}
}

void LeaderboardDisplay::DownloadCurrBoard()
{
	Clear();
	tabWhenDownloadingBoard = tabs->currTabIndex;
	action = A_LOADING;
	frame = 0;

	if (tabs->currTabIndex == 0)
	{
		manager.DownloadBoard(origPowersBoardName);
	}
	else
	{
		manager.DownloadBoard(anyPowersBoardName);
	}
}

void LeaderboardDisplay::Start()
{
	//Reset();

	Show();
	DownloadCurrBoard();
}

void LeaderboardDisplay::SetTopLeft(const sf::Vector2f &p_pos)
{
	topLeft = p_pos;

	float titleHeight = 50;

	/*sf::Vertex fullscreenBGQuad[4];
	sf::Vertex leaderboardBGQuad[4];
	sf::Vertex titleQuad[4];*/

	int totalHeight = (ROW_HEIGHT * NUM_ROWS);
	int totalWidth = ROW_WIDTH + leaderboardBGBorder * 2;

	SetRectTopLeft(leaderboardBGQuad, ROW_WIDTH + leaderboardBGBorder * 2, totalHeight + 400, topLeft);

	titleLabel->SetCenterPosition(Vector2i( topLeft + Vector2f(totalWidth / 2, 0 )));

	SetRectCenter(titleQuad, 300, titleHeight, topLeft + Vector2f(totalWidth / 2, 0 ));

	Vector2f boardOptionsRowTopLeft = topLeft + Vector2f(0, titleHeight / 2 + 20);

	Vector2f boardOptionsRowTopCenter = boardOptionsRowTopLeft + Vector2f(totalWidth / 2, 0);

	friendsOnlyLabel->SetTopLeftPosition(Vector2i(boardOptionsRowTopCenter + Vector2f(-200, 0)));

	friendsOnlyCheckBox->SetPos(Vector2i(friendsOnlyLabel->GetTopRight().x, boardOptionsRowTopLeft.y));

	refreshBoardButton->SetPos(Vector2i(boardOptionsRowTopCenter + Vector2f( 100, 0 )));

	Vector2f tabTopLeft = boardOptionsRowTopLeft + Vector2f(leaderboardBGBorder, 50);

	tabs->SetPos(Vector2i(tabTopLeft));	
	
	Vector2f topRowPos = tabTopLeft + Vector2f(0, tabs->totalSize.y);

	SetRectTopLeft(topRowQuad, ROW_WIDTH, ROW_HEIGHT, topRowPos);

	Vector2f rowsStart = topRowPos + Vector2f(0, ROW_HEIGHT);

	Vector2f rowTopLeft;
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rowTopLeft = rowsStart + Vector2f(0, i * ROW_HEIGHT);
		SetRectTopLeft(rowQuads + i * 4, ROW_WIDTH, ROW_HEIGHT, rowTopLeft);
		rows[i].SetTopLeft(rowTopLeft);
	}

	float dividerWidth = 2;
	float dividerGap = 10;
	SetRectTopLeft(dividerQuads, dividerWidth, totalHeight + ROW_HEIGHT, topRowPos + Vector2f(rows[0].nameSpacing - dividerWidth - dividerGap, 0));
	SetRectTopLeft(dividerQuads + 4, dividerWidth, totalHeight + ROW_HEIGHT, topRowPos + Vector2f(rows[0].scoreSpacing - dividerWidth - dividerGap, 0));
	SetRectTopLeft(dividerQuads + 8, dividerWidth, totalHeight + ROW_HEIGHT, topRowPos + Vector2f(rows[0].ghostSpacing - dividerWidth - dividerGap, 0));
	SetRectTopLeft(dividerQuads + 12, dividerWidth, totalHeight + ROW_HEIGHT, topRowPos + Vector2f(rows[0].watchSpacing - dividerWidth - dividerGap, 0));

	SetRectTopLeft(dividerQuads + 16, ROW_WIDTH, dividerWidth, topRowPos + Vector2f(0, ROW_HEIGHT - dividerWidth / 2));

	float nameSpacing = rows[0].nameSpacing - dividerGap;
	float scoreSpacing = rows[0].scoreSpacing - dividerGap;
	float ghostSpacing = rows[0].ghostSpacing - dividerGap;
	float replaySpacing = rows[0].watchSpacing - dividerGap;

	float rankCenter = (nameSpacing) / 2.f;
	float nameCenter = (nameSpacing + scoreSpacing) / 2.f;
	float scoreCenter = (scoreSpacing + ghostSpacing) / 2.f;
	float ghostCenter = (ghostSpacing + replaySpacing) / 2.f;
	float replayCenter = (replaySpacing + ROW_WIDTH) / 2.f;

	float rowMiddle = ROW_HEIGHT / 2;

	topRowRankLabel->SetCenterPosition(Vector2i(topRowPos + Vector2f(rankCenter, rowMiddle)));
	topRowNameLabel->SetCenterPosition(Vector2i(topRowPos + Vector2f(nameCenter, rowMiddle)));
	topRowTimeLabel->SetCenterPosition(Vector2i(topRowPos + Vector2f(scoreCenter, rowMiddle)));
	topRowGhostLabel->SetCenterPosition(Vector2i(topRowPos + Vector2f(ghostCenter, rowMiddle)));
	topRowReplayLabel->SetCenterPosition(Vector2i(topRowPos + Vector2f(replayCenter, rowMiddle)));

	scrollBar->SetPos(Vector2i(rowsStart + Vector2f(ROW_WIDTH, 0)));

	Vector2f bottomStart = rowsStart + Vector2f(0, ROW_HEIGHT * NUM_ROWS + 20);

	Vector2i bottomStartI(bottomStart);
	int textGap = 40;

	originalGhostCheckBoxLabel->SetTopLeftPosition(bottomStartI);
	originalGhostCheckBox->SetPos(Vector2i(originalGhostCheckBoxLabel->GetTopRight().x + 10, originalGhostCheckBoxLabel->GetTopRight().y));

	showGhostsWithReplayLabel->SetTopLeftPosition(bottomStartI + Vector2i(0, textGap));
	showGhostsWithReplayCheckBox->SetPos(Vector2i(showGhostsWithReplayLabel->GetTopRight().x + 10, showGhostsWithReplayLabel->GetTopRight().y));

	ghostsUseOriginalSkinsLabel->SetTopLeftPosition(bottomStartI + Vector2i(0, textGap * 2));
	ghostsUseOriginalSkinsCheckBox->SetPos(Vector2i(ghostsUseOriginalSkinsLabel->GetTopRight().x + 10, ghostsUseOriginalSkinsLabel->GetTopRight().y));

	//Vector2i bottomRightColumnStartI = bottomStartI + Vector2i(300, 0);

	raceGhostsButton->SetPos(bottomStartI + Vector2i(rows[0].ghostSpacing, 10));
	clearCheckedGhostsButton->SetPos(bottomStartI + Vector2i(rows[0].ghostSpacing, textGap * 2 + 10));
	

	

	//SetRectTopLeft()

	

	//ghostsUseOriginalSkinsCheckBox;
	//ghostsUseOriginalSkinsLabel;

	
	/*showGhostsWithReplayLabel = panel->AddLabel("showghostswithreplaylabel", Vector2i(), LeaderboardDisplay::CHAR_HEIGHT, "Show ghosts with replay:");

	friendsOnlyCheckBox = panel->AddCheckBox("friendsonlycheckbox", Vector2i(), false);
	friendsOnlyLabel = panel->AddLabel("friendsonlylabel", Vector2i(), LeaderboardDisplay::CHAR_HEIGHT, "Friends Only: ");*/
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

	int myIndex = -1;

	int currIndex;
	CSteamID myId = SteamUser()->GetSteamID();

	for (int i = 0; i < NUM_ROWS; ++i)
	{
		currIndex = i + scrollBar->currIndex;
		if (currIndex == numEntries)
		{
			break;
		}

		rows[i].Set(manager.currBoard.entries[currIndex]);

		if (manager.currBoard.entries[currIndex].steamEntry.m_steamIDUser == myId)
		{
			myIndex = i;
		}
	}

	
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		if (myIndex >= 0 && i == myIndex)
		{
			SetRectColor(rowQuads + i * 4, myColor);
		}
		else
		{
			if (i % 2 == 0)
			{
				SetRectColor(rowQuads + i * 4, evenColor);
			}
			else
			{
				SetRectColor(rowQuads + i * 4, oddColor);
			}
		}
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

			//shouldn't really happen
			if (numEntries > 0)
			{
				for (auto it = storedCheckedGhosts.begin(); it != storedCheckedGhosts.end(); ++it)
				{
					for (auto mit = manager.currBoard.entries.begin(); mit != manager.currBoard.entries.end(); ++mit)
					{
						if ((*it) == (*mit).steamEntry.m_steamIDUser)
						{
							(*mit).ghostOn = true;
							(*mit).DownloadReplay();
							break;
						}
					}
				}

				scrollBar->SetRows(numEntries, NUM_ROWS);


				assert(manager.myEntryIndex != -1);

				int myIndexPosition = 4;
				if (manager.myEntryIndex < myIndexPosition)
				{
					scrollBar->SetIndex(0);
				}
				else
				{
					scrollBar->SetIndex(manager.myEntryIndex - myIndexPosition);
				}

				//if( manager.currBoard.entries)

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

void LeaderboardDisplay::OnManagerUploadingScoreFailed()
{
	uploadState = UPLOAD_STATE_NONE;
	Session *sess = Session::GetSession();
	if (sess != NULL)
	{
		sess->StartAlertBox("Score failed to upload. Try again later.");
	}
}

void LeaderboardDisplay::OnManagerUploadingScoreSucceeded()
{
	if (uploadState == UPLOAD_STATE_ANY_POWERS && uploadType == UPLOAD_TYPE_BOTH_BOARDS)
	{
		successfulScoreChange = true;
		uploadState = UPLOAD_STATE_ORIG_POWERS;
		manager.UploadScore(origPowersBoardName, manager.scoreToUpload, manager.localReplayPath);
		cout << "completed the upload to the any powers board. Still need to upload to the orig power board\n";
	}
	else
	{	
		Session *sess = Session::GetSession();
		if (sess != NULL)
		{
			sess->StartAlertBox("Leaderboard score(s) updated successfully.");
		}

		uploadState = UPLOAD_STATE_NONE;
		cout << "full leaderboard submission complete and successful\n";
	}
}

void LeaderboardDisplay::OnManagerScoreWasNotGoodEnoughToUpload()
{
	if (uploadState == UPLOAD_STATE_ANY_POWERS && uploadType == UPLOAD_TYPE_BOTH_BOARDS)
	{
		uploadState = UPLOAD_STATE_ORIG_POWERS;
		manager.UploadScore(origPowersBoardName, manager.scoreToUpload, manager.localReplayPath);
		cout << "upload to the any powers board was unncessary due to score. Still need to try upload to the orig power board\n";
	}
	else
	{
		if (uploadType == UPLOAD_TYPE_BOTH_BOARDS && uploadState == UPLOAD_STATE_ORIG_POWERS && successfulScoreChange)
		{
			Session *sess = Session::GetSession();
			if (sess != NULL)
			{
				sess->StartAlertBox("Leaderboard score updated successfully.");
			}

			cout << "full leaderboard submission complete and successful\n";
		}

		uploadState = UPLOAD_STATE_NONE;
	}
}

void LeaderboardDisplay::UploadScore(int score, const std::string &replayPath, bool origCompatible)
{
	successfulScoreChange = false;
	if (origCompatible)
	{
		uploadType = UPLOAD_TYPE_BOTH_BOARDS;
	}
	else
	{
		uploadType = UPLOAD_TYPE_ANY_POWERS;
	}

	uploadState = UPLOAD_STATE_ANY_POWERS;
	manager.UploadScore(anyPowersBoardName, score, replayPath);
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

bool LeaderboardDisplay::IsUsingPlayerGhostSkins()
{
	return ghostsUseOriginalSkinsCheckBox->checked;
}

void LeaderboardDisplay::AddGhostsToVec(std::vector<ReplayGhost*> &vec, PlayerReplayManager *ignore)
{
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn && (*it).playerReplayManager != NULL)
		{
			if ((*it).playerReplayManager == ignore)
				continue;

			(*it).playerReplayManager->AddGhostsToVec(vec, IsUsingPlayerGhostSkins());
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
		if ((*it).playerReplayManager == ignore)
			continue;

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

	target->draw(fullscreenBGQuad, 4, sf::Quads);
	target->draw(leaderboardBGQuad, 4, sf::Quads);
	target->draw(titleQuad, 4, sf::Quads);
	target->draw(topRowQuad, 4, sf::Quads);

	target->draw(rowQuads, NUM_ROWS * 4, sf::Quads);


	if (action == A_SHOWING)
	{
		for (int i = 0; i < NUM_ROWS; ++i)
		{
			rows[i].Draw(target);
		}

		target->draw(dividerQuads, 5 * 4, sf::Quads);

		panel->Draw(target);
	}

	if (action == A_UPLOAD_FAILED_POPUP)
	{
		messagePop.Draw(target);
	}
}

void LeaderboardDisplay::RefreshCurrBoard()
{
	for (int i = 0; i < NUM_ROWS; ++i)
	{
		rows[i].Clear();
	}

	storedCheckedGhosts.clear();
	for (auto it = manager.currBoard.entries.begin(); it != manager.currBoard.entries.end(); ++it)
	{
		if ((*it).ghostOn)
		{
			storedCheckedGhosts.push_back((*it).steamEntry.m_steamIDUser);
		}
	}

	action = A_LOADING;
	frame = 0;

	manager.RefreshCurrBoard();
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
		RefreshCurrBoard();
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

bool LeaderboardDisplay::IsFriendsOnlyMode()
{
	return friendsOnlyCheckBox->checked;
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
	else if (cb == friendsOnlyCheckBox)
	{
		RefreshCurrBoard();
		if (cb->checked)
		{
			
		}
		else
		{

		}
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

void LeaderboardDisplay::TabGroupCallback(TabGroup *tg, const std::string &e)
{
	if (tg->currTabIndex != tabWhenDownloadingBoard)
	{
		/*if (tg->currTabIndex == 0)
		{
			for (int i = 0; i < NUM_ROWS; ++i)
			{
				rows[i].Clear();
			}
		}*/
		storedCheckedGhosts.clear();
		DownloadCurrBoard();
	}
	
}

