#ifndef __VICTORY_SCREEN_H__
#define __VICTORY_SCREEN_H__

#include "SFML/Graphics.hpp"
#include "Input.h"

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
		STATE_INITIAL_WAIT, //STODO
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

	PlayerInfoBar( GameSession *owner );
	void AssignCorners();
	void AssignEdges();
	void Update( bool pressedA );
	int frame;
	State state;
	void SetBottomLeftPos( sf::Vector2i &pos );
	const sf::Vector2f &GetBottomLeftPos();
	void Draw( sf::RenderTarget *target );
	void SetHeight( int height );
	void SetCornerPos( sf::Vector2i &topLeft, int index ); 
	void SetWallPos( int x, int index );

	int minSize;
	Tileset *ts_bar;
	sf::Vertex windowVA[4 * 4];
	//sf::Vertex infoBarVA[4*3];

	sf::Vector2i windowSize;
	GameSession *owner;
	int botQuadHeight;
	int topQuadHeight;
	int totalWidth;
	int currMiddleHeight;
	int origMiddleHeight;

	int framesBeforeShowFace;
	int framesExpandingShowFace;
	int framesExpandingFull;
	int framesToClose;

	int heightWait;
	int heightShowFace;
	int heightFull;

	sf::Vector2i bottomLeft;
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