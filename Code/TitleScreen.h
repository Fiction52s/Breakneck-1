#ifndef __TITLESCREEN_H__
#define __TITLESCREEN_H__

#include <SFML/Graphics.hpp>
#include <list>
#include "Tileset.h"
#include "GUI.h"
#include <string>

struct MainMenu;
struct Background;
struct ScrollingBackground;
struct MusicInfo;

struct TitleScreen : TilesetManager, GUIHandler
{
	Panel *panel;
	Button *feedbackButton;
	std::string feedbackURL;

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

	TitleScreen(MainMenu *p_mainMenu);
	~TitleScreen();
	void ButtonCallback(Button *b, const std::string & e);
	void Draw(sf::RenderTarget *target);
	void Reset();
	void Update();
};

#endif