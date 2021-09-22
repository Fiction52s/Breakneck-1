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

struct CoyoteHelper;
struct Skeleton;
struct Tiger;
struct SequenceCoyote;
struct MusicInfo;

struct EnterFortressScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		COYOTE_ENTRANCE,
		WAIT2,
		FACES1,
		COYOTECONV,
		FACES2,
		SPLITUP,
		Count
	};

	SequenceCoyote *seqCoyote;
	MusicInfo *wind;
	MusicInfo *specialMusic;
	MusicInfo *prevMusic;

	EnterFortressScene();

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

	TigerAndBirdTunnelScene();
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

	CoyoteHelper *coyHelper;
	Skeleton *skeleton;

	SkeletonPreFightScene();

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

	CoyoteHelper *coyHelper;
	Skeleton *skeleton;


	SkeletonPostFightScene();
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

	Tiger *tiger;

	TigerPreFight2Scene();

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

	Tiger *tiger;

	TigerPostFight2Scene();
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