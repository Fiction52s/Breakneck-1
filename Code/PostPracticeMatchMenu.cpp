#include "PostPracticeMatchMenu.h"
#include "BasicTextMenu.h"
#include "SingleInputMenuButton.h"
#include "NetplayManager.h"
#include "MainMenu.h"
#include "GamePopup.h"

using namespace std;
using namespace sf;

PostPracticeMatchMenu::PostPracticeMatchMenu()
{
	vector<string> practiceHostOptions = { "Invite to Custom Lobby", "Leave" };
	textMenu = new BasicTextMenu(practiceHostOptions);

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	readyButton = new SingleInputMenuButton( "Not Ready", "Ready",
		Color::Red, Color::Green, Color::White, Color::White, Vector2f( 500, 200 ), XBoxButton::XBOX_START, 64 );
	readyButton->SetCenter(Vector2f(960, 300));

	inviteButton = new SingleInputMenuButton("Invite to Custom Lobby", "Player Invited!",
		Color::Red, Color::Green, Color::White, Color::White, Vector2f(500, 200), XBoxButton::XBOX_START, 64);
	inviteButton->SetCenter(Vector2f(1400, 300));

	acceptButton = new SingleInputMenuButton("Accept Custom Lobby Invite", "Invite Accepted",
		Color::Red, Color::Green, Color::White, Color::White, Vector2f(500, 200), XBoxButton::XBOX_START, 64);
	acceptButton->SetCenter(Vector2f(1400, 700));

	acceptPopup = new GamePopup;

	MainMenu *mainMenu = MainMenu::GetInstance();

	waitForLobbyText.setFont(mainMenu->arial);
	waitForLobbyText.setCharacterSize(50);
	waitForLobbyText.setString("waiting for lobby");
	waitForLobbyText.setFillColor(Color::White);
	auto lb = waitForLobbyText.getLocalBounds();
	waitForLobbyText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	waitForLobbyText.setPosition(960, 540);


	SetRectTopLeft(otherPlayerTestQuad, 100, 100, Vector2f(0, 0));
}

PostPracticeMatchMenu::~PostPracticeMatchMenu()
{
	delete textMenu;
	delete readyButton;

	delete inviteButton;
	delete acceptButton;

	delete acceptPopup;
}

void PostPracticeMatchMenu::Reset()
{
	action = A_ACCEPT_POPUP;
	acceptPopup->SetInfo("You are being invited to a custom lobby. Accept?", 2, GamePopup::OPTION_YES);

	textMenu->Reset();
	readyButton->Reset();
	inviteButton->Reset();
	acceptButton->Reset();

	SetRectColor(otherPlayerTestQuad, Color::Red);
}

bool PostPracticeMatchMenu::WantsToKeepPlaying()
{
	return readyButton->action == SingleInputMenuButton::A_ON;
}

bool PostPracticeMatchMenu::IsReadyToJoinCustomLobby()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
	return action == A_WAIT_FOR_LOBBY && netplayManager->hasReceivedPostPracticeRaceCustomLobbyID;
}

int PostPracticeMatchMenu::Update()
{
	if (action == A_NORMAL)
	{
		NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

		if (netplayManager->hasReceivedPostPracticeRaceCustomLobbyInvite)
		{
			action = A_ACCEPT_POPUP;
			acceptPopup->SetInfo("You are being invited to a custom lobby. Accept?", 2, GamePopup::OPTION_YES);
			return -1;
		}


		bool oldKeepPlayingOn = WantsToKeepPlaying();

		readyButton->Update(CONTROLLERS.ButtonPressed_Start(), CONTROLLERS.ButtonPressed_B());

		bool keepPlayingOn = WantsToKeepPlaying();

		if (oldKeepPlayingOn != keepPlayingOn)
		{
			netplayManager->SendPostMatchPracticeKeepPlayingSignal(keepPlayingOn);
		}

		if (netplayManager->PeerWantsToKeepPlayingPractice())
		{
			SetRectColor(otherPlayerTestQuad, Color::Green);
		}
		else
		{
			SetRectColor(otherPlayerTestQuad, Color::Red);
		}

		inviteButton->Update(CONTROLLERS.ButtonPressed_X(), false);
		acceptButton->Update(CONTROLLERS.ButtonPressed_Y(), false);

		int textResult = textMenu->Update();

		return textResult;
	}
	else if( action == A_ACCEPT_POPUP)
	{
		NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;
		int result = acceptPopup->Update(netplayManager->myControllerInput);

		if (result == GamePopup::OPTION_NO)
		{
			action = A_NORMAL;
			netplayManager->SendPostMatchPracticeCustomLobbyRejectSignal();
		}
		else if (result == GamePopup::OPTION_YES)
		{
			action = A_WAIT_FOR_LOBBY;
			netplayManager->SendPostMatchPracticeCustomLobbyAcceptSignal();
		}

		return -1;
	}
	else
	{
		NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

		if (netplayManager->hasReceivedPostPracticeRaceCustomLobbyID)
		{
			//I have the lobby ID at this point
			//customMatchManager->TryEnterLobbyFromPostPracticeInvite
			//how do I get from here to the waiting room
		}
	}
}

void PostPracticeMatchMenu::Draw(sf::RenderTarget *target)
{
	if (action == A_NORMAL)
	{
		textMenu->Draw(target);
		readyButton->Draw(target);
		inviteButton->Draw(target);
		acceptButton->Draw(target);
		target->draw(otherPlayerTestQuad, 4, sf::Quads);
	}

	else if (action == A_ACCEPT_POPUP)
	{
		acceptPopup->Draw(target);
	}

	else if (action == A_WAIT_FOR_LOBBY)
	{
		target->draw(waitForLobbyText);
	}
	


}