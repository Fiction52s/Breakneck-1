#ifndef __HUD_H__
#define __HUD_H__

#include "ImageText.h"
#include "Tileset.h"
#include <SFML\Graphics.hpp>
#include "GameSession.h"

struct Session;
struct GameSession;


struct RaceFightHUD
{
	/*enum MaskState
	{
		NEUTRAL,
		RED,
		BLUE
	};*/

	enum PlayerColor
	{
		BLUE,
		RED,
		GREEN,
		PURPLE,
		NONE
	};

	RaceFightHUD(GameSession::RaceFight *rf );
	sf::Sprite mask;
	ImageText *scoreRed;
	ImageText *scoreBlue;
	GameSession *owner;
	Tileset *ts_mask;

	Tileset *ts_smallMask;

	Tileset *ts_fx_gainPointBlue;
	Tileset *ts_fx_gainPointRed;
	Tileset *ts_ready;

	sf::Sprite smallMask;

	void Draw(sf::RenderTarget *target);
	void UpdateScore( PlayerColor pc, int newScore );
	void UpdateScore();
	

	void SetRaceWinner( PlayerColor pc );
	void ScorePoint( PlayerColor pc );

	GameSession::RaceFight *raceFight;
};



struct Minimap;
struct Actor;
struct MomentumBar;
struct KinRing;


struct KinMask
{
	enum Expr
	{
		Expr_NEUTRAL,
		Expr_HURT,
		Expr_SPEED1,
		Expr_SPEED2,
		Expr_DESP,
		Expr_DEATH,
		Expr_DEATHYELL,
		Expr_NONE
	};

	KinMask( Actor *a);
	~KinMask();
	void Update( int speedLevel,
		bool desp );
	void Draw(sf::RenderTarget *target);
	void SetTopLeft(sf::Vector2f &pos);
	sf::Vector2f GetTopLeft();

	void Reset();

	void SetExpr(Expr ex);
	int frame;
	Expr expr;

	Actor *actor;
	Tileset *ts_face;
	sf::Sprite face;
	sf::Sprite faceBG;
	Session *sess;
	MomentumBar *momentumBar;
	KinRing *kinRing;

	Tileset *ts_newShardMarker;
	sf::Sprite shardMarker;
};

struct AdventureHUD
{
	enum State
	{
		SHOWN,
		ENTERING,
		EXITING,
		HIDDEN
	};

	AdventureHUD();
	~AdventureHUD();
	void Hide(int frames = 0);
	void Show(int frames = 0);
	void Draw(sf::RenderTarget *target);
	void Update();
	void Reset();
	bool IsHidden();
	bool IsShown();

	int processFrames;
	bool show;
	int frame;
	State state;

	sf::Text flyCountText;

	sf::Vector2f miniShowPos;
	sf::Vector2f miniHidePos;

	sf::Vector2f keyMarkerShowPos;
	sf::Vector2f keyMarkerHidePos;

	sf::Vector2f kinMaskShowPos;
	sf::Vector2f kinMaskHidePos;

	CubicBezier showBez;
	CubicBezier hideBez;

	sf::Vector2f flyCountTextShowPos;
	sf::Vector2f flyCountTextHidePos;
	
	Minimap *mini;
	KinMask *kinMask;
	KeyMarker *keyMarker;

	Session *sess;
};

#endif