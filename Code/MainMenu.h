#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "SoundManager.h"
#include "Tileset.h"
#include "WorldMap.h"

struct SaveFile;

struct MainMenu
{
	enum MainMenuOptions
	{
		M_NEW_GAME,
		M_CONTINUE,
		M_CUSTOM_MAPS,
		M_LEVEL_EDITOR,
		M_OPTIONS,
		M_CREDITS,
		M_EXIT,
		M_Count
	};

	enum Mode
	{
		MAINMENU,
		WORLDMAP,
		SAVEMENU,
		TRANS_MAIN_TO_SAVE,
		TRANS_SAVE_TO_MAIN,
		TRANS_SAVE_TO_WORLDMAP
	};

	MainMenu();
	void GameEditLoop( const std::string &fileName );
	void GameEditLoop2( const std::string &filename);
	sf::RenderWindow *window;
	sf::View v;
	sf::View uiView;
	GameController controller;
	SoundManager soundManager;
	TilesetManager tilesetManager;

	static sf::RenderTexture *preScreenTexture;
	static sf::RenderTexture *postProcessTexture;
	static sf::RenderTexture *postProcessTexture1;
	static sf::RenderTexture *postProcessTexture2;
	static sf::RenderTexture *minimapTexture;
	static sf::RenderTexture *mapTexture;
	
	sf::Texture worldMapTex;
	sf::Sprite worldMapSpr;
	Tileset *ts_worldMap;
	Tileset *ts_saveMenuBG;
	Tileset *ts_saveMenuSelect;
	Tileset *ts_saveMenuKinFace;
	Tileset *ts_kinTitle[7];
	Tileset *ts_breakneckTitle;
	Tileset *ts_backgroundTitle;


	
	Mode menuMode;

	SaveFile *files[6];

	ControllerState currInput;
	ControllerState prevInput;

	sf::Sprite saveBG;
	sf::Sprite saveSelect;
	sf::Sprite saveKinFace;
	sf::Sprite backgroundTitleSprite;
	sf::Sprite breakneckTitleSprite;
	sf::Sprite kinTitleSprite;
	int saveKinFaceFrame;
	int saveKinFaceTurnLength;
	sf::Font arial;

	LevelSelector *levelSelector;
	WorldMap *worldMap;

	int selectedSaveIndex;

	int kinTitleSpriteFrame;
	int kinTotalFrames;
	int currentMenuSelect = 0;
};

#endif