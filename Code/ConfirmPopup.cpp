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

	action = A_ACTIVE;
}

ConfirmPopup::~ConfirmPopup()
{
	delete panel;
}

void ConfirmPopup::SetQuestion(const std::string &q)
{
	panel->labels["question"]->setString(q);
}

void ConfirmPopup::Pop(ConfirmType ct)
{
	if (edit != NULL)
	{
		edit->AddActivePanel(panel);
	}
	
	type = ct;

	action = A_ACTIVE;

	switch (type)
	{
	case SAVE_CURRENT:
	case SAVE_CURRENT_EXIT:
		SetQuestion("Changes to current map have not been saved.\nSave before continuing?");
		break;
	case DEFAULT:
	{
		SetQuestion("");
		//panel->labels["question"]->setString("Changes to current map have not been saved.\nSave before continuing?");
		break;
	}
	}
}

void ConfirmPopup::ButtonCallback(Button *b,
	const std::string &e)
{
	if (b == panel->confirmButton)
	{
		action = A_YES;

		switch (type)
		{
		case SAVE_CURRENT:
		{
			edit->WriteFile();
			edit->ReloadNew();
			break;
		}
		case SAVE_CURRENT_EXIT:
		{
			edit->WriteFile();

			edit->quit = true;
			edit->returnVal = 1;
			break;
		}
		}

		if (edit != NULL)
		{
			edit->RemoveActivePanel(panel);
		}
	}
	else if (b == panel->cancelButton)
	{
		CancelCallback(panel);
	}
}

void ConfirmPopup::CancelCallback(Panel *p)
{
	action = A_NO;

	switch (type)
	{
	case SAVE_CURRENT:
	{
		edit->ReloadNew();
		break;
	}
	case SAVE_CURRENT_EXIT:
	{
		edit->quit = true;
		edit->returnVal = 1;
		break;
	}

	}

	if (edit != NULL)
	{
		edit->RemoveActivePanel(panel);
	}
}