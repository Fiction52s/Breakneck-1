#ifndef __VICTORY_SCREEN_H__
#define __VICTORY_SCREEN_H__

#include "SFML/Graphics.hpp"
#include "Input.h"
#include "Movement.h"
#include "MatchStats.h"
#include "Tileset.h"



struct UIWindow;
struct MatchResultsScreen : TilesetManager
{
	MatchResultsScreen(MatchStats *mStats);
	virtual ~MatchResultsScreen();
	virtual void Draw( sf::RenderTarget *target ) = 0;
	virtual bool Update() = 0; //return false when done
	virtual void ResetSprites() = 0;
	virtual void UpdateSprites() = 0;
	virtual void Reset();
	int frame;
	MatchStats *matchStats;
};

struct Tileset;
struct GameSession;


struct PlayerInfoBar
{
	enum Action
	{
		A_IDLE,
		A_RISE,
		A_WAIT,
		A_DONE,
		A_Count
	};

	MatchResultsScreen *resultsScreen;
	Action action;
	int frame;
	sf::Vertex quad[4];
	int actionLength[A_Count];

	sf::Text nameText;

	int waitHeight;
	int startHeight;
	int width;
	int currHeight;
	int pIndex;

	PlayerInfoBar(MatchResultsScreen *mrs, int playerIndex );
	void Update( bool pressedA );
	void SetHeight( int height );
	void Draw(sf::RenderTarget *target);
};

struct VictoryScreen2PlayerVS : MatchResultsScreen
{
	VictoryScreen2PlayerVS(MatchStats *mStats);
	~VictoryScreen2PlayerVS();
	void Draw( sf::RenderTarget *target );
	bool Update();
	void ResetSprites();
	void UpdateSprites();
	void Reset();
	GameSession *game;

	PlayerInfoBar *player1Bar;
	PlayerInfoBar *player2Bar;
	
};

struct Tileset;
//struct ResultsScreen : MatchResultsScreen
//{
//	enum State
//	{
//		FADEIN,
//		SLIDEIN,
//		WAIT,
//		SLIDEOUT,
//		FADEOUT,
//		DONE,
//	};
//
//	State state;
//	ResultsScreen(GameSession *owner);
//	void Draw(sf::RenderTarget *target);
//	bool Update();
//	void ResetSprites();
//	void UpdateSprites();
//	void Reset();
//	bool IsDone();
//	GameSession *owner;
//	void SetupColumns();
//
//	Tileset *ts_column[4];
//	int maxPlacing;
//
//	sf::Sprite columnSprites[4];
//
//	bool columnReady[4];
//
//	Tileset * GetTeamTileset(int teamIndex, bool win);
//	Tileset * GetSoloTilset(int soloIndex, bool win);
//
//	void SetTile(int boxIndex, int tile);
//	void SetBoxPos(int boxIndex, float yHeight);
//
//	CubicBezier slideInBez[4];
//	int slideInStartFrame[4];
//	int slideInFrames[4];
//
//	CubicBezier slideOutBez;
//	int slideOutFrames;
//};

#endif