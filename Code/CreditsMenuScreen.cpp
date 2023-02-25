#include "MainMenu.h"
#include "Tileset.h"
#include "CreditsMenuScreen.h"
#include "UIMouse.h"

using namespace sf;
using namespace std;

CreditsMenuScreen::CreditsMenuScreen()
{
	menuOffset = Vector2f(0, 0);
	ts = GetSizedTileset("Menu/credits_960x540.png");

	SetRectTopLeft(creditsQuad, 1920, 1080, Vector2f(0, 0));
	SetRectSubRect(creditsQuad, ts->GetSubRect(0));

	//HandleEvent isn't currently getting called anywhere! know this is if you add the twitter symbols etc!


	panel = new Panel("creditspanel", 1920, 1080, this);

	Start();
}

CreditsMenuScreen::~CreditsMenuScreen()
{
	delete panel;
}

void CreditsMenuScreen::Start()
{
	action = A_SHOW;
	frame = 0;
}

//isnt currently getting called anywhere! know this is if you add the twitter symbols etc!
bool CreditsMenuScreen::HandleEvent(sf::Event ev)
{
	return panel->HandleEvent(ev);
}

void CreditsMenuScreen::ConfirmCallback(Panel *p)
{
	Cancel();
}

void CreditsMenuScreen::Cancel()
{
	action = A_BACK;
	frame = 0;
}

void CreditsMenuScreen::CancelCallback(Panel *p)
{
	Cancel();
}

void CreditsMenuScreen::Update()
{
	if( CONTROLLERS.ButtonPressed_Any() )
	{
		Cancel();
		return;
	}

	if (MOUSE.IsMouseLeftClicked() || MOUSE.IsMouseRightClicked())
	{
		Cancel();
		return;
	}

	panel->MouseUpdate();
}

void CreditsMenuScreen::Draw(sf::RenderTarget *target)
{
	target->draw(creditsQuad, 4, sf::Quads, ts->texture);
}