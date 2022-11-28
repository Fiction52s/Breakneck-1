#include "PostMatchClientPopup.h"
#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

PostMatchClientPopup::PostMatchClientPopup()
{
	panel = new Panel("postmatchpopup", 1600,
		500, this, true);

	panel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(30, 0));

	//panel->AddHyperLink("test", Vector2i(0, 0), 30, "test link", "blah");

	panel->ReserveImageRects(Action::Count);

	waitingLabel = panel->AddLabel("waitinglabel", sf::Vector2i(10, 10), 30, "");

	panel->SetAutoSpacing(true, false, Vector2i(200, 200), Vector2i(20, 0));
	ImageChooseRect *icr = NULL;
	icr = panel->AddImageRect(ChooseRect::I_POST_MATCH_LEAVE, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Leave");
	icr->SetShown(true);
	icr->Init();

	secondsToWait = 20;

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

PostMatchClientPopup::~PostMatchClientPopup()
{
	delete panel;
}

bool PostMatchClientPopup::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

int PostMatchClientPopup::GetSecondsRemaining()
{
	return secondsToWait - (framesWaiting / 60);
}

void PostMatchClientPopup::Start()
{
	action = A_IDLE;
	framesWaiting = 0;
	UpdateMsg();
}

void PostMatchClientPopup::ChooseRectEvent(ChooseRect *cr, int eventType)
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
		case ChooseRect::I_POST_MATCH_LEAVE:
		{
			action = A_LEAVE;
			break;
		}
		}
	}
}

void PostMatchClientPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "back")
	{
		//action = A_CANCELLED;
	}
}

void PostMatchClientPopup::UpdateMsg()
{
	int numPeriods = (framesWaiting / 20) % 4;
	if (framesWaiting % 20 == 0)
	{
		string waitingMsg = "Waiting for Host";
		for (int i = 0; i < numPeriods; ++i)
		{
			waitingMsg += ".";
		}

		waitingMsg += "(" + to_string(GetSecondsRemaining()) + ")";
		//mapNameText->setString(word);
		waitingLabel->text.setString(waitingMsg);
	}
}

void PostMatchClientPopup::Update()
{
	panel->MouseUpdate();

	//waitingText

	UpdateMsg();

	++framesWaiting;
	
	/*ControllerState &menuCurrInput = mainMenu->menuCurrInput;
	ControllerState &menuPrevInput = mainMenu->menuPrevInput;

	if (menuCurrInput.B && !menuPrevInput.B)
	{
	mainMenu->SetMode(MainMenu::Mode::TRANS_CREDITS_TO_MAIN);
	}*/


}

void PostMatchClientPopup::Draw(sf::RenderTarget *target)
{
	/*if (browserHandler->chooser->action == MapBrowser::A_WAITING_FOR_MAP_DOWNLOAD)
	{
	SetRectColor(downloadingQuad, Color::Red);
	}*/
	//target->draw(bgQuad, 4, sf::Quads);
	panel->Draw(target);
}