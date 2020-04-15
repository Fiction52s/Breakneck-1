#include "TitleScreen.h"
#include "Tileset.h"
#include "MainMenu.h"
#include "Background.h"
#include "MusicPlayer.h"
#include "MusicSelector.h"
#include "Fader.h"
#include "VisualEffects.h"

using namespace std;
using namespace sf;

TitleScreen::TitleScreen(MainMenu *p_mainMenu)
	:mainMenu( p_mainMenu )
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

	ts_kin = p_mainMenu->tilesetManager.GetTileset("Title/kintitle_128x256.png", 128, 256);
	kinSpr.setTexture(*ts_kin->texture);
	kinSpr.setTextureRect(ts_kin->GetSubRect(0));
	kinSpr.setPosition(913, 710);

	ts_kinHandEnergy = p_mainMenu->tilesetManager.GetTileset("Title/kintitle_energy_128x256.png", 128, 256);
	kinHandEnergySpr.setTexture(*ts_kinHandEnergy->texture);
	kinHandEnergySpr.setTextureRect(ts_kinHandEnergy->GetSubRect(0));
	kinHandEnergySpr.setPosition(913, 710);

	scrollingBackgrounds.push_back(
		new ScrollingBackground(
			p_mainMenu->tilesetManager.GetTileset("Title/titlecloud_1_1920x1080.png", 1920, 1080), 0, 1, 5 * 5));
	scrollingBackgrounds.push_back(
		new ScrollingBackground(
			p_mainMenu->tilesetManager.GetTileset("Title/titlecloud_2_1920x1080.png", 1920, 1080), 0, 1, 10 * 5));

	frame = 0;

	
	titleMusic = mainMenu->musicManager->songMap["w0_1_Breakneck_Title"];
	titleMusic->Load();

}

TitleScreen::~TitleScreen()
{
	delete background;

	for (auto it = scrollingBackgrounds.begin(); it != scrollingBackgrounds.end(); ++it)
	{
		delete (*it);
	}
}

void TitleScreen::Reset()
{
	background->Reset();
	frame = 0;
}

void TitleScreen::Update()
{
	if (frame == 60)
	{
		//mainMenu->fader->CrossFade(60, 60, Color::Red);
		mainMenu->musicPlayer->PlayMusic(titleMusic);
	}

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

	int kinMult = 5;
	int hairAnimFrames = 12;
	int waitFrames = 120;
	int totalFrames = hairAnimFrames * 6 * kinMult + waitFrames * kinMult;
	int kinFrame = (frame % totalFrames) / kinMult;
	if (kinFrame < hairAnimFrames * 6)
	{
		int wind = 30;
		if (kinFrame <= hairAnimFrames * 2)
		{
			wind = ((float)kinFrame / ( hairAnimFrames*2)) * wind;
		}
		else if (kinFrame < hairAnimFrames * 4)
		{
			
		}
		else
		{
			wind = wind - ((float)(kinFrame % ( hairAnimFrames*2))/ (hairAnimFrames*2)) * wind;
		}
		//cout << "wind: " << wind << endl;
		auto it = scrollingBackgrounds.begin();
		(*it)->scrollSpeedX = 5 * 5 + wind;
		++it;
		(*it)->scrollSpeedX = 10 * 5 + wind / 2;
		int realKinFrame = kinFrame % hairAnimFrames;
		kinSpr.setTextureRect(ts_kin->GetSubRect(realKinFrame));
	}
	else 
	{
		auto it = scrollingBackgrounds.begin();
		(*it)->scrollSpeedX = 5 * 5;
		++it;
		(*it)->scrollSpeedX = 10 * 5;
		kinSpr.setTextureRect(ts_kin->GetSubRect(0));
	}
	
	
	
	kinHandEnergySpr.setTextureRect(ts_kinHandEnergy->GetSubRect(kinFrame % 12));

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

	target->draw(kinHandEnergySpr);
	target->draw(kinSpr);

	mainMenu->DrawMenuOptionText(target);

	/*for (int i = 0; i < 4; ++i)
	{
		target->draw(lightSpr[i]);
	}*/

	target->draw(breakneckTitleSprite);
	target->draw(emergenceTitleSprite);

	
	
}