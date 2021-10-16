#ifndef __SEQUENCEW4_H__
#define __SEQUENCEW4_H__

#include "Sequence.h"

struct GroundedWarper;

struct Tiger;
struct CrawlerQueen;
struct SequenceTiger;
struct SequenceBird;
struct SequenceCrawler;

struct CrawlerPreFight2Scene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		CONV,
		Count
	};

	CrawlerQueen *queen;

	CrawlerPreFight2Scene();

	void SetupStates();
	void SetEntranceShot();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();

	
};

struct CrawlerPostFight2Scene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		TIGERRUMBLE,
		CONV2,
		CRAWLERLEAVE,
		Count
	};


	SequenceCrawler *seqCrawler;
	GroundedWarper *warper;

	CrawlerPostFight2Scene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
	

	
};

struct CrawlerVSTigerScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		CONV1,
		CRAWLER_FLIP_SWITCH,
		NEXUS_EXPLODE,
		CONV2,
		TIGER_LUNGE,
		TIGER_KILL_CRAWLER,
		CONV3,
		CRAWLER_DEATH,
		Count
	};

	//Tiger *tiger;
	SequenceTiger *seqTiger;
	SequenceCrawler *seqCrawler;

	CrawlerVSTigerScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
	void SetEntranceShot();
};

struct TigerPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		TIGERENTRANCE,
		TIGERCRAWLERCONV,
		KINAPPROACH,
		TIGERKINCONV,
		Count
	};

	Tiger *tiger;

	TigerPreFightScene();

	void SetupStates();
	bool IsAutoRunState();
	//void SetEntranceShot();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct TigerPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV1,
		BIRD_ENTER,
		CONV2,
		BIRD_SWOOP,
		CONV3,
		BIRD_EXIT,
		Count
	};

	//Tiger *tiger;
	SequenceTiger *seqTiger;
	SequenceBird *seqBird;

	TigerPostFightScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct BirdTigerAllianceScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		TIGERSTORY,
		FADEOUT,
		Count
	};

	SequenceTiger *seqTiger;
	SequenceBird *seqBird;

	BirdTigerAllianceScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
	
};

struct BirdVSTigerScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		BIRDRETREAT,
		TIGERCHASE,
		FADEOUT,
		Count
	};

	SequenceTiger *seqTiger;
	SequenceBird *seqBird;

	BirdVSTigerScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
	void StartRunning();
};

#endif