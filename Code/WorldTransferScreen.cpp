#include "WorldTransferScreen.h"
#include "MainMenu.h"
#include "Fader.h"
#include "VectorMath.h" 
#include "WorldMap.h"
#include "MusicPlayer.h"
#include "AdventureManager.h"
#include "ShardAndLogDisplay.h"


using namespace std;
using namespace sf;

WorldTransferScreen::WorldTransferScreen()
	:skinShader(PlayerSkinShader::ST_BOOST)
{
	ts_bg = GetSizedTileset("Menu/WorldTransfer/world_transfer_bg_1920x1080.png");
	ts_planet = GetSizedTileset("Menu/WorldTransfer/world_transfer_world_1920x310.png");
	ts_bgShape = GetTileset("Menu/KinBoost/kinboost_BG1_shape.png", 1920, 1080);

	//ts_light[0] = GetTileset("Menu/KinBoost/kinboost_light_01a.png", 1920, 1080);
	//ts_light[1] = GetTileset("Menu/KinBoost/kinboost_light_01b.png", 1920, 1080);

	/*ts_stars[0] = GetTileset("Menu/KinBoost/kinboost_stars_01a.png", 1920, 1080);
	ts_stars[1] = GetTileset("Menu/KinBoost/kinboost_stars_01b.png", 1920, 1080);
	ts_stars[2] = GetTileset("Menu/KinBoost/kinboost_stars_01c.png", 1920, 1080);
	ts_stars[3] = GetTileset("Menu/KinBoost/kinboost_stars_01d.png", 1920, 1080);*/

	ts_stars[0] = GetSizedTileset("Menu/WorldTransfer/world_transfer_stars_1920x1080.png");

	ts_planet->SetSpriteTexture(planetSpr);
	planetSpr.setPosition(0, 1080 - planetSpr.getGlobalBounds().height);

	ts_ship = GetSizedTileset("Ship/ship_864x410.png");
	ts_ship->SetSpriteTexture(shipSpr);
	shipSpr.setOrigin(shipSpr.getLocalBounds().width / 2, shipSpr.getLocalBounds().height / 2);

	shipExitLength = 120;

	MainMenu *mm = MainMenu::GetInstance();

	worldText.setFont(mm->arial);
	worldText.setCharacterSize(60);
	worldText.setFillColor(Color::White);
	worldText.setOutlineColor(Color::Black);
	worldText.setOutlineThickness(-2);

	shipStart = Vector2f(960, 540);
	shipEnd = Vector2f(1920 + 500, 540);

	shipSpr.setPosition(shipStart);

	for (int i = 0; i < 4; ++i)
	{
		starAccel[i] = .001;
		starMax[i] = 1.f / 30.f;
	}

	for (int i = 0; i < 2; ++i)
	{
		lightAccel[i] = .001;
	}

	lightMax[0] = 1.f / 30.f;
	lightMax[1] = 1.f / 40.f;

	bgSpr.setTexture(*ts_bg->texture);
	bgShapeSpr.setTexture(*ts_bgShape->texture);

	for (int i = 0; i < 1; ++i)
	{
		starSpr[i].setTexture(*ts_stars[i]->texture);
	}

	/*for (int i = 0; i < 2; ++i)
	{
		lightSpr[i].setTexture(*ts_light[i]->texture);
	}*/

	for (int i = 0; i < 1; ++i)
	{
		if (!scrollShaderStars[i].loadFromFile("Resources/Shader/horizslider.frag", sf::Shader::Fragment))
		{
			assert(0);
		}
		scrollShaderStars[i].setUniform("u_texture", sf::Shader::CurrentTexture);
	}

	/*for (int i = 0; i < 2; ++i)
	{
		if (!scrollShaderLight[i].loadFromFile("Resources/Shader/horizslider.frag", sf::Shader::Fragment))
		{
			assert(0);
		}
		scrollShaderLight[i].setUniform("u_texture", sf::Shader::CurrentTexture);
	}*/
}

void WorldTransferScreen::End()
{
	action = A_FINISHBOOST;
	frame = 0;

	auto *mainMenu = MainMenu::GetInstance();
	//mainMenu->fader->CrossFade(30, 0, 30, Color::Black, true);
	//mainMenu->fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);// true);
	mainMenu->musicPlayer->FadeOutCurrentMusic(30);
	//frame = 0;
}

void WorldTransferScreen::SetWorld(int wIndex)
{
	currWorld = wIndex;
	worldText.setString("Flying to World " + to_string(currWorld + 1) + "...");
	auto lb = worldText.getLocalBounds();
	worldText.setOrigin(lb.left + lb.width / 2, lb.top + lb.height / 2);
	worldText.setPosition(960, 200);
}

void WorldTransferScreen::Reset()
{
	frame = 0;
	action = A_STARTING;
	ended = false;

	shipSpr.setPosition(shipStart);

	auto *mainMenu = MainMenu::GetInstance();

	if (mainMenu->adventureManager != NULL)
	{
		skinShader.SetSkin(mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex);
	}
	else if (mainMenu->rushManager != NULL)
	{
		skinShader.SetSkin(0);//mainMenu->adventureManager->currSaveFile->visualInfo.skinIndex);
	}
	else
	{
		skinShader.SetSkin(0);
	}

	

	for (int i = 0; i < 4; ++i)
	{
		starFac[i] = 0;
	}

	for (int i = 0; i < 2; ++i)
	{
		lightFac[i] = 0;
	}

	starSpeed[0] = 1.f / 10000.f;
	starSpeed[1] = 1.f / 10000.f;
	starSpeed[2] = 1.f / 10000.f;
	starSpeed[3] = 1.f / 10000.f;

	lightSpeed[0] = 1.f / 10000.f;
	lightSpeed[1] = 1.f / 10000.f;
}

void WorldTransferScreen::Update()
{
	auto *mainMenu = MainMenu::GetInstance();

	switch (action)
	{
	case A_STARTING:
		action = A_BOOSTING;
		frame = 0;
		mainMenu->fader->Fade(true, 120, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);
		break;
	case A_BOOSTING:
		break;
	case A_ENDING:
		//if( mainMenu->swiper->IsPostWipe())
		break;
	}


	if (action != A_STARTING)
	{
		/*int scrollFramesBack = 40;
		int scrollFramesFront = 30;
		float fBack = frame % scrollFramesBack;
		float fFront = frame % scrollFramesFront;

		int mult = frame / scrollFramesBack;
		int multFront = frame / scrollFramesFront;

		float facBack = fBack / (scrollFramesBack);
		float facFront = fFront / (scrollFramesFront);*/

		if (frame == 4)
		{
			//mainMenu->ActivateIndEffect(
			//	mainMenu->tilesetManager.GetTileset("Kin/exitenergy_2_512x512.png", 512, 512), V2d(960, 540), false, 0, 6, 2, true);
		}

		for (int i = 0; i < 4; ++i)
		{
			scrollShaderStars[i].setUniform("quant", starFac[i]);//0.f);
		}

		for (int i = 0; i < 2; ++i)
		{
			scrollShaderLight[i].setUniform("quant", lightFac[i]);//0.f);
		}

		if (action == A_BOOSTING)
		{
			float bgFade = 360;
			int bgFadeI = bgFade;
			if (frame <= bgFadeI)
			{
				bgSpr.setColor(Color(255, 255, 255, (frame / bgFade) * 255.f));
			}
		}

		if (action == A_FINISHBOOST && frame == shipExitLength)//&& mainMenu->fader->IsFullyFadedOut())
		{
			auto *mainMenu = MainMenu::GetInstance();
			//mainMenu->fader->CrossFade(30, 0, 30, Color::Black, true);
			mainMenu->fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);// true);
			action = A_ENDING;
			frame = 0;
			//ended
			//state = ENDING;

		}
		else if (action == A_FINISHBOOST)
		{
			float factor = ((float)frame) / shipExitLength;
			shipSpr.setPosition(shipStart * (1.f - factor) + shipEnd * factor);
		}

		if (action == A_ENDING)
		{
			if (mainMenu->fader->IsFullyFadedOut())
			{
				ended = true;
			}
		}

	}

	for (int i = 0; i < 4; ++i)
	{
		starFac[i] += starSpeed[i];
		if (starFac[i] > 1)
			starFac[i] -= 1;
		if (starFac[i] < 0)
		{
			starFac[i] += 1;
		}

		starSpeed[i] += starAccel[i];
		if (starSpeed[i] > starMax[i])
		{
			starSpeed[i] = starMax[i];
		}
	}

	for (int i = 0; i < 2; ++i)
	{
		lightFac[i] += lightSpeed[i];
		if (lightFac[i] > 1)
			lightFac[i] -= 1;
		if (lightFac[i] < 0)
		{
			lightFac[i] += 1;
		}

		lightSpeed[i] += lightAccel[i];
		if (lightSpeed[i] > lightMax[i])
		{
			lightSpeed[i] = lightMax[i];
		}
	}

	++frame;


}

bool WorldTransferScreen::IsEnded()
{
	return ended;
}

bool WorldTransferScreen::IsBoosting()
{
	return action == A_BOOSTING;
}

void WorldTransferScreen::Draw(RenderTarget *target)
{
	switch (action)
	{
	case A_STARTING:
	{
		//target->draw(swipeSpr);
		break;
	}
	case A_ENDING:
	case A_FINISHBOOST:
	case A_BOOSTING:
	{
		target->draw(bgSpr);

		if (frame < 120)
		{
			//target->draw(kinSpr);
			//return;
		}
		//target->draw(bgShapeSpr);
		//for (int i = 3; i >= 0; --i)

		{
			target->draw(starSpr[0], &scrollShaderStars[0]);
		}

		/*for (int i = 1; i >= 0; --i)
		{
			target->draw(lightSpr[i], &scrollShaderLight[i]);
		}*/

		target->draw(planetSpr);

		target->draw(shipSpr);

		DrawLevelInfo(target);

		target->draw(worldText);

		break;
	}
	}

}