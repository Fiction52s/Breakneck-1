#ifndef __SEQUENCEW4_H__
#define __SEQUENCEW4_H__

#include "Sequence.h"


struct CrawlerPreFight2Scene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		CONV,
		Count
	};

	CrawlerPreFight2Scene();

	void SetupStates();
	void SetEntranceShot();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct CrawlerPostFight2Scene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		CRAWLERLEAVE,
		Count
	};

	CrawlerPostFight2Scene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};


struct TigerPreFightScene : BasicBossScene
{
	enum State
	{
		ENTRANCE,
		WAIT,
		TIGERENTRANCE,
		TIGERCRAWLERCONV,
		KINAPPROACH,
		TIGERKINCONV,
		Count
	};

	TigerPreFightScene();

	void SetupStates();
	bool IsAutoRunState();
	//void SetEntranceShot();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct TigerPostFightScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		BIRDRESCUETIGER,
		Count
	};

	TigerPostFightScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
};

struct BirdTigerAllianceScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		TIGERSTORY,
		FADEOUT,
		Count
	};

	BirdTigerAllianceScene();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddPoints();
	void AddFlashes();
	void AddEnemies();
	void AddGroups();
	void UpdateState();
	
};

struct BirdVSTigerScene : BasicBossScene
{
	enum State
	{
		FADE,
		WAIT,
		CONV,
		BIRDRETREAT,
		TIGERCHASE,
		FADEOUT,
		Count
	};

	BirdVSTigerScene();
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