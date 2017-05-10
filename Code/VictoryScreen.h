#ifndef __VICTORY_SCREEN_H__
#define __VICTORY_SCREEN_H__

#include "SFML/Graphics.hpp"
#include "Input.h"

struct UIWindow;
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
		STATE_INITIAL_WAIT, //TODO
		STATE_SHOW_FACE,
		STATE_WAIT,
		STATE_WAIT_EXPANDED,
		STATE_CLOSED
	};

	State currState;

	enum QuadIndex
	{
		BOT_QUAD_INDEX = 0,
		STRETCH_QUAD_INDEX = 1,
		TOP_QUAD_INDEX = 2
	};

	PlayerInfoBar( GameSession *owner, int width,
		int playerIndex );
	void Update( bool pressedA );
	int frame;
	State state;
	void Draw( sf::RenderTarget *target );
	void SetHeight( int height );
	
	GameSession *owner;
	
	int framesBeforeShowFace;
	int framesExpandingShowFace;
	int framesExpandingFull;
	int framesToClose;

	int heightWait;
	int heightShowFace;
	int heightFull;

	UIWindow *uiWindow;
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