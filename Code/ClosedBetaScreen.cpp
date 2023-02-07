#include "ClosedBetaScreen.h"
#include "MainMenu.h"
#include "Input.h"

using namespace std;
using namespace sf;

ClosedBetaScreen::ClosedBetaScreen()
{
	MainMenu *mm = MainMenu::GetInstance();

	text.setFont(mm->arial);
	text.setCharacterSize(40);
	text.setString("The following is a WORK IN PROGRESS. All assets, levels, and mechanics are subject to change.\nWe recommend using a controller. "
		"Thanks for playing and we look forward to your feedback!\n");
	auto lb = text.getLocalBounds();
	text.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	text.setPosition(Vector2f(960, 540));
	
	text.setFillColor(Color::White);

	SetRectColor(quad, Color::Black);
	SetRectTopLeft(quad, 1920, 1080, Vector2f(0, 0));

	action = A_IDLE;
	frame = 0;
}

void ClosedBetaScreen::Update()
{
	if (action == A_IDLE )
	{
		if (frame == 60 * 5)
		{
			action = A_DONE;
			frame = 0;
		}
		else
		{
			if (CONTROLLERS.ButtonPressed_Any() && frame > 60)
			{
				action = A_DONE;
				frame = 0;
			}
		}
	}

	++frame;
}

void ClosedBetaScreen::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);
	target->draw(text);
}