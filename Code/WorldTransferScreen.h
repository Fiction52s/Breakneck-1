#ifndef __WORLDTRANSFERSCREEN_H__
#define __WORLDTRANSFERSCREEN_H__

#include <SFML\Graphics.hpp>
#include "Tileset.h"
#include "Actor.h"
#include "PlayerSkinShader.h"

struct MainMenu;
struct Level;
struct WorldTransferScreen : TilesetManager
{
	enum State
	{
		STARTING,
		BOOSTING,
		FINISHBOOST,
		ENDING
	};

	sf::Text levelNameText;
	sf::Sprite shardIconSpr;
	sf::Text shardText;
	sf::Sprite logIconSpr;
	sf::Text logText;

	bool ended;
	sf::Text worldText;
	int currWorld;
	State state;
	sf::Sprite bgSpr;
	sf::Sprite bgShapeSpr;
	sf::Sprite starSpr[4];
	sf::Sprite lightSpr[2];
	sf::Sprite shipSpr;
	sf::Sprite planetSpr;
	Tileset *ts_stars[4];
	Tileset *ts_light[2];
	Tileset *ts_bg;
	Tileset *ts_planet;
	Tileset *ts_bgShape;
	Tileset *ts_enterFX;
	Tileset *ts_ship;
	sf::Vector2f shipStart;
	sf::Vector2f shipEnd;
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
	int shipExitLength;
	PlayerSkinShader skinShader;
	Level *level;
	
	WorldTransferScreen();
	void SetLevel(Level *lev);
	void Draw(sf::RenderTarget *target);
	void SetWorld(int wIndex);
	void Reset();
	bool IsEnded();
	bool IsBoosting();
	void Update();
	void End();
	
};

#endif