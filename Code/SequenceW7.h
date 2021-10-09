#ifndef __SEQUENCEW7_H__
#define __SEQUENCEW7_H__

#include "Sequence.h"

struct Bird;
struct GreySkeleton;
struct SequenceBird;

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

	BirdChaseScene();

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

	Bird *bird;

	BirdPreFight3Scene();

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

	SequenceBird *seqBird;

	BirdPostFight3Scene();
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

	GreySkeleton *greySkeleton;

	FinalSkeletonPreFightScene();

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

	GreySkeleton *greySkeleton;

	FinalSkeletonPostFightScene();
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