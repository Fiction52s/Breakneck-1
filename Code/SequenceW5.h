#ifndef __SEQUENCEW5_H__
#define __SEQUENCEW5_H__

#include "Sequence.h"

struct BirdPreFight2Scene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		BIRDCONV,
		Count
	};

	BirdPreFight2Scene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
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

	BirdPostFight2Scene(GameSession *owner);
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};


struct GatorPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		GATORCONV,
		Count
	};

	GatorPreFightScene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
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

	GatorPostFightScene(GameSession *owner);
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
		ENTERGATORAREA,
		FADEOUT,
		Count
	};

	BirdTigerApproachScene(GameSession *owner);
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