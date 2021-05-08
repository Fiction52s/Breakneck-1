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

void ListChooserHandler::PanelCallback(Panel *p, const std::string & e)
{
	if (e == "leftclickoffpopup")
	{
		chooser->ClosePopup();	
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
	

	//panel->AddButton("button", Vector2i(0, 0), Vector2f(100, 100), "blah");

	/*panel->ReserveImageRects(1);
	ImageChooseRect *icrect = panel->AddImageRect(ChooseRect::ChooseRectIdentity::I_BRUSHHOTBAR,
		Vector2f(100, 100), NULL, 0);
	icrect->SetShown(true);
	icrect->Init();
	icrect->SetName("blahblah");*/

	/*panel->ReserveTextRects(1);
	TextChooseRect *test = panel->AddTextRect(ChooseRect::ChooseRectIdentity::I_BRUSHHOTBAR,
		Vector2f(100, 100), Vector2f( 100, 100 ), "blahblahblbah");
	test->SetShown(true);
	test->Init();
	test->SetName("neheheh");*/

	panel->ReserveTextRects(totalRects + numMyMusicRects);
	for (int i = 0; i < totalRects; ++i)
	{
		textRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLIBRARY,
			Vector2f(startRects.x, startRects.y + (boxSize.y + spacing.y) * i),
			boxSize, "hello" + to_string(i));
		textRects[i]->SetShown(true);
		textRects[i]->Init();
	}

	for (int i = 0; i < numMyMusicRects; ++i)
	{
		myMusicRects[i] = panel->AddTextRect(ChooseRect::I_MUSICLIBRARY,
			Vector2f(myMusicRectsXStart, startRects.y + (boxSize.y + spacing.y) * i),
			boxSize, "");
		myMusicRects[i]->SetShown(true);
		myMusicRects[i]->Init();
	}

	//
}

ListChooser::~ListChooser()
{
	delete[] textRects;
}

bool ListChooser::MouseUpdate()
{
	return false;
}

void ListChooser::OpenPopup()
{
	PopulateRects();
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
	listHandler = new ListChooserHandler(10);
	/*panel = new Panel("panel", 500, 500, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2,
		540 - panel->size.y / 2));*/
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