#include "TitleScreen.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "Background.h"
#include "Parallax.h"

using namespace std;
using namespace sf;

TitleScreen::TitleScreen(MainMenu *p_mainMenu)
{
	ts_breakneckTitle = p_mainMenu->tilesetManager.GetTileset("Title/breakneck_1034x835.png", 1034, 835);
	//ts_backgroundTitle = tilesetManager.GetTileset( "Title/title_bg_1920x1080.png", 1920, 1080 );
	ts_titleBG = p_mainMenu->tilesetManager.GetTileset("Title/title_base_1920x1080.png", 1920, 1080);
	ts_emergenceTitle = p_mainMenu->tilesetManager.GetTileset("Title/emergence_337x51.png", 337, 51);

	titleBGSprite.setTexture(*ts_titleBG->texture);
	breakneckTitleSprite.setTexture(*ts_breakneckTitle->texture);
	emergenceTitleSprite.setTexture(*ts_emergenceTitle->texture);

	breakneckTitleSprite.setPosition(444, 0 + 20);
	emergenceTitleSprite.setPosition(794, 233 + 20);

	background = new Background(p_mainMenu);


	ts_light[0] = p_mainMenu->tilesetManager.GetTileset("Title/titlelight_1_1920x1080.png", 1920, 1080);
	ts_light[1] = p_mainMenu->tilesetManager.GetTileset("Title/titlelight_2_1920x1080.png", 1920, 1080);
	ts_light[2] = p_mainMenu->tilesetManager.GetTileset("Title/titlelight_3_1920x1080.png", 1920, 1080);
	ts_light[3] = p_mainMenu->tilesetManager.GetTileset("Title/titlelight_4_1920x1080.png", 1920, 1080);

	for (int i = 0; i < 4; ++i)
	{
		lightSpr[i].setTexture(*ts_light[i]->texture);
	}

	ts_energy = p_mainMenu->tilesetManager.GetTileset("Title/energy_1920x1080.png", 1920, 1080);
	energySpr.setTexture(*ts_energy->texture);

	energySpr.setColor(Color::Blue);


	scrollingBackgrounds.push_back(
		new ScrollingBackground(
			p_mainMenu->tilesetManager.GetTileset("Title/titlecloud_1_1920x1080.png", 1920, 1080), 0, 1, -5 * 5));
	scrollingBackgrounds.push_back(
		new ScrollingBackground(
			p_mainMenu->tilesetManager.GetTileset("Title/titlecloud_2_1920x1080.png", 1920, 1080), 0, 1, -10 * 5));

	frame = 0;
}

void TitleScreen::Reset()
{
	background->Reset();
	frame = 0;
}

void TitleScreen::Update()
{
	background->Update();

	for (int i = 0; i < 4; ++i)
	{
		Color c = background->GetShapeColor();
		c.r = 255 - c.r;
		c.g = 255 - c.g;
		c.b = 255 - c.b;

		lightSpr[i].setColor(c);//
	}

	for ( auto it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Update(Vector2f( 960, 540 ));
	}

	Color c = energySpr.getColor();
	int energyBreathe = 360;
	int ff = frame % (energyBreathe);
	if (ff < energyBreathe / 2)
	{
		float factor = (float)ff / (energyBreathe / 2);
		c.a = 150 * factor;
		//c.a = std::max(20.f, (float)c.a);
	}
	else
	{
		float factor = 1.f - (float)(ff - energyBreathe / 2) / (energyBreathe / 2);
		c.a = 150 * factor;
		//c.a = std::max(20.f, (float)c.a);
	}

	if (ff % (energyBreathe * 2) == 0)
	{
		c = Color(rand() % 255, rand() % 255, rand() % 255, c.a );
	}

	energySpr.setColor(c);

	frame++;
}

void TitleScreen::Draw(sf::RenderTarget *target)
{
	background->Draw(target);
	target->draw(titleBGSprite);

	target->draw(energySpr);

	for (list<ScrollingBackground*>::iterator it = scrollingBackgrounds.begin();
		it != scrollingBackgrounds.end(); ++it)
	{
		(*it)->Draw(target);
	}

	/*for (int i = 0; i < 4; ++i)
	{
		target->draw(lightSpr[i]);
	}*/

	target->draw(breakneckTitleSprite);
	target->draw(emergenceTitleSprite);
}