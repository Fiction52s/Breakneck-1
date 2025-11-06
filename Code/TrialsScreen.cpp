#include "TrialsScreen.h"
#include "MainMenu.h"
#include "Input.h"
#include "Fader.h"
#include "TrialsManager.h"
#include "WorldMap.h"

using namespace std;
using namespace sf;

TrialsScreen::TrialsScreen()
{
	MainMenu *mm = MainMenu::GetInstance();

	SetRectColor(quad, Color::Black);
	SetRectTopLeft(quad, 1920, 1080, Vector2f(0, 0));

	trialsMan = NULL;

	worldMap = new WorldMap;

	action = A_WORLD_MAP;
	frame = 0;

	ts_closedBeta = GetSizedTileset("Menu/EarlyAccess/early_access_screen_1920x1080.png");
	ts_closedBeta->SetSpriteTexture(closedBetaSpr);
	ts_closedBeta->SetSubRect(closedBetaSpr, 0);
	closedBetaSpr.setPosition(0, 0);

	closedBetaText.setFont(mm->arial);
	closedBetaText.setCharacterSize(40);
	closedBetaText.setString("Testing trials mode!\n");
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

TrialsScreen::~TrialsScreen()
{
	delete worldMap;
}

void TrialsScreen::Reset()
{
	action = A_WORLD_MAP;
	frame = 0;
}

void TrialsScreen::Update()
{
	if (action == A_WORLD_MAP)
	{
		worldMap->Update();
	}
	return;

	//if (action == A_IDLE)
	//{
	//	if (frame == 60 * 5)
	//	{
	//		action = A_DONE;
	//		frame = 0;
	//	}
	//	else
	//	{
	//		
	//		//if (CONTROLLERS.ButtonPressed_B() && frame > 60)
	//		if (CONTROLLERS.ButtonPressed_A())
	//		{
	//			action = A_RUN_MAP;
	//			frame = 0;
	//		}
	//		else if (CONTROLLERS.ButtonPressed_B())
	//		{
	//			action = A_DONE;
	//			frame = 0;
	//			//MainMenu *mm = MainMenu::GetInstance();
	//			//mm->fader->CrossFade(30, 0, 30, Color::Black);
	//		}
	//		else if (CONTROLLERS.DirPressed_Down())
	//		{

	//		}
	//		
	//	}
	//}

	++frame;
}

bool TrialsScreen::IsRunningMap()
{
	return action == A_RUN_LEVEL;
}

void TrialsScreen::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);

	/*if (action == A_IDLE || action == A_DONE || action == A_RUN_MAP )
	{
		target->draw(closedBetaSpr);
		target->draw(closedBetaText);
	}*/
	worldMap->Draw(target);
}