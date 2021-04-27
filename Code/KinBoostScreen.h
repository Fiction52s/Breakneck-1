#ifndef __KINBOOSTSCREEN_H__
#define __KINBOOSTSCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "Actor.h"

struct MainMenu;
struct Level;
struct KinBoostScreen
{
	enum State
	{
		STARTING,
		BOOSTING,
		FINISHBOOST,
		ENDING
	};

	bool ended;
	bool IsEnded();
	int stateFrame;
	State state;
	KinBoostScreen( MainMenu *mm);
	void Draw(sf::RenderTarget *target);
	void Reset();
	bool IsBoosting();
	sf::Sprite bgSpr;
	sf::Sprite bgShapeSpr;
	sf::Sprite starSpr[4];
	sf::Sprite lightSpr[2];
	sf::Sprite kinAuraSpr;
	bool showAura;
	MainMenu *mainMenu;
	Tileset *ts_stars[4];
	Tileset *ts_light[2];
	Tileset *ts_bg;
	Tileset *ts_bgShape;
	Tileset *ts_kinBoost;
	Tileset *ts_kinAura;
	Tileset *ts_enterFX;
	sf::Sprite enterFXSpr;
	sf::Sprite kinSpr;
	int kinLoopLength;
	int kinLoopTileStart;
	int kinEndTileStart;
	int kinEndLength;
	int frame;
	void Update();
	void End();
	void DrawLateKin(sf::RenderTarget *target);
	sf::Shader scrollShaderStars[4];
	sf::Shader scrollShaderLight[2];
	float starSpeed[4];
	float lightSpeed[4];
	float starFac[4];
	float lightFac[2];
	float starMax[4];
	float lightMax[2];
	float starAccel[4];
	float lightAccel[2];



	const static int NUM_SWIPE_SPRITES = 1;
	int numCoverTiles;
	Tileset *ts_swipe[NUM_SWIPE_SPRITES];

	sf::Sprite swipeSpr;

	sf::Image skinPaletteImage;
	sf::Shader pShader;
	sf::Glsl::Vec4 paletteArray[Actor::NUM_PALETTE_COLORS];

	//std::string levName;
	Level *level;
};

#endif