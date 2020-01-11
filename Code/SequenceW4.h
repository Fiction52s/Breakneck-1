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

	CrawlerPreFight2Scene(GameSession *owner);

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

	CrawlerPostFight2Scene(GameSession *owner);
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

	TigerPreFightScene(GameSession *owner);

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

	TigerPostFightScene(GameSession *owner);
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