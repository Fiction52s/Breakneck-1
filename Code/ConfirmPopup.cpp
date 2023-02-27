#include "GUI.h"
#include "EditSession.h"
#include "MapBrowser.h"

using namespace std;
using namespace sf;

ConfirmPopup::ConfirmPopup()
{
	edit = EditSession::GetSession();

	panel = new Panel("confirmpanel", 500, 200, this, true);
	panel->SetPosition(Vector2i(960 - panel->size.x / 2, 540 - panel->size.y / 2));

	panel->AddLabel("question", Vector2i(10, 10), 28, "");

	yesButton = panel->AddButton("yes", Vector2i(10, 100), Vector2f(50, 30), "Yes");
	noButton = panel->AddButton("no", Vector2i(70, 100), Vector2f(50, 30), "No");
	backButton = panel->AddButton("back", Vector2i(70 + 60, 100), Vector2f(50, 30), "Cancel");

	//panel->SetConfirmButton(yesButton);
	//panel->SetCancelButton(noButton);

	action = A_ACTIVE;
}

ConfirmPopup::~ConfirmPopup()
{
	delete panel;
}

void ConfirmPopup::SetQuestion(const std::string &q)
{
	panel->labels["question"]->text.setString(q);
}

void ConfirmPopup::Pop(ConfirmType ct)
{
	if (edit != NULL)
	{
		edit->AddActivePanel(panel);
	}
	
	type = ct;

	action = A_ACTIVE;

	backButton->HideMember();

	switch (type)
	{
	case SAVE_CURRENT:
	case SAVE_CURRENT_EXIT:
	case SAVE_CURRENT_OPEN:
	case SAVE_CURRENT_NEW:
		backButton->ShowMember();
		SetQuestion("Changes to current map have not been saved.\nSave before continuing?");
		break;
	case DEFAULT:
	{
		SetQuestion("");
		//panel->labels["question"]->setString("Changes to current map have not been saved.\nSave before continuing?");
		break;
	}
	case BACK_ALLOWED:
	{
		backButton->ShowMember();
		break;
	}
	case OVERWRITE_FILE_AND_EXIT:
	case OVERWRITE_FILE_WITH_BLANK:
	case OVERWRITE_FILE_WITH_BLANK_AND_EXIT:
	case OVERWRITE_FILE_WITH_BLANK_AND_OPEN:
	case OVERWRITE_FILE_AND_OPEN:
	case OVERWRITE_FILE:
	case OVERWRITE_FILE_AND_NEW:
	case OVERWRITE_FILE_WITH_BLANK_AND_NEW:
	{
		SetQuestion("File already exists. Overwrite it?");
		backButton->ShowMember();
		break;
	}
	}
}

void ConfirmPopup::ButtonCallback(Button *b,
	const std::string &e)
{
	if (b == yesButton)
	{
		if (edit != NULL)
		{
			edit->RemoveActivePanel(panel);
		}

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
			if (edit->filePath.string() == "")
			{
				edit->SaveMapDialog( MapBrowser::EDITOR_SAVE_AND_EXIT);
			}
			else
			{
				edit->WriteFile();
				edit->ExitEditor();
			}

			//if (edit->TrySaveMap())
			//{
			//	//if map was saved without the save dialog, cleanly exit
			//	
			//}
			break;
		}
		case SAVE_CURRENT_OPEN:
		{
			if (edit->filePath.string() == "")
			{
				edit->SaveMapDialog(MapBrowser::EDITOR_SAVE_AND_OPEN);
			}
			else
			{
				edit->WriteFile();
				edit->ChooseFileOpen(edit->fileToOpen);
			}
			break;
		}
		case SAVE_CURRENT_NEW:
		{
			if (edit->filePath.string() == "")
			{
				edit->SaveMapDialog(MapBrowser::EDITOR_SAVE_AND_NEW);
			}
			else
			{
				edit->WriteFile();
				edit->ReloadNew();
			}
			break;
		}
		case OVERWRITE_FILE_WITH_BLANK:
		case OVERWRITE_FILE:
		{
			edit->mapBrowserHandler->chooser->TurnOff();
			edit->mapBrowserHandler->chooser->action = MapBrowser::A_CONFIRMED;
			edit->WriteFile();
			break;
		}
		case OVERWRITE_FILE_WITH_BLANK_AND_EXIT:
		case OVERWRITE_FILE_AND_EXIT:
		{
			edit->mapBrowserHandler->chooser->TurnOff();
			edit->mapBrowserHandler->chooser->action = MapBrowser::A_CONFIRMED;
			edit->WriteFile();

			edit->ExitEditor();
			break;
		}
		case OVERWRITE_FILE_AND_OPEN:
		case OVERWRITE_FILE_WITH_BLANK_AND_OPEN:
		{
			edit->mapBrowserHandler->chooser->TurnOff();
			edit->mapBrowserHandler->chooser->action = MapBrowser::A_CONFIRMED;
			edit->WriteFile();

			edit->ChooseFileOpen(edit->fileToOpen);
			//editor opens new map here
			break;
		}
		case OVERWRITE_FILE_AND_NEW:
		case OVERWRITE_FILE_WITH_BLANK_AND_NEW:
		{
			edit->mapBrowserHandler->chooser->TurnOff();
			edit->mapBrowserHandler->chooser->action = MapBrowser::A_CONFIRMED;
			edit->WriteFile();

			edit->ReloadNew();
			break;
		}
		}

		
	}
	else if (b == noButton )
	{
		if (edit != NULL)
		{
			edit->RemoveActivePanel(panel);
		}

		action = A_NO;

		switch (type)
		{
		case SAVE_CURRENT:
		{
			break;
		}
		case SAVE_CURRENT_EXIT:
		{
			edit->quit = true;
			edit->returnVal = 1;
			break;
		}
		case SAVE_CURRENT_OPEN:
		{
			edit->ChooseFileOpen(edit->fileToOpen);
			break;
		}
		case SAVE_CURRENT_NEW:
		{
			edit->ReloadNew();
			break;
		}
		case OVERWRITE_FILE_AND_EXIT:
		case OVERWRITE_FILE:
		{
			
			break;
		}
		case OVERWRITE_FILE_WITH_BLANK:
		case OVERWRITE_FILE_WITH_BLANK_AND_EXIT:
		case OVERWRITE_FILE_WITH_BLANK_AND_OPEN:
		case OVERWRITE_FILE_WITH_BLANK_AND_NEW:
		{
			//return to blank when you cancel/say no
			edit->filePath = "";
			edit->filePathStr = "";
			break;
		}
		}
		
	}
	else if (b == backButton)
	{
		CancelCallback(panel);
	}
}

void ConfirmPopup::CancelCallback(Panel *p)
{
	if (type == BACK_ALLOWED)
	{
		action = A_BACK;
	}
	else
	{
		action = A_NO;

		switch (type)
		{
		case SAVE_CURRENT:
		{
			//edit->ReloadNew();
			break;
		}
		case SAVE_CURRENT_EXIT:
		{
			//edit->quit = true;
			//edit->returnVal = 1;
			break;
		}
		case OVERWRITE_FILE_WITH_BLANK:
		case OVERWRITE_FILE_WITH_BLANK_AND_EXIT:
		case OVERWRITE_FILE_WITH_BLANK_AND_OPEN:
		case OVERWRITE_FILE_WITH_BLANK_AND_NEW:
		{
			//return to blank when you cancel/say no
			edit->filePath = "";
			edit->filePathStr = "";
			break;
		}
		case SAVE_CURRENT_NEW:
		{

			break;
		}

		}
	}

	if (edit != NULL)
	{
		edit->RemoveActivePanel(panel);
	}
}

void ConfirmPopup::Update()
{
	panel->MouseUpdate();
}

bool ConfirmPopup::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void ConfirmPopup::Draw(sf::RenderTarget *target)
{
	panel->Draw(target);
}