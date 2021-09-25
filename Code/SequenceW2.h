#ifndef __SEQUENCEW2_H__
#define __SEQUENCEW2_H__

#include "Sequence.h"

//struct BirdBoss;
struct Bird;
struct GroundedWarper;
struct SequenceCrawler;
struct SequenceBird;

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
		WAIT,
		BIRDCONV,
		BIRDLEAVE,
		Count
	};

	BirdPostFightScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();

	Bird *bird;
	GroundedWarper *warper;
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