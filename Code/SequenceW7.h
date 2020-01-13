#ifndef __SEQUENCEW7_H__
#define __SEQUENCEW7_H__

#include "Sequence.h"


struct BirdChaseScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		SKELETONCONV,
		SKELETONANDBIRDLEAVE,
		Count
	};

	BirdChaseScene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};



struct BirdPreFight3Scene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		SKELETONCONV,
		Count
	};

	BirdPreFight3Scene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct BirdPostFight3Scene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		BIRDCONV,
		BIRDDIES,
		Count
	};

	BirdPostFight3Scene(GameSession *owner);
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct FinalSkeletonPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		SKELETONCONV,
		Count
	};

	FinalSkeletonPreFightScene(GameSession *owner);

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

struct FinalSkeletonPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		SKELETONCONV,
		SKELETONDIES,
		Count
	};

	FinalSkeletonPostFightScene(GameSession *owner);
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