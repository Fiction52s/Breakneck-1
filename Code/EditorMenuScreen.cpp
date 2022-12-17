#include "EditorMenuScreen.h"
#include <iostream>
#include "MainMenu.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

EditorMenuScreen::EditorMenuScreen(MainMenu *mm)
{
	mainMenu = mm;

	panel = new Panel("editormenu", 1600,
		500, this, true);

	panel->SetAutoSpacing(true, false, Vector2i(10, 10), Vector2i(30, 0));

	//panel->AddHyperLink("test", Vector2i(0, 0), 30, "test link", "blah");

	panel->ReserveImageRects(Action::Count);

	panel->SetAutoSpacing(true, false, Vector2i(200, 200), Vector2i(20, 0));
	ImageChooseRect *icr = NULL;
	icr = panel->AddImageRect(ChooseRect::I_EDITORMENU_NEW, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("New Map");
	icr->SetShown(true);
	icr->Init();
	icr = panel->AddImageRect(ChooseRect::I_EDITORMENU_OPEN, Vector2f(0, 0), NULL, sf::IntRect(), 200);
	icr->SetName("Open Map");
	icr->SetShown(true);
	icr->Init();

	//panel->confirmButton =
	//	panel->AddButton("ok", Vector2i(0, 0), Vector2f(60, 30), "Host");
	panel->cancelButton =
		panel->AddButton("back", Vector2i(0, 0), Vector2f(80, 30), "Back");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	SetRectColor(bgQuad, Color::Blue);
	SetRectCenter(bgQuad, 1920, 1080, Vector2f(960, 540));

	action = A_NONE;
}

EditorMenuScreen::~EditorMenuScreen()
{
	delete panel;
}

bool EditorMenuScreen::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void EditorMenuScreen::Start()
{
	action = A_NONE;
}

void EditorMenuScreen::ChooseRectEvent(ChooseRect *cr, int eventType)
{
	if (eventType == ChooseRect::ChooseRectEventType::E_FOCUSED)
	{

	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_UNFOCUSED)
	{
	}
	else if (eventType == ChooseRect::ChooseRectEventType::E_LEFTCLICKED)
	{
		/*I_EDITORMENU_NEW,
			I_EDITORMENU_OPEN,*/

		switch (cr->rectIdentity)
		{
		case ChooseRect::I_EDITORMENU_NEW:
		{
			action = A_NEW_MAP;
			break;
		}
		case ChooseRect::I_EDITORMENU_OPEN:
		{
			action = A_OPEN_MAP;
			break;
		}
		}
	}
}

void EditorMenuScreen::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "back")
	{
		CancelCallback(b->panel);
	}
}

void EditorMenuScreen::CancelCallback(Panel *p)
{
	action = A_CANCELLED;
}

void EditorMenuScreen::Update()
{
	panel->MouseUpdate();
}

void EditorMenuScreen::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}