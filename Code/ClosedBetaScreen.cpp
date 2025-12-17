#include "ClosedBetaScreen.h"
#include "MainMenu.h"
#include "Input.h"
#include "Fader.h"
#include "SinglePlayerControllerJoinScreen.h"

using namespace std;
using namespace sf;

ClosedBetaScreen::ClosedBetaScreen()
{
	MainMenu *mm = MainMenu::GetInstance();

	SetRectColor(quad, Color::Black);
	SetRectTopLeft(quad, 1920, 1080, Vector2f(0, 0));

	action = A_IDLE;
	frame = 0;
	 
	ts_closedBeta = GetSizedTileset("Menu/EarlyAccess/early_access_screen_1920x1080.png");
	ts_closedBeta->SetSpriteTexture(closedBetaSpr);
	ts_closedBeta->SetSubRect(closedBetaSpr, 0);
	closedBetaSpr.setPosition(0, 0);

	closedBetaText.setFont(mm->arial);
	closedBetaText.setCharacterSize(40);
	closedBetaText.setString("The following is a WORK IN PROGRESS. All assets, levels, and mechanics are subject to change.\nWe recommend using a controller. "
		"Thanks for playing and we look forward to your feedback!\n");
	auto lb = closedBetaText.getLocalBounds();
	closedBetaText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	closedBetaText.setPosition(Vector2f(960, 540));
	closedBetaText.setFillColor(Color::White);


	ts_splash = GetSizedTileset("Menu/splash_1920x1080.png");
	splashSpr.setTexture(*ts_splash->texture);

	splashText.setFont(mm->arial);
	splashText.setCharacterSize(40);
	splashText.setString("Press any button to start!");
	lb = splashText.getLocalBounds();
	splashText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	splashText.setPosition(Vector2f(960, 640));
	splashText.setFillColor(Color::White);
}

void ClosedBetaScreen::Update()
{
	SinglePlayerControllerJoinScreen *js = MainMenu::GetInstance()->singlePlayerControllerJoinScreen;
	assert(js != NULL);

	if (action == A_IDLE )
	{
		if (frame == 60 * 5)
		{
			action = A_SPLASH_TRANSITION;
			frame = 0;
		}
		else
		{
			if (CONTROLLERS.ButtonPressed_Any() && frame > 60)
			{
				action = A_SPLASH_TRANSITION;
				frame = 0;
				MainMenu *mm = MainMenu::GetInstance();
				mm->fader->CrossFade(30, 0, 30, Color::Black);
			}
		}
	}
	else if (action == A_SPLASH_TRANSITION)
	{
		if (frame == 60)
		{
			action = A_SPLASH;
			frame = 0;
		}
	}
	else if (action == A_SPLASH)
	{

		//Update()

		//if (frame > 10)
		{
			js->ClosedBetaUpdate();
			if (js->action == SinglePlayerControllerJoinScreen::A_READY)
			{
				action = A_DONE;
				frame = 0;
			}
		}

		/*if (CONTROLLERS.ButtonPressed_Any() && frame > 60)
		{
			action = A_DONE;
			frame = 0;
		}*/

		int fadeCycle = 120;
		int fadeFrame = frame % fadeCycle;
		
		if (fadeFrame < fadeCycle / 2)
		{
			float factor = (float)fadeFrame / ( fadeCycle / 2 );
			Color c = Color::White;
			c.a = 255 * factor;
			splashText.setFillColor(c);
		}
		else
		{
			float factor = (float)( fadeFrame - (fadeCycle / 2 )) / (fadeCycle / 2);
			Color c = Color::White;
			c.a = 255 - ( 255 * factor );
			splashText.setFillColor(c);
		}

	}

	++frame;
}

void ClosedBetaScreen::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);

	if (action == A_IDLE)
	{
		target->draw(closedBetaSpr);
		target->draw(closedBetaText);
	}
	else if (action == A_SPLASH_TRANSITION)
	{
		if (frame < 30)
		{
			target->draw(closedBetaSpr);
			target->draw(closedBetaText);
		}
		else
		{
			target->draw(splashSpr);
			target->draw(splashText);
		}
	}
	else if (action == A_SPLASH || action == A_DONE )
	{
		target->draw(splashSpr);
		target->draw(splashText);
	}
}