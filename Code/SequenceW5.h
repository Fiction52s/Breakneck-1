#ifndef __SEQUENCEW5_H__
#define __SEQUENCEW5_H__

#include "Sequence.h"

struct Bird;
struct GroundedWarper;
struct SequenceBird;

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
		BIRDATTACKS,
		TIGERFALL,
		TIGERLEAVESWITHBIRD,
		Count
	};

	GatorPostFightScene();
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

struct BirdTigerApproachScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		ENTERGATORAREA,
		FADEOUT,
		Count
	};

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