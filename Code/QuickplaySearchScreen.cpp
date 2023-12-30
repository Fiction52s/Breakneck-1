#include "QuickplaySearchScreen.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

QuickplaySearchScreen::QuickplaySearchScreen()
{
	SetRectColor(bgQuad, Color::Red);
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	Reset();
}

void QuickplaySearchScreen::Reset()
{
	action = A_SEARCHING;
	frame = 0;
}

bool QuickplaySearchScreen::IsCanceled()
{
	return action == A_CANCELED;
}

bool QuickplaySearchScreen::IsMatchFound()
{
	return action == A_FOUND;
}

void QuickplaySearchScreen::Update()
{
	//no update for now
}

void QuickplaySearchScreen::Draw(sf::RenderTarget *target)
{
	target->draw(bgQuad, 4, sf::Quads);
}