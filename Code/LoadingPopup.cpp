#include "LoadingPopup.h"
#include "Editsession.h"

using namespace std;
using namespace sf;

#include "GUI.h"
#include "EditSession.h"

using namespace std;
using namespace sf;

LoadingPopup::LoadingPopup()
{
	edit = EditSession::GetSession();

	panel = new Panel("loadingpanel", 500, 200, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	messageLabel = panel->AddLabel("message", Vector2i(10, 10), 28, "");

	//Button *button = panel->AddButton("ok", Vector2i(panel->size.x / 2, 100), Vector2f(50, 30), "OK");
	//panel->SetConfirmButton(button);
	//panel->SetCancelButton(button);

	action = A_INACTIVE;
}

LoadingPopup::~LoadingPopup()
{
	delete panel;
}

void LoadingPopup::Update()
{
	panel->MouseUpdate();

	UpdateFullMessage();

	++frame;
}

void LoadingPopup::UpdateFullMessage()
{
	int numPeriods = (frame / 20) % 4;
	if (frame % 20 == 0)
	{
		string fullStr = preDotsStr;
		for (int i = 0; i < numPeriods; ++i)
		{
			fullStr += ".";
		}

		fullStr += postDotsStr;
		messageLabel->text.setString(fullStr);
	}
}

void LoadingPopup::Pop(const std::string &preMsg, const std::string &postMsg )
{
	if (edit != NULL)
	{
		edit->AddActivePanel(panel);
	}

	preDotsStr = preMsg;
	postDotsStr = postMsg;

	action = A_ACTIVE;
	frame = 0;

	UpdateFullMessage();

	++frame; //prevents instant double update
}

void LoadingPopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}

//void LoadingPopup::ButtonCallback(Button *b,
//	const std::string &e)
//{
//	ConfirmCallback(b->panel);
//}
//
//void LoadingPopup::CancelCallback(Panel *p)
//{
//	ConfirmCallback(p);
//}
//
//void LoadingPopup::ConfirmCallback(Panel *p)
//{
//	if (edit != NULL)
//	{
//		edit->RemoveActivePanel(panel);
//	}
//
//	action = A_INACTIVE;
//}