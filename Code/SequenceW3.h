#ifndef __SEQUENCEW3_H__
#define __SEQUENCEW3_H__

#include "Sequence.h"

struct CoyoteSleepScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		COYOTEWAKE,
		Count
	};

	CoyoteSleepScene(GameSession *owner);

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

struct CoyotePreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		COYOTECONV,
		COYOTEFACES,
		Count
	};

	CoyotePreFightScene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct CoyotePostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		COYOTECONV,
		NEXUSEXPLODE,
	//	COYOTECONV1,
		COYOTELEAVE,
		Count
	};

	CoyotePostFightScene(GameSession *owner);
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct CoyoteAndSkeletonScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		SHOWIMAGE,
		SKELECOYCONV,
		Count
	};

	CoyoteAndSkeletonScene(GameSession *owner);
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

#endif