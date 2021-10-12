#ifndef __SEQUENCEW5_H__
#define __SEQUENCEW5_H__

#include "Sequence.h"

struct Bird;
struct GroundedWarper;
struct SequenceBird;
struct SequenceTiger;
struct SequenceGator;

struct BirdPreFight2Scene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		BIRDCONV,
		Count
	};

	BirdPreFight2Scene();

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

struct BirdPostFight2Scene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		BIRDCONV,
		BIRDLEAVE,
		Count
	};

	SequenceBird *seqBird;
	GroundedWarper *warper;

	BirdPostFight2Scene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();

	
};

struct Gator;

struct GatorPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		GATORCONV,
		Count
	};

	GatorPreFightScene();

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();

	Gator *gator;
};

struct GatorPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		GATORANGRY,
		BIRD_BREAKS_FREE,
		BIRD_KICK,
		TIGERFALL,
		TIGER_WALK_TO_BIRD,
		TIGER_ROAR,
		TIGER_PUTS_BIRD_ON_BACK,
		TIGER_LEAVES_WITH_BIRD,
		Count
	};

	SequenceGator *seqGator;
	SequenceBird *seqBird;
	SequenceTiger *seqTiger;

	GatorPostFightScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct BirdTigerApproachScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		WALK_OUT,
		FADEOUT,
		Count
	};

	SequenceBird *seqBird;
	SequenceTiger *seqTiger;

	BirdTigerApproachScene();
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