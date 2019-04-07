#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include <SFML/Graphics.hpp>
#include <list>

struct MainMenu;
struct Tileset;
struct Background;
struct ScrollingBackground;
struct MusicInfo;

struct TitleScreen
{
	TitleScreen(MainMenu *p_mainMenu);
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
	Tileset *ts_light[4];
	sf::Sprite lightSpr[4];
	sf::Sprite kinSpr;
	Tileset *ts_kin;
	sf::Sprite kinHandEnergySpr;
	Tileset *ts_kinHandEnergy;
	MainMenu *mainMenu;
	Background *background;

	MusicInfo *titleMusic;
	
};

#endif