#ifndef __SEQUENCEW2_H__
#define __SEQUENCEW2_H__

#include "Sequence.h"

struct BirdBoss;

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

	BirdBoss *bird;
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