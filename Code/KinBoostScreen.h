#ifndef __KINBOOSTSCREEN_H__
#define __KINBOOSTSCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"

struct MainMenu;
struct KinBoostScreen
{
	enum State
	{
		STARTING,
		BOOSTING,
		ENDING
	};

	bool ended;
	bool IsEnded();
	State state;
	KinBoostScreen( MainMenu *mm);
	void Draw(sf::RenderTarget *target);
	void Reset();
	bool IsBoosting();
	sf::Sprite bgSpr;
	sf::Sprite bgShapeSpr;
	sf::Sprite starSpr[4];
	sf::Sprite lightSpr[2];
	MainMenu *mainMenu;
	Tileset *ts_stars[4];
	Tileset *ts_light[2];
	Tileset *ts_bg;
	Tileset *ts_bgShape;
	Tileset *ts_kinBoost;
	sf::Sprite kinSpr;
	int kinLoopLength;
	int kinLoopTileStart;
	int frame;
	void Update();
	void End();
	void DrawLateKin(sf::RenderTarget *target);
	sf::Shader scrollShaderStars[4];
	sf::Shader scrollShaderLight[2];



	const static int NUM_SWIPE_SPRITES = 1;
	int numCoverTiles;
	Tileset *ts_swipe[NUM_SWIPE_SPRITES];

	sf::Sprite swipeSpr;

	std::string levName;
};

#endif