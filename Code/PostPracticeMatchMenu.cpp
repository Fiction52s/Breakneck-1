#include "PostPracticeMatchMenu.h"
#include "BasicTextMenu.h"
#include "KeepPlayingButton.h"

using namespace std;
using namespace sf;

PostPracticeMatchMenu::PostPracticeMatchMenu()
{
	vector<string> practiceHostOptions = { "Invite to Custom Lobby", "Leave" };
	textMenu = new BasicTextMenu(practiceHostOptions);

	keepPlayingButton = new KeepPlayingButton;
	keepPlayingButton->SetCenter(Vector2f(960, 300));
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
}

bool PostPracticeMatchMenu::WantsToKeepPlaying()
{
	return keepPlayingButton->action == KeepPlayingButton::A_ON;
}

int PostPracticeMatchMenu::Update()
{
	keepPlayingButton->Update();

	int textResult = textMenu->Update();

	return textResult;
}

void PostPracticeMatchMenu::Draw(sf::RenderTarget *target)
{
	textMenu->Draw(target);
	keepPlayingButton->Draw(target);
}