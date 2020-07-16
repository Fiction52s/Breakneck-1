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

	KinMask( Actor *a);
	~KinMask();
	void Update( int speedLevel,
		bool desp );
	void Draw(sf::RenderTarget *target);
	void SetTopLeft(sf::Vector2f &pos);
	sf::Vector2f GetTopLeft();
	void Reset();
	void SetExpr(Expr ex);
	
};

struct HUD
{
	enum HUDType
	{
		ADVENTURE,
		FIGHT,
		Count
	};

	Session *sess;
	HUDType hType;
	Minimap *mini;

	HUD();
	virtual ~HUD();
	virtual void Hide(int frames = 0) {}
	virtual void Show(int frames = 0) {}
	virtual void Draw(sf::RenderTarget *target) {}
	virtual void Update() {}
	virtual void Reset() {}
	virtual bool IsHidden() { return false; }
	virtual bool IsShown() { return true; }
};

struct FightHUD : HUD
{
	FightHUD();
	~FightHUD();
	void Draw(sf::RenderTarget *target);
	void Update();
	void Reset();
};

struct AdventureHUD : HUD
{
	enum State
	{
		SHOWN,
		ENTERING,
		EXITING,
		HIDDEN
	};

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

	

	AdventureHUD();
	~AdventureHUD();
	
	void Hide(int frames = 0);
	void Show(int frames = 0);
	void Draw(sf::RenderTarget *target);
	void Update();
	void Reset();
	bool IsHidden();
	bool IsShown();
};

#endif