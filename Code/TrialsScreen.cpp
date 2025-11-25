#include "TrialsScreen.h"
#include "MainMenu.h"
#include "Input.h"
#include "Fader.h"
#include "TrialsManager.h"
#include "WorldMap.h"
#include "Fader.h"
#include "DrawLayer.h"
#include "md5.h"
#include "Leaderboard.h"

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

		if( worldMap->state == WorldMap::TRIAL_COLONY )
		{
			action = A_LEVEL_SELECT;
			frame = 0;

			MainMenu *mm = MainMenu::GetInstance();
			//mm->fader->Fade(true, 30, Color::Black, false, DrawLayer::IN_FRONT);
		}
	}
	else if (action == A_LEVEL_SELECT)
	{
			/*if (frame == 60 * 5)
			{
				action = A_DONE;
				frame = 0;
			}
			else*/
			{
				
				//if (CONTROLLERS.ButtonPressed_B() && frame > 60)
				if (CONTROLLERS.ButtonPressed_A())
				{
					action = A_RUN_LEVEL;
					frame = 0;
				}
				else if (CONTROLLERS.ButtonPressed_B())
				{
					action = A_DONE;
					frame = 0;
					//MainMenu *mm = MainMenu::GetInstance();
					//mm->fader->CrossFade(30, 0, 30, Color::Black);
				}
				else if (CONTROLLERS.DirPressed_Down())
				{

				}
				else if (CONTROLLERS.ButtonPressed_RightShoulder())
				{
					action = A_LEADERBOARD;

					int mapIndex = 0;

					string filePathStr = trialsMan->rushFile.worlds[trialsMan->currWorld].maps[0].GetMapPath();//string("Resources\\Maps\\") + saveFile->adventureFile->GetMap(level->index).GetFilePath() + string(MAP_EXT);

					string myHash = md5file(filePathStr);

					trialsMan->SetBoards(mapIndex, myHash);

					//trialsMan->leaderboard->SetAnyPowersMode(true);

					trialsMan->leaderboard->Start();
				}
				
			}
	}
	else if (action == A_LEADERBOARD)
	{
		if (CONTROLLERS.ButtonPressed_B())
		{
			action = A_LEVEL_SELECT;
			frame = 0;
			trialsMan->leaderboard->Hide();
			//MainMenu *mm = MainMenu::GetInstance();
			//mm->fader->CrossFade(30, 0, 30, Color::Black);
		}
		else
		{
			trialsMan->leaderboard->Update();//controllerInput->GetPrevState(), controllerInput->GetCurrState());

			//if (ms->mainMenu->adventureManager->leaderboard->IsTryingToStartReplay())
			//{
			//	state = LEADERBOARD_STARTING;
			//	//currLevel->TryStartLeaderboardReplay(adventureManager->leaderboard->replayChosen);
			//	//ms->mainMenu->adventureManager->leaderboard->Hide();

			//	//this is because the Hide() call for leaderboard happens in the other thread while loading, so the mouse won't disappear
			//	MOUSE.Hide();
			//	MOUSE.SetControllersOn(false);
			//	return false;
			//}
			//else if (ms->mainMenu->adventureManager->leaderboard->IsTryingToRaceGhosts())
			//{
			//	state = LEADERBOARD_STARTING;
			//	MOUSE.Hide();
			//	MOUSE.SetControllersOn(false);
			//	return false;
			//}
		}
	}
	//if (action == A_IDLE)
	//{
	

	++frame;
}

bool TrialsScreen::IsRunningMap()
{
	return action == A_RUN_LEVEL;
}

void TrialsScreen::Draw(sf::RenderTarget *target)
{
	target->draw(quad, 4, sf::Quads);

	if (action == A_LEVEL_SELECT || action == A_DONE || action == A_RUN_LEVEL || action == A_LEADERBOARD )
	{
		target->draw(closedBetaSpr);
		target->draw(closedBetaText);

		if (action == A_LEADERBOARD)
		{
			MainMenu *mm = MainMenu::GetInstance();
			auto *pauseTex = mm->pauseTexture;
			pauseTex->clear(Color::Transparent);
			trialsMan->leaderboard->Draw(pauseTex);

			pauseTex->display();
			Sprite pauseMenuSprite;
			pauseMenuSprite.setTexture(pauseTex->getTexture());
			pauseMenuSprite.setPosition(0, 0);//960 / 2, 540 / 2);//(1920 - 1820) / 4 - 960 / 2, (1080 - 980) / 4 - 540 / 2)
			target->draw(pauseMenuSprite);

		}

	}
	else if (action == A_WORLD_MAP)
	{
		worldMap->Draw(target);
	}
}