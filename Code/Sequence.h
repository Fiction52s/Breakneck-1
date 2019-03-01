#ifndef __CUTSCENE_H__
#define __CUTSCENE_H__
#include "Tileset.h"
#include "SFML/Graphics.hpp"
#include <list>
#include "Movement.h"

struct GameSession;
struct MovementSequence;
struct Cutscene
{
	Cutscene( GameSession *owner,
		sf::Vector2i & origin );
	~Cutscene();
	Tileset **tilesets;
	bool LoadFromFile( const std::string &path );
	sf::View & GetView( int frame );
	void Draw( sf::RenderTarget *target, int frame );
	GameSession *owner;
	//CamInfo *cameras;
	sf::View *cameras;
	std::list<sf::Sprite> **activeSprites;
	int totalFrames;
	bool init;
	sf::Vector2f origin;
};

struct Sequence
{
	//Sequence *next;
	//Sequence *prev;
	Sequence()
		:frameCount(-1),frame(0)
	{

	}
	int frameCount;
	int frame;
	virtual bool Update() = 0;
	virtual void Reset() = 0;
	virtual void Draw( sf::RenderTarget *target, 
		EffectLayer layer = EffectLayer::IN_FRONT ) = 0;
};

struct ShipExitSeq : Sequence
{
	enum State
	{
		SHIP_SWOOP,

	};

	ShipExitSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	Tileset *ts_ship;
	sf::Sprite shipSprite;
	MovementSequence shipMovement;
	MovementSequence center;
	GameSession *owner;
	int enterTime;
	int exitTime;
	sf::Vector2<double> abovePlayer;
	sf::Vector2<double> origPlayer;
	sf::Vector2<double> attachPoint;
};

struct CrawlerFightSeq : Sequence
{
	enum State
	{
		INIT,
		FIGHTSTARTMSG,
		Count
	};


	State state;
	int stateLength[State::Count];

	CrawlerFightSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();
	void StartFightMsg();

	int startFightMsgFrame;
	MovementSequence camMove0;
	//Tileset *ts_fall;
	//Tileset *ts_land;

	sf::Vector2<double> startFallPos;
	sf::Vector2<double> nexus1Pos;
	sf::Vector2<double> cfightCenter;
	Edge *landGround;
	double landQuant;
	int playerStartFrame;


	

	Tileset *ts_crawlerBoss;


	//Tileset *

	GameSession *owner;
};

struct CrawlerAfterFightSeq : Sequence
{
	/*enum State
	{
		INIT,
		FIGHTSTARTMSG,
		Count
	};


	State state;
	int stateLength[State::Count];*/

	CrawlerAfterFightSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();
	//void StartFightMsg();

	//int startFightMsgFrame;
	MovementSequence camMove0;
	//Tileset *ts_fall;
	//Tileset *ts_land;

	sf::Vector2<double> startFallPos;
	sf::Vector2<double> nexus1Pos;
	sf::Vector2<double> cfightCenter;
	Edge *landGround;
	double landQuant;
	int playerStartFrame;


	

	Tileset *ts_crawlerBoss;


	//Tileset *

	GameSession *owner;
};

struct NexusCore1Seq : Sequence
{
	enum State
	{
		ENTERCORE,
		DESTROYCORE,
		FADEEXIT,
		EXITCORE,
		END,
		Count
	};

	State state;
	NexusCore1Seq(GameSession *owner);
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();


	GameSession *owner;

	sf::Vertex darkQuad[4];
};

struct CrawlerQueen;
struct CrawlerAttackSeq : Sequence
{
	enum State
	{
		KINSTOP,
		ROCKSFALL,
		CRAWLERSWOOP,
		DIGGINGAROUND,
		THROWOUT,
		CONVERSATION,
		END,
		Count
	};

	State state;
	int stateLength[Count];
	void Init();
	CrawlerAttackSeq(GameSession *owner);
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	CrawlerQueen *queen;

	int frame;

	PoiInfo *camPoint0;
	PoiInfo *camPoint1;
	PoiInfo *roomCenter;
	PoiInfo *surface;
	PoiInfo *throwkin;

	Tileset *ts_queenGrab;
	sf::Sprite queenGrabSprite;

	GameSession *owner;
};

struct Nexus;
struct EnterNexus1Seq : Sequence
{
	enum State
	{
		INIT,
		FIGHTSTARTMSG,
		Count
	};


	
	State state;
	//int stateLength[State::Count];*/

	Nexus *nexus;
	EnterNexus1Seq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();
	//void StartFightMsg();

	//int startFightMsgFrame;
	//MovementSequence camMove0;
	//Tileset *ts_fall;
	//Tileset *ts_land;

	//sf::Vector2<double> startFallPos;
	//sf::Vector2<double> nexus1Pos;
	//sf::Vector2<double> cfightCenter;
	Edge *afterNexusGround;
	double afterQuantity;
	//int playerStartFrame;


	

	//Tileset *ts_crawlerBoss;


	//Tileset *

	GameSession *owner;
};

struct Barrier;
struct MeetCoyoteSeq : Sequence
{
	enum State
	{
		//INIT,
		//FIGHTSTARTMSG,
		Count
	};

	Edge *coyGround;
	double coyQuant;

	//State state;
	//int stateLength[State::Count];

	MeetCoyoteSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();
	void CoyoteGone();

	Edge *startGround;
	double startQuant;
	bool coyoteGone;
	int coyoteGoneFrame;

	Edge *coyotePos;
	double coyoteQuant;

	int confirmTestFrame;

	GameSession *owner;
};

struct CoyoteFightSeq : Sequence
{
	CoyoteFightSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	Barrier *bleft;
	Barrier *bright;
	Barrier *btop;
	Barrier *bbot;

	sf::Vector2f kinCamStart;
	sf::Vector2f coyCamStart;

	Edge *startEdge;
	double startQuant;

	Edge *coyStartEdge;
	double coyStartQuant;

	MovementSequence camMove0;

	GameSession *owner;
};

struct CoyoteTalkSeq : Sequence
{
	CoyoteTalkSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	GameSession *owner;
};

struct SkeletonAttackCoyoteSeq : Sequence
{
	SkeletonAttackCoyoteSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	GameSession *owner;
};

struct SkeletonFightSeq : Sequence
{
	SkeletonFightSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	GameSession *owner;
};

struct BirdCrawlerAllianceSeq : Sequence
{
	BirdCrawlerAllianceSeq( GameSession *owner );
	bool Update();
	void Draw( sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	GameSession *owner;
};

#endif 
