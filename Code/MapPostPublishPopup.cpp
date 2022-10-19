#include "MapPostPublishPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"
#include "EditSession.h"
#include "WorkshopManager.h"

using namespace std;
using namespace sf;


MapPostPublishPopup::MapPostPublishPopup()
{
	panel = new Panel("postpublish", 900,
		500, this, true);

	SetGameResourcesMode(false);

	panel->extraUpdater = this;

	mapNameText = panel->AddLabel("namelabel", Vector2i(10, 10), 40, "");

	panel->SetAutoSpacing(false, true, Vector2i(500, 0), Vector2i(0, 10));

	remainPrivateText = panel->AddLabel("remainprivatelabel", Vector2i(0, 10), 24, "Your maps will remain hidden\nuntil you accept the ");

	agreementLink = panel->AddHyperLink("agreementlink", Vector2i(0, 0), 24, "Workshop terms of service", "");

	mapLink = panel->AddHyperLink("workshoplink", Vector2i(0, 30), 24, "Click here to see your map on the Workshop\nand edit its information.", "");

	panel->SetAutoSpacing(true, false, Vector2i(500, 300), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	panel->confirmButton =
		panel->AddButton("ok", Vector2i(0, 0), Vector2f(90, 30), "OK");
	//panel->cancelButton =
	//	panel->AddButton("cancel", Vector2i(0, 0), Vector2f(90, 30), "Cancel");

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	action = A_INACTIVE;

	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	ts_preview = NULL;

}

MapPostPublishPopup::~MapPostPublishPopup()
{
	delete panel;
}

void MapPostPublishPopup::Update()
{
	panel->MouseUpdate();
}

void MapPostPublishPopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void MapPostPublishPopup::Draw(sf::RenderTarget *target)
{
	//only works in editor mode rn
	//panel->Draw(target);
	target->draw(previewSpr);
}

void MapPostPublishPopup::ButtonCallback(Button *b, const std::string & e)
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

void MapPostPublishPopup::Activate( bool agreementSigned, PublishedFileId_t uploadID )
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

		mapNameText->setString(edit->filePath.stem().string());
	}

	if (agreementSigned)
	{
		remainPrivateText->setFillColor(Color::Transparent);
		agreementLink->HideMember();
	}
	else
	{
		remainPrivateText->setFillColor(Color::Black);
		agreementLink->ShowMember();
	}

	mapLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(uploadID));

	panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	action = A_ACTIVE;
}