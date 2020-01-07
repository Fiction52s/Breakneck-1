#ifndef __SEQUENCEW1_H__
#define __SEQUENCEW1_H__

#include "Sequence.h"

struct AfterCrawlerFightSeq : BasicBossScene
{
	enum State
	{
		FADE,
		PLAYMOVIE,
		Count
	};

	AfterCrawlerFightSeq(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void StartRunning();
	void AddMovies();
	void AddPoints();
	void UpdateState();
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

	GameSession *owner;

	sfe::Movie mov;

	sf::Vertex darkQuad[4];
};

struct CrawlerQueen;
struct TextDisp;
struct Conversation;
struct ConversationGroup;

struct CrawlerAttackSeq : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		KINSTOP,
		ROCKSFALL,
		CRAWLERSWOOP,
		DIGGINGAROUND,
		THROWOUT,
		CRAWLERFACE,
		CRAWLERTALK,
		KINTALK,
		Count
	};

	CrawlerAttackSeq(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void SpecialInit();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);

	void UpdateCrawlerSwoop();

	CrawlerQueen *queen;

	Tileset *ts_queenGrab;
	sf::Sprite queenGrabSprite;
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