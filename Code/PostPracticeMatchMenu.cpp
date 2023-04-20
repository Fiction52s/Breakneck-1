#include "PostPracticeMatchMenu.h"
#include "BasicTextMenu.h"
#include "KeepPlayingButton.h"
#include "NetplayManager.h"
#include "MainMenu.h"

using namespace std;
using namespace sf;

PostPracticeMatchMenu::PostPracticeMatchMenu()
{
	vector<string> practiceHostOptions = { "Invite to Custom Lobby", "Leave" };
	textMenu = new BasicTextMenu(practiceHostOptions);

	keepPlayingButton = new KeepPlayingButton;
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
	return keepPlayingButton->action == KeepPlayingButton::A_ON;
}

int PostPracticeMatchMenu::Update()
{
	NetplayManager *netplayManager = MainMenu::GetInstance()->netplayManager;

	bool oldKeepPlayingOn = WantsToKeepPlaying();

	keepPlayingButton->Update();

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