#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

struct Session;

struct GameMode
{

	GameMode();
	Session *sess;

	virtual void StartGame() = 0;
	virtual bool CheckVictoryConditions() = 0;
	virtual void EndGame() = 0;
};

struct BasicMode : GameMode
{
	BasicMode();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

struct ReachEnemyBaseMode : GameMode
{

	ReachEnemyBaseMode();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

#endif