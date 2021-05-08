#include "MusicSelector.h"
#include "MainMenu.h"
#include "EditSession.h"

using namespace sf;
using namespace std;

ListChooserHandler::ListChooserHandler(int rows)
{
	chooser = new ListChooser(this, rows);
}

ListChooserHandler::~ListChooserHandler()
{
	delete chooser;
}


MusicChooserHandler::MusicChooserHandler(int rows)
	:ListChooserHandler( rows )
{
	grabbedText = chooser->textRects[0]->nameText;
	grabbedText.setString("");
}

MusicChooserHandler::~MusicChooserHandler()
{

}

void MusicChooserHandler::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		chooser->ClosePopup();	
	}
}

void MusicChooserHandler::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (cr->rectIdentity == ChooseRect::I_MUSICLIBRARY)
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
			ClickText(cr);
		}
	}
	else if (cr->rectIdentity == ChooseRect::I_MUSICLEVEL)//map
	{
		if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
		{
			//ts_largePreview = cr->GetAsImageChooseRect()->ts;
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
		{
			//ts_largePreview = NULL;
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
		{
			ClickText(cr);
			cr->SetName("");
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			if (state == DRAG)
			{
				cr->SetName(grabbedString);
				//grabbedString = "";
			}
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_RIGHTCLICKED)
		{
			/*FileNode *fn = (FileNode*)cr->info;

			chooser->TurnOff();
			chooser->edit->Reload(fn->filePath);*/
		}
	}
	else
	{
		assert(0);
	}
}

void MusicChooserHandler::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "ok")
	{
		int numSongsSelected = 0;
		for (int i = 0; i < chooser->numMyMusicRects; ++i)
		{
			if (chooser->myMusicRects[i]->nameText.getString() != "")
			{
				numSongsSelected++;
			}
		}

		string currString;
		MapHeader *mh = chooser->edit->mapHeader;
		if (numSongsSelected == 0)
		{
			mh->ClearSongs();
			chooser->edit->ClearMusic();
		}
		else// if (numSongsSelected == 1)
		{
			bool setSong = false;
			mh->ClearSongs();
			for (int i = 0; i < chooser->numMyMusicRects; ++i)
			{
				currString = chooser->myMusicRects[i]->nameText.getString();
				if ( currString != "")
				{
					mh->AddSong(currString, 100);

					if (!setSong)
					{
						chooser->edit->SetMusic(currString);
						setSong = true;
					}
				}
			}
		}

		chooser->ClosePopup();
	}
	else if (b->name == "cancel")
	{
		chooser->ClosePopup();
	}
}

bool MusicChooserHandler::MouseUpdate()
{
	if (MOUSE.IsMouseLeftReleased())
	{
		state = BROWSE;
		grabbedString = "";
	}

	if (state == DRAG)
	{
		grabbedText.setPosition(Vector2f(chooser->panel->GetMousePos()));
	}
	return true;
}

void MusicChooserHandler::ClickText(ChooseRect *cr)
{
	//FileNode *fn = (FileNode*)cr->info;

	grabbedString = cr->nameText.getString();
	grabbedText.setString(grabbedString);
	grabbedText.setOrigin(grabbedText.getLocalBounds().left
		+ grabbedText.getLocalBounds().width / 2,
		grabbedText.getLocalBounds().top
		+ grabbedText.getLocalBounds().height / 2);
	state = DRAG;
}

void MusicChooserHandler::Draw(sf::RenderTarget *target)
{

}

void MusicChooserHandler::LateDraw(sf::RenderTarget *target)
{
	if (state == DRAG)
	{
		target->draw(grabbedText);
	}
}

ListChooser::ListChooser( ListChooserHandler *p_handler, int rows )
	:handler( p_handler )
{
	edit = EditSession::GetSession();

	numMyMusicRects = 3;
	totalRects = rows;
	topRow = 0;
	maxTopRow = 0;
	numEntries = 10; //this should be the number of current entries

	textRects = new TextChooseRect*[totalRects];
	myMusicRects = new TextChooseRect*[numMyMusicRects];

	panel = new Panel("listchooser", 500, 500, handler, true);
	panel->SetPosition(Vector2i(300, 300));
	//panel->ReserveTextRects(totalRects);
	panel->extraUpdater = this;

	Vector2f startRects(10, 100);
	Vector2f spacing(60, 2);
	Vector2f boxSize(100, 30);
	float myMusicRectsXStart = startRects.x + boxSize.x + 100;

	auto &songMap = edit->mainMenu->musicManager->songMap;
	int numSongs = songMap.size();
	songNames.resize(numSongs);
	int ind = 0;
	for (auto it = songMap.begin(); it != songMap.end(); ++it)
	{
		songNames[ind] = (*it).first;
		++ind;
	}
	numEntries = numSongs;

	maxTopRow = numSongs - totalRects;
	if (maxTopRow < 0)
		maxTopRow = 0;
	

	panel->ReserveTextRects(totalRects + numMyMusicRects);
	for (int i = 0; i < totalRects; ++i)
	{
		textRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLIBRARY,
			Vector2f(startRects.x, startRects.y + (boxSize.y + spacing.y) * i),
			boxSize, "");
		textRects[i]->SetShown(true);
		textRects[i]->Init();
	}

	for (int i = 0; i < numMyMusicRects; ++i)
	{
		myMusicRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLEVEL,
			Vector2f(myMusicRectsXStart, startRects.y + (boxSize.y + spacing.y) * i),
			boxSize, "");
		myMusicRects[i]->SetShown(true);
		myMusicRects[i]->Init();
	}

	panel->SetConfirmButton(panel->AddButton("ok", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "OK"));
	panel->SetCancelButton(panel->AddButton("cancel", Vector2i(140, panel->size.y - 100), Vector2f(100, 40), "Cancel"));

	//
}

ListChooser::~ListChooser()
{
	delete[] textRects;
	delete[] myMusicRects;
	delete panel;
}

bool ListChooser::MouseUpdate()
{
	return handler->MouseUpdate();
}

void ListChooser::OpenPopup()
{
	PopulateRects();

	auto &songOrder = edit->mapHeader->songOrder;
	int songCounter = 0;
	for (auto it = songOrder.begin(); it != songOrder.end(); ++it)
	{
		myMusicRects[songCounter]->SetName((*it));

		++songCounter;
		if (songCounter == 3)
			break;
	}

	edit->AddActivePanel(panel);
}

void ListChooser::ClosePopup()
{
	edit->RemoveActivePanel(panel);
}

void ListChooser::Draw(sf::RenderTarget *target)
{
	handler->Draw(target);
}

void ListChooser::Deactivate()
{

}

void ListChooser::MouseScroll(int delta)
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

void ListChooser::LateDraw(sf::RenderTarget *target)
{
	handler->LateDraw(target);
}

void ListChooser::PopulateRects()
{
	TextChooseRect *tcRect;
	int start = topRow;

	int i;
	for (i = start; i < numEntries && i < start + totalRects; ++i)
	{
		tcRect = textRects[i - start];

		tcRect->SetText(songNames[i]);
		//node = nodes[i];
		//tcRect->SetName( )
		//tcRect->SetName(node->filePath.filename().stem().string());
		//tcRect->SetInfo(node);

		/*if (ts != NULL)
			icRect->SetImage(ts, ts->GetSubRect(0));
		else
		{
			icRect->SetImage(NULL, 0);
		}*/

		tcRect->SetShown(true);
	}

	for (; i < start + totalRects; ++i)
	{
		tcRect = textRects[i - start];
		tcRect->SetShown(false);
	}

	
}


MusicSelectorUI::MusicSelectorUI()
{
	edit = EditSession::GetSession();
	listHandler = new MusicChooserHandler(10);
	/*panel = new Panel("panel", 500, 500, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2,
		540 - panel->size.y / 2));*/
}

MusicSelectorUI::~MusicSelectorUI()
{
	delete listHandler;
}

void MusicSelectorUI::Draw(sf::RenderTarget *target)
{

	//panel->Draw(target);
}

void MusicSelectorUI::Update()
{
	//panel->MouseUpdate();
}

void MusicSelectorUI::OpenPopup()
{
	listHandler->chooser->OpenPopup();
	
	
	//edit->AddActivePanel(panel);
}

void MusicSelectorUI::ClosePopup()
{
	listHandler->chooser->ClosePopup();
	//edit->RemoveActivePanel(panel);
}

//void MusicSelectorUI::PanelCallback(Panel *p, const std::string & e)
//{
//	if (e == "leftclickoffpopup")
//	{
//		ClosePopup();
//		
//	}
//}