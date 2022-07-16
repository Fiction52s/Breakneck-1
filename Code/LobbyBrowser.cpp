#include "LobbyBrowser.h"
#include "LobbyManager.h"
#include <assert.h>
#include <iostream>

using namespace sf;
using namespace std;

LobbyListChooserHandler::LobbyListChooserHandler(int rows)
{
	chooser = new LobbyListChooser(this, rows);
}

LobbyListChooserHandler::~LobbyListChooserHandler()
{
	delete chooser;
}

LobbyChooserHandler::LobbyChooserHandler(int rows)
	:LobbyListChooserHandler(rows)
{
}

LobbyChooserHandler::~LobbyChooserHandler()
{

}

void LobbyChooserHandler::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		chooser->ClosePopup();
	}
}

void LobbyChooserHandler::ChooseRectEvent(ChooseRect *cr, int eventType)
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

			for (int i = 0; i < chooser->totalRects; ++i)
			{
				if (chooser->lobbyRects[i] != cr)
				{
					chooser->lobbyRects[i]->Deselect();
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

void LobbyChooserHandler::ButtonCallback(Button *b, const std::string & e)
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

		chooser->ClosePopup();
	}
	else if (b->name == "cancel")
	{
		chooser->ClosePopup();
	}
}

bool LobbyChooserHandler::MouseUpdate()
{
	return true;
}

void LobbyChooserHandler::ClickText(ChooseRect *cr)
{
	//FileNode *fn = (FileNode*)cr->info;

	//grabbedString = cr->nameText.getString();
	//grabbedText.setString(grabbedString);
	//grabbedText.setOrigin(grabbedText.getLocalBounds().left
	//	+ grabbedText.getLocalBounds().width / 2,
	//	grabbedText.getLocalBounds().top
	//	+ grabbedText.getLocalBounds().height / 2);
	//state = DRAG;

	//chooser->HideSlider(grabbedString);
}

void LobbyChooserHandler::Draw(sf::RenderTarget *target)
{

}

void LobbyChooserHandler::LateDraw(sf::RenderTarget *target)
{
	//if (state == DRAG)
	//{
	//	target->draw(grabbedText);
	//}
}

LobbyListChooser::LobbyListChooser(LobbyListChooserHandler *p_handler, int rows)
	:handler(p_handler)
{
	lobbyManager = new LobbyManager;

	totalRects = rows;
	topRow = 0;
	maxTopRow = 0;
	numEntries = 0;//10; //this should be the number of current entries

	lobbyRects = new LobbyChooseRect*[totalRects];

	panel = new Panel("listchooser", 1000, 500, handler, true);
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

LobbyListChooser::~LobbyListChooser()
{
	delete lobbyManager;

	delete[] lobbyRects;
	delete panel;
}

bool LobbyListChooser::MouseUpdate()
{
	return handler->MouseUpdate();
}

void LobbyListChooser::OpenPopup()
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

void LobbyListChooser::ClosePopup()
{
	//edit->RemoveActivePanel(panel);
}

void LobbyListChooser::SetPlayingColor(const std::string &str)
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

void LobbyListChooser::SetStoppedColor()
{
	playingSongName = "";
	currPlayingMyRect = NULL;

	currPlayingRect = NULL;
	for (int i = 0; i < totalRects; ++i)
	{
		lobbyRects[i]->SetIdleColor(lobbyRects[i]->defaultIdleColor);
	}
}

void LobbyListChooser::Draw(sf::RenderTarget *target)
{
	handler->Draw(target);
}

void LobbyListChooser::Deactivate()
{

}

void LobbyListChooser::MouseScroll(int delta)
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

void LobbyListChooser::LateDraw(sf::RenderTarget *target)
{
	handler->LateDraw(target);
}

void LobbyListChooser::PopulateRects()
{
	cout << "populating" << endl;
	LobbyChooseRect *lcRect;
	int start = topRow;

	int i;

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
	lobbyChooserHandler = new LobbyChooserHandler(10);

	/*panel = new Panel("panel", 500, 500, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2,
	540 - panel->size.y / 2));*/
}

LobbyBrowser::~LobbyBrowser()
{
	delete lobbyChooserHandler;
}

void LobbyBrowser::Draw(sf::RenderTarget *target)
{
	lobbyChooserHandler->chooser->panel->Draw(target);
	//panel->Draw(target);
}

void LobbyBrowser::Update()
{
	if (lobbyChooserHandler->chooser->panel->MouseUpdate())
	{
		
	}

	//lobbyChooserHandler->chooser->panel->UpdateSprites(spriteUpdateFrames);


	switch (action)
	{
	case A_GET_LOBBIES:
		if (lobbyChooserHandler->chooser->lobbyManager->action == LobbyManager::A_FOUND_LOBBIES)
		{
			action = A_IDLE;
			lobbyChooserHandler->chooser->PopulateRects();
			
		}
		else if (lobbyChooserHandler->chooser->lobbyManager->action == LobbyManager::A_FOUND_NO_LOBBIES)
		{

		}
		break;
	case A_IDLE:
		break;
	}
	//panel->MouseUpdate();
}


void LobbyBrowser::OpenPopup()
{
	//lobbyChooserHandler->chooser->OpenPopup();
	lobbyChooserHandler->chooser->lobbyManager->RefreshLobbyList();

	action = A_GET_LOBBIES;
	//edit->AddActivePanel(panel);
}

void LobbyBrowser::ClosePopup()
{
	lobbyChooserHandler->chooser->ClosePopup();
	//edit->RemoveActivePanel(panel);
}