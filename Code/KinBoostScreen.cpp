#include "KinBoostScreen.h"
#include "MainMenu.h"
#include "Fader.h"

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
	ts_kinAura = mainMenu->tilesetManager.GetTileset("Kin/exitaura_256x256.png", 256, 256);

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
	


	numCoverTiles = 2;

	kinLoopLength = 22;//39;
	kinLoopTileStart = 79;

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
	state = ENDING;
	frame = 0;
	
}

void KinBoostScreen::Reset()
{
	frame = 0;
	state = STARTING;
	ended = false;
}

void KinBoostScreen::DrawLateKin(sf::RenderTarget *target)
{
	target->draw(kinAuraSpr);
	target->draw(kinSpr);
}

void KinBoostScreen::Update()
{
	switch (state)
	{
	case STARTING:
		state = BOOSTING;
		frame = 0;
		break;
	case BOOSTING:
		break;
	case ENDING:
		//if( mainMenu->swiper->IsPostWipe())
		break;
	}


	switch (state)
	{
	case STARTING:
	{

		break;
	}
	case ENDING:
	case BOOSTING:
	{
		int scrollFramesBack = 40;
		int scrollFramesFront = 30;
		float fBack = frame % scrollFramesBack;
		float fFront = frame % scrollFramesFront;

		int mult = frame / scrollFramesBack;
		int multFront = frame / scrollFramesFront;

		float facBack = fBack / (scrollFramesBack);
		float facFront = fFront / (scrollFramesFront);

		for (int i = 0; i < 4; ++i)
		{
			scrollShaderStars[i].setUniform("quant", facFront);//0.f);
		}

		scrollShaderLight[0].setUniform("quant", facFront);
		scrollShaderLight[1].setUniform("quant", facBack);

		float bgFade = 360;
		int bgFadeI = bgFade;
		if (frame <= bgFadeI)
		{
			bgSpr.setColor(Color(255, 255, 255, (frame / bgFade) * 255.f));
		}

		int kFrame = (frame % (kinLoopLength * 2));

		int kActual = kFrame / 2 + kinLoopTileStart;
		kinSpr.setTextureRect(ts_kinBoost->GetSubRect(kActual));

		kinAuraSpr.setTextureRect( ts_kinAura->GetSubRect( kActual - 55 ) );

		//cout << "ff: " << kActual << endl;

		if (state == ENDING && kFrame == kinLoopLength * 2 - 1 )
		{
			ended = true;
		}

		break;
	}
	}

	

	

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
	case BOOSTING:
	{
		target->draw(bgSpr);

		if (frame < 120)
		{
			//target->draw(kinSpr);
			//return;
		}
		target->draw(bgShapeSpr);
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