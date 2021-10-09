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
struct SequenceTiger;
struct SequenceBird;
struct SequenceSkeleton;

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
		SUMMON_SCORPION,
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
		WALK_IN,
		CONV,
		WALK_OUT,
		FADEOUT,
		Count
	};

	SequenceTiger *seqTiger;
	SequenceBird *seqBird;

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

struct MindControlScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV1,
		NEXUS_EXPLODE,
		CONV2,
		SKELETON_ENTRANCE,
		CONV3,
		SKELETON_FACE_PRE_MIND_CONTROL,
		SKELETON_MIND_CONTROL,
		CONV4,
		MIND_CONTROL_FINISH,
		CONV5,
		BIRD_WALK_OVER_TO_SKELETON,
		ENTRANCE,
		CONV6,
		SKELETON_JUMP_ONTO_BIRD,
		TIGER_FACE_KIN,
		SKELETON_BIRD_EXIT,
		Count
	};

	SequenceTiger *seqTiger;
	SequenceBird *seqBird;
	SequenceSkeleton *seqSkeleton;

	MindControlScene();

	bool IsAutoRunState();
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