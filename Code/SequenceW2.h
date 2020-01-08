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

	BirdPreFightScene(GameSession *owner);

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

	BirdPostFightScene(GameSession *owner);
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