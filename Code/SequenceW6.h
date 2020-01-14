#ifndef __SEQUENCEW6_H__
#define __SEQUENCEW6_H__

#include "Sequence.h"

//1. enter fortress
//2. tiger and bird in tunnels
//2. preskeletonfight
//3. postskeletonfight, top 3, and coyotedeath
//4. tiger and bird at nexus get mind controlled. 
//	skeleton leaves
//5. pre tiger fight
//6. post tiger fight

struct EnterFortressScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		COYOTECONV,
		SPLITUP,
		Count
	};

	EnterFortressScene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct TigerAndBirdTunnelScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		FADEOUT,
		Count
	};

	TigerAndBirdTunnelScene(GameSession *owner);
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

struct SkeletonPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		SKELECONV,
		Count
	};

	SkeletonPreFightScene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct SkeletonPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		MOVIE,
		TOP3TRANSFORMATIONSTART,
		TOP3BIRDANDTIGER,
		TOP3SKELETON,
		SKELETONLEAVES,
		KINMOVE,
		COYOTEDEATH,
		Count
	};

	SkeletonPostFightScene(GameSession *owner);
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	
	void AddMovies();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();

	bool IsAutoRunState();
};

struct TigerPreFight2Scene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		TIGERCONV,
		Count
	};

	TigerPreFight2Scene(GameSession *owner);

	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct TigerPostFight2Scene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		TIGERDEATH,
		Count
	};

	TigerPostFight2Scene(GameSession *owner);
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