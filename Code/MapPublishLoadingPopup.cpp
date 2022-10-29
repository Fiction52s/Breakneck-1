#include "MapPublishLoadingPopup.h"
#include "MapPublishPopup.h"
#include "MapHeader.h"
#include "LobbyManager.h"
#include "MapBrowser.h"
#include "md5.h"
#include "EditSession.h"
#include "WorkshopManager.h"

using namespace std;
using namespace sf;


MapPublishLoadingPopup::MapPublishLoadingPopup()
{
	panel = new Panel("publishload", 500,
		200, this, true);

	SetGameResourcesMode(false);

	panel->extraUpdater = this;

	mapNameText = panel->AddLabel("uploading", Vector2i(10, 10), 30, "");



	//panel->SetAutoSpacing(true, false, Vector2i(500, 300), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	//panel->confirmButton =
	//	panel->AddButton("ok", Vector2i(0, 0), Vector2f(90, 30), "OK");
	//panel->cancelButton =
	//	panel->AddButton("cancel", Vector2i(0, 0), Vector2f(90, 30), "Cancel");

	//panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	action = A_INACTIVE;


	ts_preview = NULL;

}

MapPublishLoadingPopup::~MapPublishLoadingPopup()
{
	delete panel;
}

void MapPublishLoadingPopup::SetAttempt(int p_currAttempt, int p_maxAttempts)
{
	currAttempt = p_currAttempt;
	maxAttempts = p_maxAttempts;

	int numPeriods = (frame / 20) % 4;
	if (frame % 20 == 0)
	{
		string word = "Uploading";
		for (int i = 0; i < numPeriods; ++i)
		{
			word += ".";
		}

		word += ", Attempt " + to_string(currAttempt) + "/" + to_string(maxAttempts);
		mapNameText->setString(word);
	}
}

void MapPublishLoadingPopup::Update()
{
	panel->MouseUpdate();
}

void MapPublishLoadingPopup::UpdateFrame(int numUpdateFrames)
{
	for (int f = 0; f < numUpdateFrames; ++f)
	{
		int numPeriods = (frame / 20) % 4;
		if (frame % 20 == 0)
		{
			string word = "Uploading";
			for (int i = 0; i < numPeriods; ++i)
			{
				word += ".";
			}

			word += ", Attempt " + to_string(currAttempt) + "/" + to_string(maxAttempts);
			mapNameText->setString(word);
		}

		++frame;
	}
	
}

void MapPublishLoadingPopup::ClosePopup()
{
	EditSession *edit = EditSession::GetSession();
	if (edit != NULL)
	{
		edit->RemoveActivePanel(panel);

		if (action == A_CONFIRMED)
		{
			//edit->workshopUploader->PublishMap();
		}
	}
}

void MapPublishLoadingPopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void MapPublishLoadingPopup::Draw(sf::RenderTarget *target)
{
	//only works in editor mode rn
	//panel->Draw(target);
	//target->draw(previewSpr);
}

void MapPublishLoadingPopup::ButtonCallback(Button *b, const std::string & e)
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

void MapPublishLoadingPopup::Activate()
{
	frame = 0;

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

		mapNameText->setString("Uploading");
	}

	//mapLink->SetLinkURL("steam://url/CommunityFilePage/" + to_string(uploadID));

	//panel->SetAutoSpacing(false, true, Vector2i(10, 10), Vector2i(0, 20));

	action = A_ACTIVE;
}