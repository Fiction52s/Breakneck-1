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

	CoyoteSleepScene();

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

struct Coyote;
struct GroundedWarper;

struct CoyotePreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		COYOTECONV,
		COYOTEFACES,
		Count
	};

	CoyotePreFightScene();

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
	

	Coyote *coy;

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

	Coyote *coy;
	GroundedWarper *warper;

	CoyotePostFightScene();
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

	CoyoteAndSkeletonScene();
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