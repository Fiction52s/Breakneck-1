#include "OnlineMenuScreen.h"
#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

OnlineMenuScreen::OnlineMenuScreen(MainMenu *mm)
{
	mainMenu = mm;

	ts_bg = GetTileset("Menu/Load/load_w8.png");


	panel = new Panel("onlinemenu", 1600,
		500, this, true);

	panel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(30, 0));

	

	//panel->AddHyperLink("test", Vector2i(0, 0), 30, "test link", "blah");

	panel->ReserveImageRects(Action::Count);

	panel->SetAutoSpacing(true, false, Vector2i(200, 200), Vector2i( 30, 0 ));
	ImageChooseRect *icr = NULL;
	icr = panel->AddImageRect(ChooseRect::I_ONLINEMENU_WORKSHOP, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Browse Workshop");
	icr->SetShown(true);
	icr->Init();

	icr = panel->AddImageRect(ChooseRect::I_ONLINEMENU_QUICKPLAY, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Quickplay");
	icr->SetShown(true);
	icr->Init();
	icr = panel->AddImageRect(ChooseRect::I_ONLINEMENU_CREATE_LOBBY, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Create Lobby");
	icr->SetShown(true);
	icr->Init();
	icr = panel->AddImageRect(ChooseRect::I_ONLINEMENU_JOIN_LOBBY, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Join Lobby");
	icr->SetShown(true);
	icr->Init();

	//panel->confirmButton =
	//	panel->AddButton("ok", Vector2i(0, 0), Vector2f(60, 30), "Host");
	panel->cancelButton =
		panel->AddButton("back", Vector2i(0, 0), Vector2f(120, 50), "Back");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	//SetRectColor(bgQuad, Color::Blue);
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));
	SetRectSubRect(bgQuad, ts_bg->GetSubRect(0));

	action = A_NONE;
}

OnlineMenuScreen::~OnlineMenuScreen()
{
	delete panel;
}

bool OnlineMenuScreen::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void OnlineMenuScreen::Start()
{
	action = A_NONE;
}

void OnlineMenuScreen::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
	{
		
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
	{
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		switch (cr->rectIdentity)
		{
		case ChooseRect::I_ONLINEMENU_WORKSHOP:
		{
			action = A_WORKSHOP;
			break;
		}
		case ChooseRect::I_ONLINEMENU_QUICKPLAY:
		{
			action = A_QUICKPLAY;
			break;
		}
		case ChooseRect::I_ONLINEMENU_CREATE_LOBBY:
		{
			action = A_CREATE_LOBBY;
			break;
		}
		case ChooseRect::I_ONLINEMENU_JOIN_LOBBY:
		{
			action = A_JOIN_LOBBY;
			break;
		}
		}
	}
}

void OnlineMenuScreen::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "back")
	{
		CancelCallback(b->panel);
	}
}

void OnlineMenuScreen::CancelCallback(Panel *p)
{
	action = A_CANCELLED;
}

void OnlineMenuScreen::Update()
{
	panel->MouseUpdate();
}

void OnlineMenuScreen::Draw(sf::RenderTarget *target)
{

	/*if (browserHandler->chooser->action == MapBrowser::A_WAITING_FOR_MAP_DOWNLOAD)
	{
	SetRectColor(downloadingQuad, Color::Red);
	}*/
	target->draw(bgQuad, 4, sf::Quads, ts_bg->texture);
	panel->Draw(target);
}