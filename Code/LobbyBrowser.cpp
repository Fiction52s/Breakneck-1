#include "LobbyBrowser.h"
#include "NetplayManager.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"

using namespace sf;
using namespace std;


LobbySelector::LobbySelector( int rows)
{
	totalRects = rows;
	topRow = 0;
	maxTopRow = 0;
	numEntries = 0;//10; //this should be the number of current entries

	lobbyRects = new LobbyChooseRect*[totalRects];

	panel = new Panel("listchooser", 1000, 500, this, true);
	panel->SetCenterPos(Vector2i(960, 540));
	//panel->ReserveTextRects(totalRects);
	panel->extraUpdater = this;

	Vector2f startRects(10, 80);
	Vector2f spacing(60, 2);
	Vector2f myRectSpacing(0, 30);
	Vector2f boxSize(300, 30);
	float myMusicRectsXStart = startRects.x + boxSize.x + 100;

	numEntries = 0;

	maxTopRow = numEntries - totalRects;
	if (maxTopRow < 0)
		maxTopRow = 0;


	panel->ReserveLobbyRects(totalRects);
	for (int i = 0; i < totalRects; ++i)
	{
		lobbyRects[i] = panel->AddLobbyRect(Vector2f(startRects.x, startRects.y + (boxSize.y + spacing.y) * i),
			boxSize);
		lobbyRects[i]->SetShown(false);
		lobbyRects[i]->Init();
	}

	panel->AddButton("join", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "JOIN");
	//panel->SetCancelButton(panel->AddButton("cancel", Vector2i(140, panel->size.y - 100), Vector2f(100, 40), "Cancel"));
}

LobbySelector::~LobbySelector()
{
	delete[] lobbyRects;
	delete panel;
}

void LobbySelector::OpenPopup()
{
	PopulateRects();

	/*auto &songOrder = mh->songOrder;
	auto &songLevels = mh->songLevels;
	int songCounter = 0;

	for (int i = 0; i < numMyMusicRects; ++i)
	{
		sliders[i]->HideMember();
	}

	for (auto it = songOrder.begin(); it != songOrder.end(); ++it)
	{
		myMusicRects[songCounter]->SetName((*it));

		sliders[songCounter]->SetCurrValue(songLevels[(*it)]);

		sliders[songCounter]->ShowMember();

		++songCounter;
		if (songCounter == 3)
			break;
	}*/

	//edit->AddActivePanel(panel);
}

void LobbySelector::ClosePopup()
{
	//edit->RemoveActivePanel(panel);
}

void LobbySelector::SetPlayingColor(const std::string &str)
{
	if (str == "")
	{
		return;
	}
	playingSongName = str;

	currPlayingRect = NULL;
	for (int i = 0; i < totalRects; ++i)
	{
		if (str == lobbyRects[i]->nameText.getString())
		{
			lobbyRects[i]->SetIdleColor(Color::Magenta);
			currPlayingRect = lobbyRects[i];
		}
		else
		{
			lobbyRects[i]->SetIdleColor(lobbyRects[i]->defaultIdleColor);
		}
	}
}

void LobbySelector::SetStoppedColor()
{
	playingSongName = "";
	currPlayingMyRect = NULL;

	currPlayingRect = NULL;
	for (int i = 0; i < totalRects; ++i)
	{
		lobbyRects[i]->SetIdleColor(lobbyRects[i]->defaultIdleColor);
	}
}

void LobbySelector::Draw(sf::RenderTarget *target)
{
	//panel->Draw(target);
}

void LobbySelector::MouseScroll(int delta)
{
	int oldTopRow = topRow;
	if (delta < 0)
	{
		topRow -= delta;
		if (topRow > maxTopRow)
			topRow = maxTopRow;
	}
	else if (delta > 0)
	{
		topRow -= delta;
		if (topRow < 0)
			topRow = 0;
	}

	if (topRow != oldTopRow)
	{
		PopulateRects();
	}
}

void LobbySelector::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		ClosePopup();
	}
}

void LobbySelector::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (cr->rectIdentity == ChooseRect::I_LOBBY)
	{
		string s = cr->nameText.getString();

		if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
		{

			/*if (node->type == FileNode::FILE)
			{
			FocusFile(cr);
			}*/
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
		{
			/*if (node->type == FileNode::FILE)
			{
			UnfocusFile(cr);
			}*/
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{

			for (int i = 0; i < totalRects; ++i)
			{
				if (lobbyRects[i] != cr)
				{
					lobbyRects[i]->Deselect();
				}
			}
			cr->Select();


			//ClickText(cr);
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_RIGHTCLICKED)
		{
			string crName = cr->nameText.getString();

			/*if (cr == chooser->currPlayingRect)
			{
			chooser->edit->StopMusic(chooser->edit->previewMusic);
			}
			else
			{
			chooser->edit->SetPreviewMusic(crName);
			chooser->edit->PlayMusic(chooser->edit->previewMusic);
			}*/
		}
	}
	else
	{
		assert(0);
	}
}

void LobbySelector::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "ok")
	{
		//int numSongsSelected = 0;
		//for (int i = 0; i < chooser->numMyMusicRects; ++i)
		//{
		//	if (chooser->myMusicRects[i]->nameText.getString() != "")
		//	{
		//		numSongsSelected++;
		//	}
		//}

		//string currString;
		//MapHeader *mh = chooser->mh;
		//if (numSongsSelected == 0)
		//{
		//	mh->ClearSongs();
		//	chooser->edit->CleanupMusic(chooser->edit->originalMusic);
		//}
		//else// if (numSongsSelected == 1)
		//{
		//	bool setSong = false;
		//	mh->ClearSongs();
		//	for (int i = 0; i < chooser->numMyMusicRects; ++i)
		//	{
		//		currString = chooser->myMusicRects[i]->nameText.getString();
		//		if (currString != "")
		//		{
		//			mh->AddSong(currString, chooser->sliders[i]->GetCurrValue());

		//			if (!setSong)
		//			{
		//				chooser->edit->SetOriginalMusic(currString);
		//				setSong = true;
		//			}
		//		}
		//	}
		//}

		ClosePopup();
	}
	else if (b->name == "cancel")
	{
		ClosePopup();
	}
}

void LobbySelector::PopulateRects()
{
	cout << "populating" << endl;
	LobbyChooseRect *lcRect;
	int start = topRow;

	int i;

	LobbyManager *lobbyManager = MainMenu::GetInstance()->netplayManager->lobbyManager;

	numEntries = lobbyManager->lobbyVec.size();
	maxTopRow = numEntries - totalRects;
	if (maxTopRow < 0)
		maxTopRow = 0;

	for (i = start; i < numEntries && i < start + totalRects; ++i)
	{
		lcRect = lobbyRects[i - start];

		lcRect->SetText(lobbyManager->lobbyVec[i].name);

		lcRect->SetShown(true);
	}

	for (; i < start + totalRects; ++i)
	{
		lcRect = lobbyRects[i - start];
		lcRect->SetShown(false);
	}

	//SetPlayingColor(playingSongName);
	//SetPlayingColor();

}

LobbyBrowser::LobbyBrowser()
{
	lobbySelector = new LobbySelector(10);

	/*panel = new Panel("panel", 500, 500, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2,
	540 - panel->size.y / 2));*/
}

LobbyBrowser::~LobbyBrowser()
{
	delete lobbySelector;
}

void LobbyBrowser::Draw(sf::RenderTarget *target)
{
	lobbySelector->panel->Draw(target);
	//panel->Draw(target);
}

void LobbyBrowser::Update()
{
	if (lobbySelector->panel->MouseUpdate())
	{
		
	}

	//lobbyChooserHandler->chooser->panel->UpdateSprites(spriteUpdateFrames);

	LobbyManager *lobbyManager = MainMenu::GetInstance()->netplayManager->lobbyManager;

	switch (action)
	{
	case A_GET_LOBBIES:
		if (lobbyManager->action == LobbyManager::A_FOUND_LOBBIES)
		{
			SetAction(A_IDLE);
			lobbySelector->PopulateRects();
		}
		else if (lobbyManager->action == LobbyManager::A_FOUND_NO_LOBBIES)
		{
			SetAction(A_AUTO_REFRESH_LOBBIES);
		}
		break;
	case A_IDLE:
		break;
	case A_AUTO_REFRESH_LOBBIES:
	{
		if (frame == 300)
		{
			SetAction(A_GET_LOBBIES);
		}
		break;
	}
	}
	//panel->MouseUpdate();

	++frame;
}


void LobbyBrowser::OpenPopup()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	netplayManager->Init();

	SetAction(A_GET_LOBBIES);
}

void LobbyBrowser::ClosePopup()
{
	lobbySelector->ClosePopup();
	//edit->RemoveActivePanel(panel);
}

void LobbyBrowser::SetAction(Action a)
{
	action = a;
	frame = 0;

	LobbyManager *lobbyManager = MainMenu::GetInstance()->netplayManager->lobbyManager;

	if (action == A_GET_LOBBIES)
	{
		lobbyManager->RefreshLobbyList();
	}
}

bool LobbyBrowser::HandleEvent(sf::Event ev)
{
	return lobbySelector->panel->HandleEvent(ev);
}