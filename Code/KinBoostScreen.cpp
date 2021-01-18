#include "KinBoostScreen.h"
#include "MainMenu.h"
#include "Fader.h"
#include "VectorMath.h" 

using namespace std;
using namespace sf;

KinBoostScreen::KinBoostScreen( MainMenu *mm )
	:mainMenu( mm )
{
	ts_bg = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_BG1.png", 1920, 1080);
	ts_bgShape = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_BG1_shape.png", 1920, 1080);

	ts_light[0] = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_light_01a.png", 1920, 1080);
	ts_light[1] = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_light_01b.png", 1920, 1080);

	ts_stars[0] = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_stars_01a.png", 1920, 1080);
	ts_stars[1] = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_stars_01b.png", 1920, 1080);
	ts_stars[2] = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_stars_01c.png", 1920, 1080);
	ts_stars[3] = mainMenu->tilesetManager.GetTileset("KinBoost/kinboost_stars_01d.png", 1920, 1080);

	ts_kinBoost = mainMenu->tilesetManager.GetTileset("Kin/exit_96x128.png", 96, 128);
	ts_kinAura = mainMenu->tilesetManager.GetTileset("Kin/FX/exitaura_256x256.png", 256, 256);

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

	numCoverTiles = 2;

	kinLoopLength = 22;//39;
	kinLoopTileStart = 79;

	kinEndTileStart = 101;
	kinEndLength = 9;

	//load this by streaming it or in another thread while in the boost screen. do not load until running
	//the gamesession
	//and 
	//ts_swipe[0] = mainMenu->tilesetManager.GetTileset("KinBoost/kinswipe_960x540.png", 960, 540);
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
	//mainMenu->fader->CrossFade(30, 0, 30, Color::Black, true);
	mainMenu->fader->Fade(false, 30, Color::Black, true);
	//frame = 0;
}

void KinBoostScreen::Reset()
{
	frame = 0;
	stateFrame = 0;
	state = STARTING;
	ended = false;

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

void KinBoostScreen::DrawLateKin(sf::RenderTarget *target)
{
	if (showAura)
	{
		target->draw(kinAuraSpr);
	}
	target->draw(kinSpr);
}

void KinBoostScreen::Update()
{
	showAura = true;
	switch (state)
	{
	case STARTING:
		state = BOOSTING;
		//frame = 0;
		stateFrame = 0;
		mainMenu->fader->Fade(true, 30, Color::Black, true);
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
		else if (state == ENDING)
		{
			kFrame = stateFrame / 2 + kinEndTileStart;
			kinSpr.setTextureRect(ts_kinBoost->GetSubRect(kFrame));
			//cout << "kframe: " << kFrame << endl;
			//if (kFrame - 55 > 60)
			//	showAura = false;

			kinAuraSpr.setTextureRect(ts_kinAura->GetSubRect(kFrame - 55));

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

		//target->draw(kinSpr);
		break;
	}
	}
	
}