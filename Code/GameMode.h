#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

struct Session;
struct MultiplayerBase;
struct BasicAirEnemyParams;

struct GameMode
{
	GameMode();
	virtual ~GameMode() {}
	Session *sess;
	bool done;

	virtual void StartGame() = 0;
	virtual bool CheckVictoryConditions() = 0;
	virtual void EndGame() = 0;
	bool IsDone() { return done; }
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
	BasicAirEnemyParams *ap0;
	BasicAirEnemyParams *ap1;
	MultiplayerBase *p0Base;
	MultiplayerBase *p1Base;
	int p0Score;
	int p1Score;

	ReachEnemyBaseMode();
	~ReachEnemyBaseMode();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

#endif