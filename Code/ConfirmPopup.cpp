#include "GUI.h"
#include "EditSession.h"

using namespace std;
using namespace sf;

ConfirmPopup::ConfirmPopup()
{
	edit = EditSession::GetSession();

	panel = new Panel("confirmpanel", 500, 200, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	panel->AddLabel("question", Vector2i(10, 10), 28, "");

	panel->SetConfirmButton(panel->AddButton("yes", Vector2i(10, 100), Vector2f(50, 30), "Yes"));
	panel->SetCancelButton(panel->AddButton("no", Vector2i(70, 100), Vector2f(50, 30), "No"));
}

ConfirmPopup::~ConfirmPopup()
{
	delete panel;
}

void ConfirmPopup::Pop(ConfirmType ct)
{
	edit->AddActivePanel(panel);
	type = ct;

	switch (type)
	{
	case SAVE_CURRENT:
		panel->labels["question"]->setString("Current file has not been saved.\nSave before continuing?");
		break;
	}
}

void ConfirmPopup::ButtonCallback(Button *b,
	const std::string &e)
{
	if (b == panel->confirmButton)
	{
		switch (type)
		{
		case SAVE_CURRENT:
		{
			edit->WriteFile(edit->currentFile);
			edit->ReloadNew();
			break;
		}
		}
	}
	else if (b == panel->cancelButton)
	{
		edit->ReloadNew();
	}
}