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
	virtual int GetNumStoredBytes() { return 0; }
	virtual void StoreBytes(unsigned char *bytes) {}
	virtual void SetFromBuffer(unsigned char *buf) {}
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
	struct ReachEnemyBaseModeData
	{
		int p0Score;
		int p1Score;
	};

	BasicAirEnemyParams *ap0;
	BasicAirEnemyParams *ap1;
	MultiplayerBase *p0Base;
	MultiplayerBase *p1Base;
	int p0Score;
	int p1Score;

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBuffer(unsigned char *buf);

	ReachEnemyBaseMode();
	~ReachEnemyBaseMode();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

#endif