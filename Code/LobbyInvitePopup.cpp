#include "LobbyInvitePopup.h"
//#include "MapPublishPopup.h"
//#include "MapHeader.h"
#include "LobbyManager.h"
//#include "MapBrowser.h"
//#include "md5.h"
//#include "WorkshopManager.h"

using namespace std;
using namespace sf;


LobbyInvitePopup::LobbyInvitePopup()
{
	panel = new Panel("publishfail", 400,
		200, this, true);

	SetGameResourcesMode(false);

	panel->extraUpdater = this;

	//mapNameText = panel->AddLabel("failure", Vector2i(10, 10), 30, "Map failed to upload. Error is: ");

	inviteMessage = panel->AddLabel("invitelabel", Vector2i(0, 0), 30, "has invited you to join a lobby");
	inviteSenderName = panel->AddHyperLink("senderlink", Vector2i(10, 10), 30, "", "");

	panel->SetAutoSpacing(true, false, Vector2i(10, 100), Vector2i(30, 0));
	//fileNameTextBox = panel->AddTextBox("filename", Vector2i(0, 0), 500, 40, "");

	confirmButton = panel->AddButton("yesbutton", Vector2i(0, 0), Vector2f(90, 30), "ACCEPT");
	panel->SetConfirmButton(confirmButton);

	cancelButton = panel->AddButton("nobutton", Vector2i(0, 0), Vector2f(90, 30), "DENY");
	panel->SetCancelButton(cancelButton);

	panel->StopAutoSpacing();

	panel->SetCenterPos(Vector2i(960, 540));

	action = A_INACTIVE;
}

LobbyInvitePopup::~LobbyInvitePopup()
{
	delete panel;
}

void LobbyInvitePopup::Update()
{
	panel->MouseUpdate();
}

void LobbyInvitePopup::HandleEvent(sf::Event ev)
{
	panel->HandleEvent(ev);
}

void LobbyInvitePopup::Draw(sf::RenderTarget *target)
{
	//only works in editor mode rn
	//panel->Draw(target);
	//target->draw(previewSpr);
}

void LobbyInvitePopup::ButtonCallback(Button *b, const std::string & e)
{
	if (b == confirmButton )
	{
		action = A_CONFIRMED;
	}
	else if (b == cancelButton)
	{
		action = A_CANCELLED;
	}
}

void LobbyInvitePopup::OpenPopup(CSteamID p_lobbyId, CSteamID p_senderId)
{
	lobbyId = p_lobbyId;
	senderId = p_senderId;

	inviteSenderName->SetLinkURL("https://steamcommunity.com/profiles/" + to_string(senderId.ConvertToUint64()));
	inviteSenderName->SetString(SteamFriends()->GetFriendPersonaName(senderId));

	auto senderGlobalBounds = inviteSenderName->text.getGlobalBounds();
	int spacing = 20;
	inviteMessage->setPosition(senderGlobalBounds.left + senderGlobalBounds.width + spacing, inviteSenderName->pos.y);

	action = A_ACTIVE;
}