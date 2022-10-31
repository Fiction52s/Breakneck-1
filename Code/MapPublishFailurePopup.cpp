#include "MapPublishFailurePopup.h"
#include "MapPublishPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"
#include "EditSession.h"
#include "WorkshopManager.h"

using namespace std;
using namespace sf;


MapPublishFailurePopup::MapPublishFailurePopup()
{
	panel = new Panel("publishfail", 900,
		500, this, true);

	SetGameResourcesMode(false);

	panel->extraUpdater = this;

	mapNameText = panel->AddLabel("failure", Vector2i(10, 10), 30, "Map failed to upload. Error is: ");



	panel->SetAutoSpacing(true, false, Vector2i(500, 300), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	panel->confirmButton =
		panel->AddButton("ok", Vector2i(0, 0), Vector2f(90, 30), "OK");
	//panel->cancelButton =
	//	panel->AddButton("cancel", Vector2i(0, 0), Vector2f(90, 30), "Cancel");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	action = A_INACTIVE;


	ts_preview = NULL;

}

MapPublishFailurePopup::~MapPublishFailurePopup()
{
	delete panel;
}

void MapPublishFailurePopup::Update()
{
	panel->MouseUpdate();
}

void MapPublishFailurePopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void MapPublishFailurePopup::Draw(sf::RenderTarget *target)
{
	//only works in editor mode rn
	//panel->Draw(target);
	//target->draw(previewSpr);
}

void MapPublishFailurePopup::ButtonCallback(Button *b, const std::string & e)
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
		edit->RemoveActivePanel(panel);

		if (action == A_CONFIRMED)
		{
			//edit->workshopUploader->PublishMap();
		}
	}

	if (ts_preview != NULL)
	{
		DestroyTileset(ts_preview);
		ts_preview = NULL;
	}
}

void MapPublishFailurePopup::Activate(PublishedFileId_t uploadID, int errorCode, bool onCreate )
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
	{
		edit->AddActivePanel(panel);

		/*string prevPath = edit->filePath.parent_path().string() + "/" + edit->filePath.stem().string() + ".png";

		assert(ts_preview == NULL);
		ts_preview = GetTileset(prevPath);

		assert(ts_preview != NULL);

		if (ts_preview == NULL)
		{
			edit->RemoveActivePanel(panel);
			return;
		}

		ts_preview->SetSpriteTexture(previewSpr);
		previewSpr.setPosition(10, 70 + 50);
		previewSpr.setScale(.5, .5);*/

		if (!onCreate)
		{
			mapNameText->setString("Map failed to upload. Error on editing is: " + to_string(errorCode));
		}
		else
		{
			mapNameText->setString("Map failed to upload. Error on creating is: " + to_string(errorCode));
		}
	}

	//mapLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(uploadID));

	//panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	action = A_ACTIVE;
}