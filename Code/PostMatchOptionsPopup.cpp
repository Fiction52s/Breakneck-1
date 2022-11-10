#include "PostMatchOptionsPopup.h"
#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

PostMatchOptionsPopup::PostMatchOptionsPopup()
{
	panel = new Panel("postmatchpopup", 1600,
		500, this, true);

	panel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(30, 0));

	//panel->AddHyperLink("test", Vector2i(0, 0), 30, "test link", "blah");

	panel->ReserveImageRects(Action::Count);

	panel->SetAutoSpacing(true, false, Vector2i(200, 200), Vector2i(20, 0));
	ImageChooseRect *icr = NULL;
	icr = panel->AddImageRect(ChooseRect::I_POST_MATCH_REMATCH, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Rematch");
	icr->SetShown(true);
	icr->Init();

	icr = panel->AddImageRect(ChooseRect::I_POST_MATCH_CHOOSE_MAP, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Choose Map");
	icr->SetShown(true);
	icr->Init();
	icr = panel->AddImageRect(ChooseRect::I_POST_MATCH_LEAVE, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Leave");
	icr->SetShown(true);
	icr->Init();

	//panel->confirmButton =
	//	panel->AddButton("ok", Vector2i(0, 0), Vector2f(60, 30), "Host");
	//panel->cancelButton =
	//	panel->AddButton("back", Vector2i(0, 0), Vector2f(80, 30), "Back");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	SetRectColor(bgQuad, Color::Blue);
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	action = A_IDLE;
}

PostMatchOptionsPopup::~PostMatchOptionsPopup()
{
	delete panel;
}

bool PostMatchOptionsPopup::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void PostMatchOptionsPopup::Start()
{
	action = A_IDLE;
}

void PostMatchOptionsPopup::ChooseRectEvent(ChooseRect *cr, int eventType)
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
		case ChooseRect::I_POST_MATCH_REMATCH:
		{
			action = A_REMATCH;
			break;
		}
		case ChooseRect::I_POST_MATCH_CHOOSE_MAP:
		{
			action = A_CHOOSE_MAP;
			break;
		}
		case ChooseRect::I_POST_MATCH_LEAVE:
		{
			action = A_LEAVE;
			break;
		}
		}
	}
}

void PostMatchOptionsPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "back")
	{
		//action = A_CANCELLED;
	}
}

void PostMatchOptionsPopup::Update()
{
	panel->MouseUpdate();
	/*ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;

	if (menuCurrInput.B && !menuPrevInput.B)
	{
	mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}*/


}

void PostMatchOptionsPopup::Draw(sf::RenderTarget *target)
{
	/*if (browserHandler->chooser->action == MapBrowser::A_WAITING_FOR_MAP_DOWNLOAD)
	{
	SetRectColor(downloadingQuad, Color::Red);
	}*/
	//target->draw(bgQuad, 4, sf::Quads);
	panel->Draw(target);
}