#ifndef __SEQUENCEW1_H__
#define __SEQUENCEW1_H__

#include "Sequence.h"

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

	CrawlerAfterFightSeq(GameSession *owner);
	bool Update();
	void Draw(sf::RenderTarget *target,
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

struct CrawlerDefeatedSeq : Sequence
{
	enum State
	{
		PLAYMOVIE,
		END,
		Count
	};

	State state;
	int stateLength[Count];
	CrawlerDefeatedSeq(GameSession *owner);
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	GameSession *owner;

	sfe::Movie mov;

	sf::Vertex darkQuad[4];
};

struct NexusCore1Seq : Sequence
{
	enum State
	{
		FADETOBLACK,
		ENTERCORE,
		DESTROYCORE,
		//FADEEXIT,
		EXITCORE,
		END,
		Count
	};

	State state;
	int stateLength[Count];
	NexusCore1Seq(GameSession *owner);
	~NexusCore1Seq();
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	//sf::Texture tex;

	//std::string imageNames[52];

	GameSession *owner;
	//static void LoadNextTex(NexusCore1Seq *seq);
	//boost::thread *loadThread;
	//sf::Image coreImages[52];
	//Tileset *ts_core[52];
	//Tileset *ts_firstCore;
	//sf::Texture coreTex[2];
	//sf::Sprite coreSprite;

	sfe::Movie mov;

	sf::Vertex darkQuad[4];
};

struct CrawlerQueen;
struct TextDisp;
struct Conversation;
struct ConversationGroup;


struct CrawlerAttackSeq1 : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		TALK,
		Count
	};

	CrawlerAttackSeq1(GameSession *owner);
	void SetupStates();
	void UpdateState();
};

struct CrawlerAttackSeq : Sequence
{
	enum State
	{
		KINSTOP,
		ROCKSFALL,
		CRAWLERSWOOP,
		DIGGINGAROUND,
		THROWOUT,
		CRAWLERTALK,
		KINTALK,
		END,
		Count
	};
	~CrawlerAttackSeq();
	State state;
	int stateLength[Count];
	void Init();
	CrawlerAttackSeq(GameSession *owner);
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	TextDisp *textDisp;
	CrawlerQueen *queen;

	int frame;

	PoiInfo *camPoint0;
	PoiInfo *camPoint1;
	PoiInfo *roomCenter;
	PoiInfo *surface;
	PoiInfo *throwkin;

	Tileset *ts_queenGrab;
	sf::Sprite queenGrabSprite;
	FlashedImage *detailedGrab;

	FlashedImage *crawlerFace;
	FlashedImage *kinFace;

	GameSession *owner;
};

struct TextTestSeq : Sequence
{
	enum State
	{
		TALK,
		END,
		Count
	};

	TextTestSeq(GameSession *owner);
	~TextTestSeq();
	State state;
	int stateLength[Count];
	void Init();

	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();
	void AddGroup(const std::string &name);

	std::vector<ConversationGroup*> groups;
	void UpdateSceneLabel();
	int gIndex;
	int cIndex;


	int frame;

	sf::Text sceneLabel;



	GameSession *owner;
};

struct Nexus;


struct GetAirdashPowerSeq : Sequence
{
	enum State
	{
		KIN_KNEELING,
		START_MEDITATE,
		FADE_BACKGROUND,
		EXPEL_ENERGY,
		WAITAFTEREXPEL,
		MASKOFF,
		PLAYMOVIE,
		FADE_BACK,
		END,
		Count
	};

	State state;
	int stateLength[Count];
	GetAirdashPowerSeq(GameSession *owner);
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();


	sf::Sprite darkAuraSprite;
	Tileset *ts_darkAura;
	sf::RectangleShape darkRect;
	MusicInfo *sceneMusic;

	sfe::Movie mov;

	GameSession *owner;
};

#endif