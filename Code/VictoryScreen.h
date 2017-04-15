#ifndef __VICTORY_SCREEN_H__
#define __VICTORY_SCREEN_H__

#include "SFML/Graphics.hpp"

struct VictoryScreen
{
	virtual void Draw( sf::RenderTarget *target ) = 0;
	virtual void Update() = 0;
	virtual void ResetSprites() = 0;
	virtual void UpdateSprites() = 0;
	virtual void Reset();
	int frame;
};

struct Tileset;
struct GameSession;

struct PlayerInfoBar
{
	enum State
	{
		STATE_SHOW_FACE,
		STATE_WAIT,
		STATE_EXPANDING,
		STATE_WAIT_EXPANDED
	};

	State currState;

	enum QuadIndex
	{
		BOT_QUAD_INDEX = 0,
		STRETCH_QUAD_INDEX = 1,
		TOP_QUAD_INDEX = 2
	};

	PlayerInfoBar( GameSession *owner );
	void AssignTiles();
	void SetBottomLeftPos( sf::Vector2f &pos );
	const sf::Vector2f &GetBottomLeftPos();
	void Draw( sf::RenderTarget *target );

	Tileset *ts_bar;
	sf::Vertex infoBarVA[4*3];
	GameSession *owner;
	int botQuadHeight;
	int topQuadHeight;
	int totalWidth;
	int currMiddleHeight;
};

struct VictoryScreen2PlayerVS : VictoryScreen
{
	VictoryScreen2PlayerVS( GameSession *owner );
	void Draw( sf::RenderTarget *target );
	void Update();
	void ResetSprites();
	void UpdateSprites();
	void Reset();
	GameSession *owner;

	PlayerInfoBar *player1Bar;
	PlayerInfoBar *player2Bar;
	
};

#endif