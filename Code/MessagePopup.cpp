#include "GUI.h"
#include "EditSession.h"

using namespace std;
using namespace sf;

MessagePopup::MessagePopup()
{
	edit = EditSession::GetSession();

	panel = new Panel("confirmpanel", 500, 200, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	panel->AddLabel("message", Vector2i(10, 10), 28, "");

	Button *button = panel->AddButton("ok", Vector2i(10, 100), Vector2f(50, 30), "OK");
	panel->SetConfirmButton(button);
	panel->SetCancelButton(button);
}

MessagePopup::~MessagePopup()
{
	delete panel;
}

void MessagePopup::Pop(const std::string &message)
{
	edit->AddActivePanel(panel);

	panel->labels["message"]->setString(message);
}

void MessagePopup::ButtonCallback(Button *b,
	const std::string &e)
{
	edit->RemoveActivePanel(panel);
}