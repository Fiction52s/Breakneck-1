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
	int action;
	int frame;
	MatchStats *matchStats;
};

struct Tileset;
struct GameSession;
struct PlayerSkinShader;

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

	sf::Sprite kinSprite;
	PlayerSkinShader *skinShader;
	

	sf::Text nameText;

	int waitHeight;
	int startHeight;
	int width;
	int currHeight;
	int pIndex;
	sf::Text infoText;

	sf::Sprite placingSpr;

	PlayerInfoBar(MatchResultsScreen *mrs, int playerIndex );
	~PlayerInfoBar();
	void Activate();
	void Update( bool pressedA );
	void SetHeight( int height );
	void Draw(sf::RenderTarget *target);
};

struct VictoryScreen4Player : MatchResultsScreen
{
	enum Action
	{
		A_SHOW_WINNER,
		A_FADE_WINNER,
		A_SHOW_RESULTS,
		A_Count
	};

	VictoryScreen4Player(MatchStats *mStats);
	~VictoryScreen4Player();
	void Draw( sf::RenderTarget *target );
	bool Update();
	void ResetSprites();
	void UpdateSprites();
	void Reset();
	GameSession *game;

	int fadeFrames;
	
	sf::Sprite bgSpr;
	sf::Sprite winSpr;
	sf::Sprite kinSpr;
	sf::Vertex fadeQuad[4];

	

	PlayerInfoBar *playerBar[4];
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