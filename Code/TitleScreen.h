#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"

struct MainMenu;
struct Background;
struct ScrollingBackground;
struct MusicInfo;

struct TitleScreen : TilesetManager
{
	TitleScreen(MainMenu *p_mainMenu);
	~TitleScreen();
	void Draw(sf::RenderTarget *target);
	void Reset();
	void Update();
	Tileset *ts_breakneckTitle;
	Tileset *ts_emergenceTitle;
	Tileset *ts_titleBG;
	int frame;
	sf::Sprite titleBGSprite;
	sf::Sprite breakneckTitleSprite;
	sf::Sprite emergenceTitleSprite;
	std::list<ScrollingBackground*> scrollingBackgrounds;
	Tileset *ts_energy;
	sf::Sprite energySpr;
	sf::Sprite kinSpr;
	Tileset *ts_kin;
	sf::Sprite kinHandEnergySpr;
	Tileset *ts_kinHandEnergy;
	MainMenu *mainMenu;
	Background *background;

	MusicInfo *titleMusic;
	
};

#endif