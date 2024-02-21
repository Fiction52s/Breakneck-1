#include "LobbyBrowser.h"
#include "NetplayManager.h"
#include <assert.h>
#include <iostream>
#include "MainMenu.h"

using namespace sf;
using namespace std;


LobbyBrowser::LobbyBrowser()
{
	totalRects = 20;
	topRow = 0;
	maxTopRow = 0;
	numEntries = 0;//10; //this should be the number of current entries

	lobbyRects = new LobbyChooseRect*[totalRects];

	lobbyBars = new Vertex[totalRects * 4];

	


	panel = new Panel("listchooser", 500, 1000, this, true);
	panel->SetCenterPos(Vector2i(960, 540));
	panel->SetColor(Color::Transparent);

	bgPanel = new Panel("bg", 500, 1000, this, true);
	bgPanel->SetCenterPos(Vector2i(960, 540));
	


	Label *title = panel->AddLabel("titlelabel", Vector2i(panel->size.x / 2, 50), 30, "Lobby Browser");
	auto lb = title->text.getLocalBounds();
	title->text.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);

	//panel->ReserveTextRects(totalRects);
	panel->extraUpdater = this;



	Vector2f startRects(10, 150);
	Vector2f spacing(60, 2);
	Vector2f myRectSpacing(0, 30);
	Vector2f boxSize(400, 30);
	float myMusicRectsXStart = startRects.x + boxSize.x + 100;

	numEntries = 0;

	maxTopRow = numEntries - totalRects;
	if (maxTopRow < 0)
		maxTopRow = 0;

	currSelectedRect = NULL;

	panel->ReserveLobbyRects(totalRects);
	Vector2f rectPos;
	Color evenColor(100, 100, 100);
	Color oddcolor(200, 200, 200);
	for (int i = 0; i < totalRects; ++i)
	{
		rectPos = Vector2f(startRects.x, startRects.y + (boxSize.y + spacing.y) * i);
		lobbyRects[i] = panel->AddLobbyRect(rectPos,boxSize);
		lobbyRects[i]->SetShown(false);
		lobbyRects[i]->Init();

		SetRectTopLeft(lobbyBars + i * 4, boxSize.x, boxSize.y, rectPos + Vector2f(panel->pos));
		if (i % 2 == 0)
		{
			SetRectColor(lobbyBars + i * 4, evenColor);
		}
		else
		{
			SetRectColor(lobbyBars + i * 4, oddcolor);
		}
	}

	joinButton = panel->AddButton("join", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "JOIN");
	panel->SetCancelButton(panel->AddButton("back", Vector2i(20 + 150, panel->size.y - 100), Vector2f(100, 40), "BACK"));
	
}

LobbyBrowser::~LobbyBrowser()
{
	delete[] lobbyRects;
	delete panel;
	delete bgPanel;

	delete[]lobbyBars;
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
			&& lobbyManager->action != LobbyManager::A_REQUEST_JOIN_LOBBY
			&& lobbyManager->action != LobbyManager::A_IN_LOBBY_WAITING_FOR_DATA)
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
	joinButton->HideMember();
}

void LobbyBrowser::TryJoinLobbyFromInvite(CSteamID id)
{
	LobbyManager *lobbyManager = MainMenu::GetInstance()->netplayManager->lobbyManager;
	lobbyManager->TryJoiningLobbyFromInvite(id);
	action = A_TRY_JOIN_LOBBY;
	joinButton->HideMember();
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

void LobbyBrowser::CancelCallback(Panel *p)
{
	SetAction(A_RETURN_TO_MENU);
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
			joinButton->ShowMember();

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
			joinButton->HideMember();
		}
		//ClosePopup();
	}
	else if (b->name == "back")
	{
		SetAction(A_RETURN_TO_MENU);
		//ClosePopup();
	}
}

void LobbyBrowser::ClearLobbyRects()
{
	for (int i = 0; i < totalRects; ++i)
	{
		lobbyRects[i]->SetShown(false);
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

		lcRect->SetText(lobbyManager->lobbyVec[i].data.lobbyName);

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

	joinButton->ShowMember();

	ClearSelection();
	ClearLobbyRects();

	

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
		lobbyManager->RetrieveLobbyList(LobbyData::LOBBYTYPE_CUSTOM, LobbyManager::SearchType::SEARCH_GET_ALL_OF_TYPE);
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

void LobbyBrowser::SpecialDraw(sf::RenderTarget *target)
{
	bgPanel->Draw(target);
	target->draw(lobbyBars, 4 * totalRects, sf::Quads);
	panel->Draw(target);
}