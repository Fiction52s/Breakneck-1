#ifndef __SEQUENCEW1_H__
#define __SEQUENCEW1_H__

#include "Sequence.h"



struct CrawlerQueen;
struct TextDisp;
struct Conversation;
struct ConversationGroup;
struct Nexus;
struct Session;
struct ShapeEmitter;
struct Nexus;
struct GroundedWarper;

//BirdPostFightScene

struct CrawlerAttackScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		KINSTOP,
		ROCKSFALL,
		CRAWLERSWOOP,
		DIGGINGAROUND,
		Count
	};

	CrawlerAttackScene();
	~CrawlerAttackScene();

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

	Tileset *ts_queenGrab;
	sf::Sprite queenGrabSprite;

	GameSession *myBonus;
};

struct CrawlerPreFightScene : BasicBossScene
{
	enum State
	{
		DIGGINGAROUND,
		THROWOUT,
		CRAWLERFACE,
		CRAWLERTALK,
		KINTALK,
		Count
	};

	CrawlerQueen *queen;

	CrawlerPreFightScene();

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
};

struct CrawlerPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		PLAYMOVIE,
		Count
	};

	CrawlerQueen *queen;
	GroundedWarper *warper;

	CrawlerPostFightScene();

	void SetupStates();
	void ReturnToGame();
	void StartRunning();
	void AddMovies();
	void AddPoints();
	void UpdateState();
};

struct NexusCoreSeq : Sequence
{
	enum State
	{
		FADETOBLACK,
		ENTERCORE,
		DESTROYCORE,
		EXITCORE,
		Count
	};

	NexusCoreSeq();
	~NexusCoreSeq();
	void SpecialInit();
	void ReturnToGame();
	void SetupStates();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	sf::Vertex darkQuad[4];

	Nexus *nexus;

	MovingGeoGroup geoGroup;
	ShapeEmitter *emitter;
};

struct GetAirdashPowerScene : BasicBossScene
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
	GetAirdashPowerScene();
	//bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);


	void StartRunning();

	void SetupStates();
	void ReturnToGame();
	//void AddShots();
	void AddPoints();
	//void AddFlashes();
	//void AddEnemies();
	//void AddGroups();
	//void SpecialInit();
	void UpdateState();
	void AddMovies();



	sf::Sprite darkAuraSprite;
	Tileset *ts_darkAura;
	sf::RectangleShape darkRect;
	MusicInfo *sceneMusic;

	//sfe::Movie mov;
};

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
	GetAirdashPowerSeq();
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();


	sf::Sprite darkAuraSprite;
	Tileset *ts_darkAura;
	sf::RectangleShape darkRect;
	MusicInfo *sceneMusic;

	sfe::Movie mov;

	Session *sess;
};

struct TextTestSeq : Sequence
{
	enum State
	{
		TALK,
		END,
		Count
	};

	TextTestSeq();
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

	Session *sess;
	int frame;

	sf::Text sceneLabel;



	GameSession *owner;
};


#endif