#ifndef __SEQUENCEW2_H__
#define __SEQUENCEW2_H__

#include "Sequence.h"

//struct BirdBoss;
struct Bird;
struct GroundedWarper;
struct SequenceCrawler;
struct SequenceBird;
struct GameSession;

struct BirdPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		BIRDWALK,
		BIRDFALL,
		BIRDCONV,
		Count
	};

	BirdPreFightScene();

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();

	Bird *bird;
};

struct BirdPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		CUT_IMAGE,
		FADE_IN,
		WAIT,
		BIRDCONV,
		BIRDLEAVE,
		Count
	};

	SequenceBird *seqBird;

	BirdPostFightScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct BirdCrawlerAllianceTransitionScene : BasicBossScene
{
	enum State
	{
		TRANSITION,
		Count
	};

	GameSession *myBonus;

	BirdCrawlerAllianceTransitionScene();
	~BirdCrawlerAllianceTransitionScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct BirdCrawlerAllianceScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CRAWLERARRIVE,
		CONV,
		FADEOUT,
		Count
	};

	SequenceCrawler *seqCrawler;
	SequenceBird *seqBird;

	BirdCrawlerAllianceScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};


#endif