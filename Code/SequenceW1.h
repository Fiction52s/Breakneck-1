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
struct SequenceCrawler;

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
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);

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
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);
};

struct CrawlerPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		CRAWLER_SLASHED,
		//KIN_1,
		//CRAWLER_ANGRY,
		//KIN_2,
		//CRAWLER_SCARED,
		FADE_IN,
		CRAWLER_GIBBERISH,
		CRAWLER_RETREAT,
		KIN_FINAL_FACE,
		WAIT,
		//PLAYMOVIE,
		Count
	};

	SequenceCrawler *seqCrawler;
	//CrawlerQueen *queen;
	//GroundedWarper *warper;

	CrawlerPostFightScene();

	void SetupStates();
	void ReturnToGame();
	void AddGroups();
	void AddShots();
	//void StartRunning();
	void AddFlashes();
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
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);
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
	void LayeredDraw(int p_drawLayer, sf::RenderTarget *target);


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
	void LayeredDraw(int layer, sf::RenderTarget *target);
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
	void LayeredDraw(int layer, sf::RenderTarget *target);
	void Reset();
	void AddGroup(const std::string &name);

	std::vector<ConversationGroup*> groups;
	void UpdateSceneLabel();
	int gIndex;

	sf::Text sceneLabel;



	GameSession *owner;
};


#endif