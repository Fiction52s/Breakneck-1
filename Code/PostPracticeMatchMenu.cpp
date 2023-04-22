#include "PostPracticeMatchMenu.h"
#include "BasicTextMenu.h"
#include "SingleInputMenuButton.h"
#include "NetplayManager.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

PostPracticeMatchMenu::PostPracticeMatchMenu()
{
	vector<string> practiceHostOptions = { "Invite to Custom Lobby", "Leave" };
	textMenu = new BasicTextMenu(practiceHostOptions);

	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	keepPlayingButton = new SingleInputMenuButton( "Ready", Vector2f( 500, 200 ), XBoxButton::XBOX_START, 64 );
	keepPlayingButton->SetCenter(Vector2f(960, 300));

	SetRectTopLeft(otherPlayerTestQuad, 100, 100, Vector2f(0, 0));
}

PostPracticeMatchMenu::~PostPracticeMatchMenu()
{
	delete textMenu;
	delete keepPlayingButton;
}

void PostPracticeMatchMenu::Reset()
{
	textMenu->Reset();
	keepPlayingButton->Reset();

	SetRectColor(otherPlayerTestQuad, Color::Red);
}

bool PostPracticeMatchMenu::WantsToKeepPlaying()
{
	return keepPlayingButton->action == SingleInputMenuButton::A_ON;
}

int PostPracticeMatchMenu::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	bool oldKeepPlayingOn = WantsToKeepPlaying();

	keepPlayingButton->Update( CONTROLLERS.ButtonPressed_Start(), CONTROLLERS.ButtonPressed_B() );

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

	int textResult = textMenu->Update();

	return textResult;
}

void PostPracticeMatchMenu::Draw(sf::RenderTarget *target)
{
	textMenu->Draw(target);
	keepPlayingButton->Draw(target);
	target->draw(otherPlayerTestQuad, 4, sf::Quads);
}