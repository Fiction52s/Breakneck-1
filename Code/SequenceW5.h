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
		GATOR_CONV1,
		GATOR_THROW_SUPER_ORB,
		GATOR_FLOAT_WITH_ORB,
		GATOR_CONV2,
		GATOR_SMASH_ORB,
		GATOR_BEAT_UP_KIN,
		BIRD_BREAKS_FREE,
		BIRD_FALL,
		BIRD_TRY_TO_GET_UP,
		BIRD_CONV1,
		BIRD_GET_UP,
		BIRD_CONV2,
		BIRD_FLOAT_UP,
		BIRD_FLOAT_CHARGE,
		BIRD_KICK,
		STORY_PAUSE_TEST,
		BIRD_KICK_2,
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