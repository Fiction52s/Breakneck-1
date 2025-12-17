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
#include "KinStore.h"

using namespace std;
using namespace sf;

TrialsScreen::TrialsScreen()
{
	MainMenu *mm = MainMenu::GetInstance();

	//SetRectColor(quad, Color::White);
	SetRectTopLeft(bgQuad, 1920, 1080, Vector2f(0, 0));

	trialsMan = NULL;
	ts_mapPreview = NULL;

	worldMap = new WorldMap;

	action = A_WORLD_MAP;
	frame = 0;

	SetRectCenter(confirmQuad, 500, 500, Vector2f(960, 540));
	SetRectColor(confirmQuad, Color::Red);

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

	Vector2f textStart(100, 100);
	Vector2f textSpace(0, 70);
	for (int i = 0; i < MAX_LEVELS_PER_WORLD; ++i)
	{
		levelNameText[i].setFont(mm->arial);
		levelNameText[i].setCharacterSize(20);
		levelNameText[i].setString("");
		//auto lb = closedBetaText.getLocalBounds();
		//closedBetaText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
		levelNameText[i].setPosition(textStart + Vector2f( 0, textSpace.y * i ));
		levelNameText[i].setFillColor(Color::White);
	}

	ts_bg = GetSizedTileset("Menu/Title/title_screen_bg_410x327.png");

	if (!scrollShader.loadFromFile("Resources/Shader/slider.frag", sf::Shader::Fragment))
	{
		assert(0);
	}
	scrollShader.setUniform("u_texture", *ts_bg->texture);

	xRate = -.005;
	yRate = -.002;

	SetRectSubRect(bgQuad, sf::FloatRect(0, 0, 1920.f / ts_bg->tileWidth, 1080.f / ts_bg->tileHeight));

	tintColors[0] = Color(0x4d, 0x89, 0xc2);
	tintColors[1] = Color(0x4b, 0xa2, 0x33);
	tintColors[2] = Color(0xb4, 0xa5, 0x3e);
	tintColors[3] = Color(0xc5, 0x99, 0x5b);
	tintColors[4] = Color(0xc6, 0x6f, 0x6f);
	tintColors[5] = Color(0xaf, 0x6d, 0xb0);
	tintColors[6] = Color(0x83, 0x44, 0xc4);
}

TrialsScreen::~TrialsScreen()
{
	delete worldMap;
}

void TrialsScreen::Reset()
{
	action = A_WORLD_MAP;
	frame = 0;
	selectedMapIndex = 0;
	trialsMan->currLevelIndex = selectedMapIndex;
	UpdateMapPreview();

	quantX = 0;
	quantY = 0;

}

void TrialsScreen::Update()
{
	quantX += xRate;
	quantY += yRate;

	scrollShader.setUniform("quantX", quantX);
	scrollShader.setUniform("quantY", quantY);

	if (action == A_WORLD_MAP)
	{
		worldMap->Update();

		if( worldMap->state == WorldMap::TRIAL_COLONY )
		{
			action = A_LEVEL_SELECT;
			frame = 0;

			trialsMan->currWorld = worldMap->selectedColony;

			MainMenu *mm = MainMenu::GetInstance();

			for (int i = 0; i < MAX_LEVELS_PER_WORLD; ++i)
			{
				levelNameText[i].setString(trialsMan->GetLeaderboardDisplayName( i ));
			}

			scrollShader.setUniform("tintColor", ColorGL(tintColors[trialsMan->currWorld]));
		}
	}
	else if (action == A_LEVEL_SELECT)
	{
		if (CONTROLLERS.DirPressed_Down())
		{
			selectedMapIndex++;
			if (selectedMapIndex == MAX_LEVELS_PER_WORLD)
			{
				selectedMapIndex = 0;
			}
			trialsMan->currLevelIndex = selectedMapIndex;
			UpdateMapPreview();
		}
		else if (CONTROLLERS.DirPressed_Up())
		{
			selectedMapIndex--;
			if (selectedMapIndex < 0)
			{
				selectedMapIndex = MAX_LEVELS_PER_WORLD - 1;
			}
			trialsMan->currLevelIndex = selectedMapIndex;
			UpdateMapPreview();
		}

		for (int i = 0; i < MAX_LEVELS_PER_WORLD; ++i)
		{
			levelNameText[i].setFillColor(Color::White);
		}
		levelNameText[selectedMapIndex].setFillColor(Color::Red);
		

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
				action = A_CONFIRM_POWERS;
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
			else if (CONTROLLERS.ButtonPressed_Y())
			{
				action = A_LEADERBOARD;

				string filePathStr = trialsMan->rushFile.worlds[trialsMan->currWorld].maps[trialsMan->currLevelIndex].GetMapPath();//string("Resources\\Maps\\") + saveFile->adventureFile->GetMap(level->index).GetFilePath() + string(MAP_EXT);

				string myHash = md5file(filePathStr);

				trialsMan->SetBoards(trialsMan->currLevelIndex, myHash);

				//trialsMan->leaderboard->SetAnyPowersMode(true);

				trialsMan->leaderboard->Start();
			}
			else if (CONTROLLERS.ButtonPressed_X())
			{
				action = A_STORE;

				trialsMan->kinStore->Open();
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

			if (trialsMan->leaderboard->IsTryingToStartReplay())
			{
				action = A_CONFIRM_POWERS;
				frame = 0;

				//currLevel->TryStartLeaderboardReplay(adventureManager->leaderboard->replayChosen);
			}

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
	else if (action == A_STORE)
	{
		if (CONTROLLERS.ButtonPressed_B())
		{
			action = A_LEVEL_SELECT;
			frame = 0;
			//trialsMan->kinStore->Hide();
			//MainMenu *mm = MainMenu::GetInstance();
			//mm->fader->CrossFade(30, 0, 30, Color::Black);
		}
		else
		{
			trialsMan->kinStore->Update();
		}
	}
	else if (action == A_CONFIRM_POWERS)
	{
		if (CONTROLLERS.ButtonPressed_A())
		{
			action = A_RUN_LEVEL;
			frame = 0;

		}
		else if (CONTROLLERS.ButtonPressed_B())
		{
			action = A_LEVEL_SELECT;
			frame = 0;
			//MainMenu *mm = MainMenu::GetInstance();
			//mm->fader->CrossFade(30, 0, 30, Color::Black);
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

void TrialsScreen::DestroyMapPreview()
{
	//cout << "destroyed preview" << endl;
	if (ts_mapPreview != NULL)
	{
		DestroyTileset(ts_mapPreview);
		ts_mapPreview = NULL;
	}
}

void TrialsScreen::UpdateMapPreview()
{
	DestroyMapPreview();


	bool allSecretsCollected = false;

	/*Level *level = GetSelectedLevel();
	AdventureMapHeaderInfo &headerInfo =
		saveFile->adventureFile->GetMapHeaderInfo(level->index);*/

	int totalNumShards = 0;//headerInfo.shardInfoVec.size();

	int numCollected = 0;
	/*for (auto it = headerInfo.shardInfoVec.begin(); it !=
	headerInfo.shardInfoVec.end(); ++it)
	{
	if (saveFile->IsShardCaptured((*it).GetTrueIndex()))
	{
	numCollected++;
	}
	}*/

	if (numCollected == totalNumShards)
	{
		allSecretsCollected = true;
	}

	string fPath = trialsMan->rushFile.GetMap(trialsMan->currWorld, trialsMan->currLevelIndex).GetFilePath();//adventureFile.GetAdventureSector(sec).maps[GetSelectedIndex()].GetFilePath();
	string previewPath;
	if (allSecretsCollected)
	{
		previewPath = "Maps\\" + fPath + ".png";
	}
	else
	{
		previewPath = "Maps\\" + fPath + "_basic.png";
	}


	ts_mapPreview = GetTileset(previewPath, 912, 492);
	mapPreviewSpr.setTexture(*ts_mapPreview->texture);
	mapPreviewSpr.setOrigin(mapPreviewSpr.getLocalBounds().width / 2, mapPreviewSpr.getLocalBounds().height / 2);
	mapPreviewSpr.setPosition(1300, 500);
}

void TrialsScreen::Draw(sf::RenderTarget *target)
{
	//target->draw(quad, 4, sf::Quads);
	target->draw(bgQuad, 4, sf::Quads, &scrollShader);

	if (action == A_LEVEL_SELECT || action == A_DONE || action == A_RUN_LEVEL || action == A_LEADERBOARD || action == A_STORE || action == A_CONFIRM_POWERS)
	{
		//target->draw(closedBetaSpr);
		//target->draw(closedBetaText);

		for (int i = 0; i < MAX_LEVELS_PER_WORLD; ++i)
		{
			target->draw(levelNameText[i]);
		}

		target->draw(mapPreviewSpr);

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
		else if (action == A_STORE)
		{
			trialsMan->kinStore->Draw(target);
		}
		else if (action == A_CONFIRM_POWERS)
		{
			target->draw(confirmQuad, 4, sf::Quads);
		}

	}
	else if (action == A_WORLD_MAP)
	{
		worldMap->Draw(target);
	}
}