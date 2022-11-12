#include "MusicSelector.h"
#include "MainMenu.h"
#include "EditSession.h"
#include "UIMouse.h"

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

void ListChooserHandler::SetHeader(MapHeader *mh)
{
	chooser->mh = mh;
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

void MusicChooserHandler::DropdownCallback(Dropdown *dropdown, const std::string & e)
{
	chooser->topRow = 0;
	chooser->numEntries = chooser->songNames[dropdown->GetSelectedText()].size();
	chooser->maxTopRow = chooser->numEntries - chooser->totalRects;
	if (chooser->maxTopRow < 0)
		chooser->maxTopRow = 0;

	chooser->PopulateRects();
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
		else if (eventType == ChooseRect::ChooseRectEventType::E_RIGHTCLICKED)
		{
			string crName = cr->nameText.getString();
			
			if (cr == chooser->currPlayingRect)
			{
				chooser->edit->StopMusic(chooser->edit->previewMusic);
			}
			else
			{
				chooser->edit->SetPreviewMusic(crName);
				chooser->edit->PlayMusic(chooser->edit->previewMusic);
			}
			//cr->SetIdleColor(Color::Magenta);

			//chooser->SetPlayingColor(crName);
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
			chooser->SetStoppedColorMyRects();
			
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTRELEASED)
		{
			if (state == DRAG)
			{
				cr->SetName(grabbedString);
				chooser->ResetSlider(grabbedString);
				if (chooser->playingSongName != "")
				{
					chooser->SetPlayingColorMyRects(chooser->playingSongName);
				}
				
				//grabbedString = "";
			}
		}
		else if (eventType == ChooseRect::ChooseRectEventType::E_RIGHTCLICKED)
		{
			string crName = cr->nameText.getString();

			if (cr == chooser->currPlayingMyRect)
			{
				chooser->edit->StopMusic(chooser->edit->previewMusic);
			}
			else
			{
				chooser->edit->SetPreviewMusic(crName);
				chooser->edit->PlayMusic(chooser->edit->previewMusic);
			}

			//chooser->SetPlayingColor(crName);
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
		MapHeader *mh = chooser->mh;
		if (numSongsSelected == 0)
		{
			mh->ClearSongs();
			chooser->edit->CleanupMusic( chooser->edit->originalMusic );
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
					mh->AddSong(currString, chooser->sliders[i]->GetCurrValue());

					if (!setSong)
					{
						chooser->edit->SetOriginalMusic(currString);
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

	chooser->HideSlider(grabbedString);
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

ListChooser::ListChooser( ListChooserHandler *p_handler, int rows)
	:handler( p_handler ), mh( NULL )
{
	edit = EditSession::GetSession();

	numMyMusicRects = 3;
	totalRects = rows;
	topRow = 0;
	maxTopRow = 0;
	numEntries = 10; //this should be the number of current entries

	textRects = new TextChooseRect*[totalRects];
	myMusicRects = new TextChooseRect*[numMyMusicRects];

	panel = new Panel("listchooser", 1000, 500, handler, true);
	panel->SetCenterPos(Vector2i(960, 540));
	//panel->ReserveTextRects(totalRects);
	panel->extraUpdater = this;

	Vector2f startRects(10, 80);
	Vector2f spacing(60, 2);
	Vector2f myRectSpacing(0, 30);
	Vector2f boxSize(300, 30);
	float myMusicRectsXStart = startRects.x + boxSize.x + 100;

	auto &songMap = edit->mainMenu->musicManager->songMap;
	int numSongs = songMap.size();

	std::vector<string> worldOptions = { "W1", "W2", "W3", "W4", "W5", "W6", "W7", "W8" };

	for (auto it = worldOptions.begin(); it != worldOptions.end(); ++it)
	{
		for (auto sit = songMap.begin(); sit != songMap.end(); ++sit)
		{
			if ((*sit).first[1] == (*it)[1])
			{
				songNames[(*it)].push_back((*sit).first);
			}
		}
	}
	
	numEntries = songNames["W1"].size();

	maxTopRow = numEntries - totalRects;
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

	Vector2f currRectPos;
	for (int i = 0; i < numMyMusicRects; ++i)
	{
		currRectPos = Vector2f(myMusicRectsXStart, startRects.y + (boxSize.y + myRectSpacing.y) * i);
		myMusicRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLEVEL,
			currRectPos, boxSize, "");

		sliders[i] = panel->AddSlider("slider" + to_string(i),
			Vector2i(currRectPos.x + boxSize.x + 20, currRectPos.y ), 200, 0, 100, 100);
		myMusicRects[i]->SetShown(true);
		myMusicRects[i]->Init();
	}

	playOriginalCheckbox = panel->AddCheckBox("playoriginal", Vector2i(400, 300), false);

	
	worldDropdown = panel->AddDropdown("worlddropdown", Vector2i(10, 10), Vector2i(100, 30),
		worldOptions, 0);
	

	panel->SetConfirmButton(panel->AddButton("ok", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "OK"));
	panel->SetCancelButton(panel->AddButton("cancel", Vector2i(140, panel->size.y - 100), Vector2f(100, 40), "Cancel"));
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

	auto &songOrder = mh->songOrder;
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
	}

	edit->AddActivePanel(panel);
}

void ListChooser::ClosePopup()
{
	edit->RemoveActivePanel(panel);
}

void ListChooser::ResetSlider(const std::string &str)
{
	for (int i = 0; i < numMyMusicRects; ++i)
	{
		if (myMusicRects[i]->nameText.getString() == str)
		{
			sliders[i]->SetCurrValue(sliders[i]->defaultValue);
			sliders[i]->ShowMember();
		}
	}
}

void ListChooser::HideSlider(const std::string &str)
{
	for (int i = 0; i < numMyMusicRects; ++i)
	{
		if (myMusicRects[i]->nameText.getString() == str)
		{
			sliders[i]->HideMember();
		}
	}
}

void ListChooser::SetPlayingColorMyRects(const std::string &str)
{
	currPlayingMyRect = NULL;
	for (int i = 0; i < numMyMusicRects; ++i)
	{
		if (str == myMusicRects[i]->nameText.getString())
		{
			myMusicRects[i]->SetIdleColor(Color::Magenta);
			currPlayingMyRect = myMusicRects[i];
		}
		else
		{
			myMusicRects[i]->SetIdleColor(myMusicRects[i]->defaultIdleColor);
		}
	}
}

void ListChooser::SetPlayingColor(const std::string &str)
{
	if (str == "")
	{
		return;
	}
	playingSongName = str;
	
	SetPlayingColorMyRects(str);

	currPlayingRect = NULL;
	for (int i = 0; i < totalRects; ++i)
	{
		if (str == textRects[i]->nameText.getString())
		{
			textRects[i]->SetIdleColor(Color::Magenta);
			currPlayingRect = textRects[i];
		}
		else
		{
			textRects[i]->SetIdleColor(textRects[i]->defaultIdleColor);
		}
	}
}

void ListChooser::SetStoppedColor()
{
	playingSongName = "";
	currPlayingMyRect = NULL;
	SetStoppedColorMyRects();

	currPlayingRect = NULL;
	for (int i = 0; i < totalRects; ++i)
	{
		textRects[i]->SetIdleColor(textRects[i]->defaultIdleColor);
	}
}

void ListChooser::SetStoppedColorMyRects()
{
	currPlayingMyRect = NULL;
	for (int i = 0; i < numMyMusicRects; ++i)
	{
		myMusicRects[i]->SetIdleColor(myMusicRects[i]->defaultIdleColor);
	}
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

	
	auto &currSongNames = songNames[worldDropdown->GetSelectedText()];
	for (i = start; i < numEntries && i < start + totalRects; ++i)
	{
		tcRect = textRects[i - start];

		tcRect->SetText(currSongNames[i]);
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

	SetPlayingColor(playingSongName);
	//SetPlayingColor();
	
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

bool MusicSelectorUI::ShouldPlayOriginal()
{
	return listHandler->chooser->playOriginalCheckbox->checked;
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



//AdventureMusicListChooser::AdventureMusicListChooser(ListChooserHandler *p_handler, int rows,
//	int p_numMyMusicRects, TextChooseRect **musicRects)
//	:handler(p_handler), mh(NULL)
//{
//	edit = EditSession::GetSession();
//
//	numMyMusicRects = p_numMyMusicRects;
//	myMusicRects = musicRects;
//	totalRects = rows;
//	topRow = 0;
//	maxTopRow = 0;
//	numEntries = 10; //this should be the number of current entries
//
//	textRects = new TextChooseRect*[totalRects];
//
//	panel = new Panel("listchooser", 1000, 500, handler, true);
//	panel->SetCenterPos(Vector2i(960, 540));
//	//panel->ReserveTextRects(totalRects);
//	panel->extraUpdater = this;
//
//	Vector2f startRects(10, 80);
//	Vector2f spacing(60, 2);
//	Vector2f myRectSpacing(0, 30);
//	Vector2f boxSize(300, 30);
//	float myMusicRectsXStart = startRects.x + boxSize.x + 100;
//
//	auto &songMap = edit->mainMenu->musicManager->songMap;
//	int numSongs = songMap.size();
//
//	std::vector<string> worldOptions = { "W1", "W2", "W3", "W4", "W5", "W6", "W7", "W8" };
//
//	for (auto it = worldOptions.begin(); it != worldOptions.end(); ++it)
//	{
//		for (auto sit = songMap.begin(); sit != songMap.end(); ++sit)
//		{
//			if ((*sit).first[1] == (*it)[1])
//			{
//				songNames[(*it)].push_back((*sit).first);
//			}
//		}
//	}
//
//	numEntries = songNames["W1"].size();
//
//	maxTopRow = numEntries - totalRects;
//	if (maxTopRow < 0)
//		maxTopRow = 0;
//
//
//	panel->ReserveTextRects(totalRects + numMyMusicRects);
//	for (int i = 0; i < totalRects; ++i)
//	{
//		textRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLIBRARY,
//			Vector2f(startRects.x, startRects.y + (boxSize.y + spacing.y) * i),
//			boxSize, "");
//		textRects[i]->SetShown(true);
//		textRects[i]->Init();
//	}
//
//	Vector2f currRectPos;
//	for (int i = 0; i < numMyMusicRects; ++i)
//	{
//		currRectPos = Vector2f(myMusicRectsXStart, startRects.y + (boxSize.y + myRectSpacing.y) * i);
//		myMusicRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLEVEL,
//			currRectPos, boxSize, "");
//
//		sliders[i] = panel->AddSlider("slider" + to_string(i),
//			Vector2i(currRectPos.x + boxSize.x + 20, currRectPos.y), 200, 0, 100, 100);
//		myMusicRects[i]->SetShown(true);
//		myMusicRects[i]->Init();
//	}
//
//	playOriginalCheckbox = panel->AddCheckBox("playoriginal", Vector2i(400, 300), false);
//
//
//	worldDropdown = panel->AddDropdown("worlddropdown", Vector2i(10, 10), Vector2i(100, 30),
//		worldOptions, 0);
//
//
//	panel->SetConfirmButton(panel->AddButton("ok", Vector2i(20, panel->size.y - 100), Vector2f(100, 40), "OK"));
//	panel->SetCancelButton(panel->AddButton("cancel", Vector2i(140, panel->size.y - 100), Vector2f(100, 40), "Cancel"));
//}
//
//ListChooser::~ListChooser()
//{
//	delete[] textRects;
//	delete[] myMusicRects;
//	delete panel;
//}
//
//bool ListChooser::MouseUpdate()
//{
//	return handler->MouseUpdate();
//}
//
//void ListChooser::OpenPopup()
//{
//	PopulateRects();
//
//	auto &songOrder = mh->songOrder;
//	auto &songLevels = mh->songLevels;
//	int songCounter = 0;
//
//	for (int i = 0; i < numMyMusicRects; ++i)
//	{
//		sliders[i]->HideMember();
//	}
//
//	for (auto it = songOrder.begin(); it != songOrder.end(); ++it)
//	{
//		myMusicRects[songCounter]->SetName((*it));
//
//		sliders[songCounter]->SetCurrValue(songLevels[(*it)]);
//
//		sliders[songCounter]->ShowMember();
//
//		++songCounter;
//		if (songCounter == 3)
//			break;
//	}
//
//	edit->AddActivePanel(panel);
//}
//
//void ListChooser::ClosePopup()
//{
//	edit->RemoveActivePanel(panel);
//}
//
//void ListChooser::ResetSlider(const std::string &str)
//{
//	for (int i = 0; i < numMyMusicRects; ++i)
//	{
//		if (myMusicRects[i]->nameText.getString() == str)
//		{
//			sliders[i]->SetCurrValue(sliders[i]->defaultValue);
//			sliders[i]->ShowMember();
//		}
//	}
//}
//
//void ListChooser::HideSlider(const std::string &str)
//{
//	for (int i = 0; i < numMyMusicRects; ++i)
//	{
//		if (myMusicRects[i]->nameText.getString() == str)
//		{
//			sliders[i]->HideMember();
//		}
//	}
//}
//
//void ListChooser::SetPlayingColorMyRects(const std::string &str)
//{
//	currPlayingMyRect = NULL;
//	for (int i = 0; i < numMyMusicRects; ++i)
//	{
//		if (str == myMusicRects[i]->nameText.getString())
//		{
//			myMusicRects[i]->SetIdleColor(Color::Magenta);
//			currPlayingMyRect = myMusicRects[i];
//		}
//		else
//		{
//			myMusicRects[i]->SetIdleColor(myMusicRects[i]->defaultIdleColor);
//		}
//	}
//}
//
//void ListChooser::SetPlayingColor(const std::string &str)
//{
//	if (str == "")
//	{
//		return;
//	}
//	playingSongName = str;
//
//	SetPlayingColorMyRects(str);
//
//	currPlayingRect = NULL;
//	for (int i = 0; i < totalRects; ++i)
//	{
//		if (str == textRects[i]->nameText.getString())
//		{
//			textRects[i]->SetIdleColor(Color::Magenta);
//			currPlayingRect = textRects[i];
//		}
//		else
//		{
//			textRects[i]->SetIdleColor(textRects[i]->defaultIdleColor);
//		}
//	}
//}
//
//void ListChooser::SetStoppedColor()
//{
//	playingSongName = "";
//	currPlayingMyRect = NULL;
//	SetStoppedColorMyRects();
//
//	currPlayingRect = NULL;
//	for (int i = 0; i < totalRects; ++i)
//	{
//		textRects[i]->SetIdleColor(textRects[i]->defaultIdleColor);
//	}
//}
//
//void ListChooser::SetStoppedColorMyRects()
//{
//	currPlayingMyRect = NULL;
//	for (int i = 0; i < numMyMusicRects; ++i)
//	{
//		myMusicRects[i]->SetIdleColor(myMusicRects[i]->defaultIdleColor);
//	}
//}
//
//void ListChooser::Draw(sf::RenderTarget *target)
//{
//	handler->Draw(target);
//}
//
//void ListChooser::Deactivate()
//{
//
//}
//
//void ListChooser::MouseScroll(int delta)
//{
//	int oldTopRow = topRow;
//	if (delta < 0)
//	{
//		topRow -= delta;
//		if (topRow > maxTopRow)
//			topRow = maxTopRow;
//	}
//	else if (delta > 0)
//	{
//		topRow -= delta;
//		if (topRow < 0)
//			topRow = 0;
//	}
//
//	if (topRow != oldTopRow)
//	{
//		PopulateRects();
//	}
//}
//
//void ListChooser::LateDraw(sf::RenderTarget *target)
//{
//	handler->LateDraw(target);
//}
//
//void ListChooser::PopulateRects()
//{
//	TextChooseRect *tcRect;
//	int start = topRow;
//
//	int i;
//
//
//	auto &currSongNames = songNames[worldDropdown->GetSelectedText()];
//	for (i = start; i < numEntries && i < start + totalRects; ++i)
//	{
//		tcRect = textRects[i - start];
//
//		tcRect->SetText(currSongNames[i]);
//		//node = nodes[i];
//		//tcRect->SetName( )
//		//tcRect->SetName(node->filePath.filename().stem().string());
//		//tcRect->SetInfo(node);
//
//		/*if (ts != NULL)
//		icRect->SetImage(ts, ts->GetSubRect(0));
//		else
//		{
//		icRect->SetImage(NULL, 0);
//		}*/
//
//		tcRect->SetShown(true);
//	}
//
//	for (; i < start + totalRects; ++i)
//	{
//		tcRect = textRects[i - start];
//		tcRect->SetShown(false);
//	}
//
//	SetPlayingColor(playingSongName);
//	//SetPlayingColor();
//
//}