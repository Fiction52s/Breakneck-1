#include "MapPublishPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"
#include "EditSession.h"
#include "WorkshopManager.h"

using namespace std;
using namespace sf;


MapPublishPopup::MapPublishPopup()
{
	panel = new Panel("mapoptions", 800,
		500, this, true);

	SetGameResourcesMode(false);

	panel->extraUpdater = this;

	mapNameTextBox = panel->AddLabeledTextBox("name", Vector2i(10, 10), 300, 30, "", "Name:");
	

	panel->SetAutoSpacing(false, true, Vector2i(500, 0), Vector2i(0, 10));

	panel->AddLabel("agreementlabel", Vector2i(0, 10), 24, "By submitting this item, \nyou agree to the");

	panel->AddHyperLink("agreementlink", Vector2i(0, 0), 24, "workshop terms of service", "https://steamcommunity.com/sharedfiles/workshoplegalagreement");

	panel->SetAutoSpacing(true, false, Vector2i(500, 400), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	panel->confirmButton =
		panel->AddButton("ok", Vector2i(0, 0), Vector2f(90, 30), "Publish");
	panel->cancelButton =
		panel->AddButton("cancel", Vector2i(0, 0), Vector2f(90, 30), "Cancel");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	action = A_INACTIVE;

	//panel->AddLabeledTextBox("description", Vector2i(10, 300), 500, 120, "", "Description:");
	panel->AddTextBox("description", Vector2i(10, 300), 50, 5, 20, 100, "hello world here\nI am doing my thing");

	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	ts_preview = NULL;
}

MapPublishPopup::~MapPublishPopup()
{
	delete panel;
}

void MapPublishPopup::Update()
{
	panel->MouseUpdate();
}

void MapPublishPopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void MapPublishPopup::Draw(sf::RenderTarget *target)
{
	//only works in editor mode rn
	//panel->Draw(target);
	target->draw(previewSpr);
}

void MapPublishPopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b->name == "ok")
	{
		action = A_CONFIRMED;
	}
	else if (b->name == "cancel")
	{
		action = A_CANCELLED;
	}

	
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
	{
		if (action == A_CONFIRMED && mapNameTextBox->GetString().length() == 0)
		{
			edit->messagePopup->Pop("Name needs to be at least 1 character long.");
			action = A_ACTIVE;
			return;
		}
		else
		{
			edit->RemoveActivePanel(panel);

			if (action == A_CONFIRMED)
			{
				edit->workshopUploader->PublishMap();
			}
		}
	}

	if (ts_preview != NULL)
	{
		DestroyTileset(ts_preview);
		ts_preview = NULL;
	}
}

void MapPublishPopup::Activate()
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
	{
		edit->AddActivePanel(panel);

		string prevPath = edit->filePath.parent_path().string() + "/" + edit->filePath.stem().string() + ".png";

		assert(ts_preview == NULL);
		ts_preview = GetTileset(prevPath);

		assert(ts_preview != NULL);

		if (ts_preview == NULL)
		{
			edit->RemoveActivePanel(panel);
			return;
		}

		ts_preview->SetSpriteTexture(previewSpr);
		previewSpr.setPosition(10, 70);
		previewSpr.setScale(.5, .5);

		mapNameTextBox->SetString(edit->filePath.stem().string());
	}

	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	action = A_ACTIVE;
}