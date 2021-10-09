#ifndef __SEQUENCEW3_H__
#define __SEQUENCEW3_H__

#include "Sequence.h"

struct SequenceCoyote;
struct SequenceSkeleton;

struct CoyoteSleepScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		COYOTEWAKE,
		COYOTELEAVE,
		Count
	};

	SequenceCoyote *seqCoyote;

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

	SequenceCoyote *seqCoyote;
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
		CONV1,
		SKELETONCHARGELASER,
		SKELETONLASER,
		CONV2,
		COYOTERETREAT,
		SKELETONIMPOSING,
		SKELETONHOPDOWN,
		SKELETONAPPROACH,
		SKELETONPOINT,
		CONV3,
		SKELETONEXIT,
		Count
	};

	SequenceCoyote *seqCoyote;
	SequenceSkeleton *seqSkeleton;
	MusicInfo *specialMusic;
	MusicInfo *prevMusic;

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