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

	Button *button = panel->AddButton("ok", Vector2i(panel->size.x / 2, 100), Vector2f(50, 30), "OK");
	panel->SetConfirmButton(button);
	panel->SetCancelButton(button);

	action = A_INACTIVE;
}

MessagePopup::~MessagePopup()
{
	delete panel;
}

void MessagePopup::Update()
{
	panel->MouseUpdate();
}

void MessagePopup::Pop(const std::string &message)
{
	if (edit != NULL)
	{
		edit->AddActivePanel(panel);
	}

	action = A_ACTIVE;

	panel->labels["message"]->setString(message);
}

void MessagePopup::ButtonCallback(Button *b,
	const std::string &e)
{
	if (edit != NULL)
	{
		edit->RemoveActivePanel(panel);
	}

	action = A_INACTIVE;
}

void MessagePopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}