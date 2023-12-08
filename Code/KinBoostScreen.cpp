#include "KinBoostScreen.h"
#include "MainMenu.h"
#include "Fader.h"
#include "VectorMath.h" 
#include "WorldMap.h"
#include "MusicPlayer.h"
#include "AdventureManager.h"


using namespace std;
using namespace sf;

KinBoostScreen::KinBoostScreen()
	:skinShader(PlayerSkinShader::ST_BOOST)
{
	ts_bg = GetTileset("Menu/KinBoost/kinboost_BG1.png", 1920, 1080);
	ts_bgShape = GetTileset("Menu/KinBoost/kinboost_BG1_shape.png", 1920, 1080);

	ts_light[0] = GetTileset("Menu/KinBoost/kinboost_light_01a.png", 1920, 1080);
	ts_light[1] = GetTileset("Menu/KinBoost/kinboost_light_01b.png", 1920, 1080);

	ts_stars[0] = GetTileset("Menu/KinBoost/kinboost_stars_01a.png", 1920, 1080);
	ts_stars[1] = GetTileset("Menu/KinBoost/kinboost_stars_01b.png", 1920, 1080);
	ts_stars[2] = GetTileset("Menu/KinBoost/kinboost_stars_01c.png", 1920, 1080);
	ts_stars[3] = GetTileset("Menu/KinBoost/kinboost_stars_01d.png", 1920, 1080);

	ts_kinBoost = GetSizedTileset("Kin/exitboost_96x128.png");
	ts_kinAura = GetSizedTileset("Kin/FX/exitaura_256x256.png");
	ts_enterFX = GetSizedTileset("Kin/FX/enter_fx_320x320.png");

	MainMenu *mm = MainMenu::GetInstance();

	ts_statIcons = GetSizedTileset("HUD/score_icons_128x96.png");

	ts_statIcons->SetSpriteTexture(shardIconSpr);
	ts_statIcons->SetSubRect(shardIconSpr, 2);
	ts_statIcons->SetSpriteTexture(logIconSpr);
	ts_statIcons->SetSubRect(logIconSpr, 3);

	shardText.setFont(mm->arial);
	shardText.setCharacterSize(40);
	shardText.setFillColor(Color::White);

	logText.setFont(mm->arial);
	logText.setCharacterSize(40);
	logText.setFillColor(Color::White);

	levelNameText.setFont(mm->arial);
	levelNameText.setCharacterSize(60);
	levelNameText.setFillColor(Color::White);

	Vector2f levelStatsTopLeft = Vector2f(960, 750) + Vector2f(-181, 152); //these seemingly random numbers just center it and put it at the correct height
	Vector2f logDiff(180, 0);

	shardIconSpr.setPosition(levelStatsTopLeft);
	shardText.setPosition(shardIconSpr.getPosition() + Vector2f(96 + 10, 20));

	logIconSpr.setPosition(levelStatsTopLeft + logDiff);
	logText.setPosition(logIconSpr.getPosition() + Vector2f(96 + 10, 20));

	ts_enterFX->SetSpriteTexture(enterFXSpr);

	kinSpr.setTexture(*ts_kinBoost->texture);
	kinSpr.setTextureRect(ts_kinBoost->GetSubRect(71));
	kinSpr.setOrigin(kinSpr.getLocalBounds().width / 2, kinSpr.getLocalBounds().height / 2);
	kinSpr.setPosition(Vector2f(960, 540));
	kinSpr.setScale(2, 2);
	
	kinAuraSpr.setTexture(*ts_kinAura->texture);
	kinAuraSpr.setTextureRect(ts_kinAura->GetSubRect(0));
	kinAuraSpr.setOrigin(kinAuraSpr.getLocalBounds().width / 2, kinAuraSpr.getLocalBounds().height / 2);
	kinAuraSpr.setPosition(kinSpr.getPosition());
	kinAuraSpr.setScale(kinSpr.getScale());

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

	kinLoopLength = 22;//39;
	kinLoopTileStart = 79;

	kinEndTileStart = 101;
	kinEndLength = 9;

	//load this by streaming it or in another thread while in the boost screen. do not load until running
	//the gamesession
	//and 
	//ts_swipe[0] = mainMenu->tilesetManager.GetTileset("Menu/KinBoost/kinswipe_960x540.png", 960, 540);
	//swipeSpr.setTexture(*ts_swipe[0]->texture);
	//swipeSpr.setTextureRect(ts_swipe[0]->GetSubRect(0));
	//swipeSpr.setScale(2, 2);
	//swipeSpr.setPosition(0, 0);

	bgSpr.setTexture(*ts_bg->texture);
	bgShapeSpr.setTexture(*ts_bgShape->texture);

	for (int i = 0; i < 4; ++i)
	{
		starSpr[i].setTexture(*ts_stars[i]->texture);
	}
	
	for (int i = 0; i < 2; ++i)
	{
		lightSpr[i].setTexture(*ts_light[i]->texture);
	}

	for (int i = 0; i < 4; ++i)
	{
		if (!scrollShaderStars[i].loadFromFile("Resources/Shader/horizslider.frag", sf::Shader::Fragment))
		{
			assert(0);
		}
		scrollShaderStars[i].setUniform("u_texture", sf::Shader::CurrentTexture);
	}
	
	for (int i = 0; i < 2; ++i)
	{
		if (!scrollShaderLight[i].loadFromFile("Resources/Shader/horizslider.frag", sf::Shader::Fragment))
		{
			assert(0);
		}
		scrollShaderLight[i].setUniform("u_texture", sf::Shader::CurrentTexture);
	}
}

void KinBoostScreen::End()
{
	state = FINISHBOOST;
	stateFrame = 0;

	auto *mainMenu = MainMenu::GetInstance();
	//mainMenu->fader->CrossFade(30, 0, 30, Color::Black, true);
	mainMenu->fader->Fade(false, 30, Color::Black, false, EffectLayer::IN_FRONT_OF_UI);// true);
	mainMenu->musicPlayer->FadeOutCurrentMusic(30);
	//frame = 0;
}

void KinBoostScreen::Reset()
{
	frame = 0;
	stateFrame = 0;
	state = STARTING;
	ended = false;

	auto *mainMenu = MainMenu::GetInstance();

	skinShader.SetSkin(mainMenu->adventureManager->currSaveFile->defaultSkinIndex);

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

void KinBoostScreen::SetLevel(Level *lev)
{
	MainMenu *mm = MainMenu::GetInstance();
	level = lev;
	levelNameText.setString(mm->adventureManager->adventureFile.GetMap(level->index).name);

	auto lb = levelNameText.getLocalBounds();

	levelNameText.setOrigin(lb.left + lb.width / 2, lb.height / 2);
	levelNameText.setPosition(960, 800);

	int totalShards = 0;
	int numShardsCaptured = 0;
	int totalLogs = 0;
	int numLogsCaptured = 0;

	
	AdventureMapHeaderInfo &amhi =
		mm->adventureManager->adventureFile.GetMapHeaderInfo(level->index);
	totalShards = amhi.shardInfoVec.size();

	SaveFile *saveFile = mm->adventureManager->currSaveFile;
	for (int j = 0; j < totalShards; ++j)
	{
		if (saveFile->IsShardCaptured(amhi.shardInfoVec[j].GetTrueIndex()))
		{
			++numShardsCaptured;
		}
	}

	totalLogs = amhi.logInfoVec.size();
	for (int j = 0; j < totalLogs; ++j)
	{
		if (saveFile->HasLog(amhi.logInfoVec[j].GetTrueIndex()))
		{
			++numLogsCaptured;
		}
	}

	stringstream ss;

	ss.str("");
	ss.clear();

	ss << numShardsCaptured << "/" << totalShards;

	shardText.setString(ss.str());

	ss.str("");
	ss.clear();

	ss << numLogsCaptured << "/" << totalLogs;

	logText.setString(ss.str());
}

void KinBoostScreen::DrawLateKin(sf::RenderTarget *target)
{
	if (state != ENDING)
	{
		if (showAura)
		{
			target->draw(kinAuraSpr, &skinShader.pShader);
		}
		target->draw(kinSpr, &skinShader.pShader);
	}

	if (state == ENDING)
	{
		target->draw(enterFXSpr );
	}
}

void KinBoostScreen::Update()
{
	auto *mainMenu = MainMenu::GetInstance();

	showAura = true;
	switch (state)
	{
	case STARTING:
		state = BOOSTING;
		//frame = 0;
		stateFrame = 0;
		mainMenu->fader->Fade(true, 120, Color::Black, true, EffectLayer::IN_FRONT_OF_UI);
		break;
	case BOOSTING:
		break;
	case ENDING:
		//if( mainMenu->swiper->IsPostWipe())
		break;
	}


	if( state != STARTING )
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

		float bgFade = 360;
		int bgFadeI = bgFade;
		if (frame <= bgFadeI)
		{
			bgSpr.setColor(Color(255, 255, 255, (frame / bgFade) * 255.f));
		}

		int kFrame = (frame % (kinLoopLength * 2));
		int kActual = kFrame / 2 + kinLoopTileStart;

		if (state == BOOSTING || state == FINISHBOOST)
		{
			kinSpr.setTextureRect(ts_kinBoost->GetSubRect(kActual));

			kinAuraSpr.setTextureRect(ts_kinAura->GetSubRect(kActual - 55));

			
			
		}

		

		if (state == FINISHBOOST && kFrame == kinLoopLength * 2 - 1 && mainMenu->fader->IsFullyFadedOut() )
		{
			state = ENDING;
			stateFrame = 0;
			
		}
		
		if (state == ENDING)
		{
			kFrame = stateFrame / 2 + kinEndTileStart;
			kinSpr.setTextureRect(ts_kinBoost->GetSubRect(kFrame));
			//cout << "kframe: " << kFrame << endl;
			//if (kFrame - 55 > 60)
			//	showAura = false;

			kinAuraSpr.setTextureRect(ts_kinAura->GetSubRect(kFrame - 55));

			if (stateFrame / 2 < 12)
			{
				ts_enterFX->SetSubRect(enterFXSpr, stateFrame / 2 + 7);
				enterFXSpr.setPosition(kinSpr.getPosition());
				enterFXSpr.setScale(2, 2);
				enterFXSpr.setOrigin(enterFXSpr.getLocalBounds().width / 2,
					enterFXSpr.getLocalBounds().height / 2);
			}

			//cout << "blah: " << kFrame - 55 << endl;

			if (stateFrame == kinEndLength * 2-1)
			{
				ended = true;
				--stateFrame;
				//return;
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

	++stateFrame;
	++frame;

	
}

bool KinBoostScreen::IsEnded()
{
	return ended;
}

bool KinBoostScreen::IsBoosting()
{
	return state == BOOSTING;
}

void KinBoostScreen::Draw(RenderTarget *target)
{
	switch (state)
	{
	case STARTING:
	{
		//target->draw(swipeSpr);
		break;
	}
	case ENDING:
	case FINISHBOOST:
	case BOOSTING:
	{
		target->draw(bgSpr);

		if (frame < 120)
		{
			//target->draw(kinSpr);
			//return;
		}
		//target->draw(bgShapeSpr);
		for (int i = 3; i >= 0; --i)
		{
			target->draw(starSpr[i], &scrollShaderStars[i]);
		}

		for (int i = 1; i >= 0; --i)
		{
			target->draw(lightSpr[i], &scrollShaderLight[i]);
		}

		target->draw(shardIconSpr);
		target->draw(shardText);
		target->draw(logIconSpr);
		target->draw(logText);

		target->draw(levelNameText);

		//target->draw(kinSpr);
		break;
	}
	}
	
}