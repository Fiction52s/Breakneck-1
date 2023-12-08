#ifndef __KINBOOSTSCREEN_H__
#define __KINBOOSTSCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "Actor.h"
#include "PlayerSkinShader.h"

struct MainMenu;
struct Level;
struct KinBoostScreen : TilesetManager
{
	enum State
	{
		STARTING,
		BOOSTING,
		FINISHBOOST,
		ENDING
	};

	bool ended;
	sf::Text levelNameText;
	sf::Sprite shardIconSpr;
	sf::Text shardText;
	sf::Sprite logIconSpr;
	sf::Text logText;
	int stateFrame;
	State state;
	sf::Sprite bgSpr;
	sf::Sprite bgShapeSpr;
	sf::Sprite starSpr[4];
	sf::Sprite lightSpr[2];
	sf::Sprite kinAuraSpr;
	bool showAura;
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
	Tileset *ts_statIcons;

	PlayerSkinShader skinShader;
	Level *level;
	
	KinBoostScreen();
	void SetLevel(Level *lev);
	bool IsEnded();
	void Draw(sf::RenderTarget *target);
	void Reset();
	bool IsBoosting();
	void Update();
	void End();
	void DrawLateKin(sf::RenderTarget *target);
	
};

#endif