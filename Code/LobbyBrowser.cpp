#include "LobbyBrowser.h"
#include "NetplayManager.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"

using namespace sf;
using namespace std;


LobbyBrowser::LobbyBrowser()
{
	totalRects = 10;
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

	currSelectedRect = NULL;

	panel->ReserveLobbyRects(totalRects);
	for (int i = 0; i < totalRects; ++i)
	{
		lobbyRects[i] = panel->AddLobbyRect(Vector2f(startRects.x, startRects.y + (boxSize.y + spacing.y) * i),
			boxSize);
		lobbyRects[i]->SetShown(false);
		lobbyRects[i]->Init();
	}

	panel->AddButton("join", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "JOIN");
	panel->SetCancelButton(panel->AddButton("back", Vector2i(20 + 150, panel->size.y - 100), Vector2f(100, 40), "BACK"));
	
}

LobbyBrowser::~LobbyBrowser()
{
	delete[] lobbyRects;
	delete panel;
}

void LobbyBrowser::Update()
{
	if (panel->MouseUpdate())
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
			PopulateRects();
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
			ClearSelection();
			SetAction(A_GET_LOBBIES);
		}
		break;
	}
	case A_TRY_JOIN_LOBBY:
	{
		if (lobbyManager->action == LobbyManager::A_IN_LOBBY)
		{
			SetAction(A_IN_LOBBY);
		}
		else if (lobbyManager->action != LobbyManager::A_IN_LOBBY_WAITING_FOR_DATA
			&& lobbyManager->action != LobbyManager::A_REQUEST_JOIN_LOBBY)
		{
			if (lobbyManager->action == LobbyManager::A_IDLE)
			{
				cout << "failed to join lobby for some reason!: " << lobbyManager->action << endl;
				
				ClearSelection();

				SetAction(A_GET_LOBBIES);
			}
		}
		break;
	}
	case A_IN_LOBBY:
	{
		break;
	}
	}
	//panel->MouseUpdate();

	++frame;
}

void LobbyBrowser::ClearSelection()
{
	currSelectedRect = NULL;
	for (int i = 0; i < totalRects; ++i)
	{
		lobbyRects[i]->Deselect();
	}

}

void LobbyBrowser::MouseScroll(int delta)
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

void LobbyBrowser::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		ClosePopup();
	}
}

void LobbyBrowser::ChooseRectEvent(ChooseRect *cr, int eventType)
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
			currSelectedRect = (LobbyChooseRect*)cr;


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

void LobbyBrowser::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "join")
	{
		NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
		LobbyManager *lobbyManager = MainMenu::GetInstance()->netplayManager->lobbyManager;
		if (currSelectedRect != NULL)
		{
			int selectedIndex = (int)currSelectedRect->info;
			lobbyManager->TryJoiningLobby(selectedIndex);
			action = A_TRY_JOIN_LOBBY;
		}
		//ClosePopup();
	}
	else if (b->name == "back")
	{
		SetAction(A_RETURN_TO_MENU);
		//ClosePopup();
	}
}

void LobbyBrowser::PopulateRects()
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

		lcRect->SetInfo((void*)i);

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

void LobbyBrowser::OpenPopup()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	ClearSelection();

	

	SetAction(A_GET_LOBBIES);
}

void LobbyBrowser::ClosePopup()
{
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
	return panel->HandleEvent(ev);
}

void LobbyBrowser::Draw(sf::RenderTarget *target)
{
	//panel->Draw(target);
	//panel->Draw(target);
}