#include "KinBoostScreen.h"
#include "MainMenu.h"

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

void KinBoostScreen::Reset()
{
	frame = 0;
}

void KinBoostScreen::Update()
{
	int scrollFramesBack = 20;
	int scrollFramesFront = 30;
	float fBack = frame % scrollFramesBack;
	float fFront = frame % scrollFramesFront;

	int mult = frame / scrollFramesBack;
	int multFront = frame / scrollFramesFront;

	float facBack = fBack / (scrollFramesBack);
	float facFront = fFront / (scrollFramesFront);

	for (int i = 0; i < 4; ++i)
	{
		scrollShaderStars[i].setUniform("quant", 0.f);
	}

	scrollShaderLight[0].setUniform("quant", facFront);
	scrollShaderLight[1].setUniform("quant", facBack);

	++frame;
	for (int i = 0; i < 2; ++i)
	{
		scrollShaderLight[i].setUniform("quant", -facBack);
	}

	
}

void KinBoostScreen::Draw(RenderTarget *target)
{
	target->draw(bgSpr);
	target->draw(bgShapeSpr);
	for (int i = 3; i >= 0; --i)
	{
		target->draw(starSpr[i], &scrollShaderStars[i]);
	}

	for (int i = 1; i >= 0; --i)
	{
		target->draw(lightSpr[i], &scrollShaderLight[i]);
	}
}