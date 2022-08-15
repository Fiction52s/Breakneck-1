#ifndef __VICTORY_SCREEN_H__
#define __VICTORY_SCREEN_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "Movement.h"

struct UIWindow;
struct MatchResultsScreen
{
	MatchResultsScreen() { Reset(); }
	~MatchResultsScreen() {}
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

struct VictoryScreen2PlayerVS : MatchResultsScreen
{
	VictoryScreen2PlayerVS( GameSession *p_game );
	void Draw( sf::RenderTarget *target );
	void Update();
	void ResetSprites();
	void UpdateSprites();
	void Reset();
	GameSession *game;

	PlayerInfoBar *player1Bar;
	PlayerInfoBar *player2Bar;
	
};

struct Tileset;
struct ResultsScreen : MatchResultsScreen
{
	enum State
	{
		FADEIN,
		SLIDEIN,
		WAIT,
		SLIDEOUT,
		FADEOUT,
		DONE,
	};

	State state;
	ResultsScreen(GameSession *owner);
	void Draw(sf::RenderTarget *target);
	void Update();
	void ResetSprites();
	void UpdateSprites();
	void Reset();
	bool IsDone();
	GameSession *owner;
	void SetupColumns();

	Tileset *ts_column[4];
	int maxPlacing;

	sf::Sprite columnSprites[4];

	bool columnReady[4];

	Tileset * GetTeamTileset(int teamIndex, bool win);
	Tileset * GetSoloTilset(int soloIndex, bool win);

	void SetTile(int boxIndex, int tile);
	void SetBoxPos(int boxIndex, float yHeight);

	CubicBezier slideInBez[4];
	int slideInStartFrame[4];
	int slideInFrames[4];

	CubicBezier slideOutBez;
	int slideOutFrames;
};

#endif