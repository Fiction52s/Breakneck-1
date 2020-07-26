#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

struct Session;
struct MultiplayerBase;
struct BasicAirEnemyParams;
struct Enemy;
struct HUD;
struct Gator;
struct Bird;

struct GameMode
{
	GameMode();
	virtual ~GameMode() {}
	Session *sess;
	bool done;

	virtual HUD *CreateHUD() { return NULL; }
	virtual void StartGame() = 0;
	virtual bool CheckVictoryConditions() = 0;
	virtual void EndGame() = 0;
	virtual void Setup() {}
	bool IsDone() { return done; }
	virtual int GetNumStoredBytes() { return 0; }
	virtual void StoreBytes(unsigned char *bytes) {}
	virtual void SetFromBytes(unsigned char *bytes) {}
};

struct BasicMode : GameMode
{


	BasicMode();

	HUD *CreateHUD();
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
		int p0HitTargets;
		int p1HitTargets;
	};

	BasicAirEnemyParams *ap0;
	BasicAirEnemyParams *ap1;
	MultiplayerBase *p0Base;
	MultiplayerBase *p1Base;
	int p0Score;
	int p1Score;
	std::vector<Enemy*> enemies;
	int totalStoredBytes;
	int totalProgressTargets;
	int p0HitTargets;
	int p1HitTargets;

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);

	ReachEnemyBaseMode();
	~ReachEnemyBaseMode();

	void Setup();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

struct FightMode : GameMode
{
	struct MyData
	{
		int p0Health;
		int p1Health;
		int p0Meter;
		int p1Meter;
	};

	MyData data;

	BasicAirEnemyParams *gatorParams;
	Gator *testGator;
	BasicAirEnemyParams *birdParams;
	Bird *testBird;

	int maxHealth;
	int meterSection;
	int numMeterSections;

	FightMode();
	~FightMode();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	HUD *CreateHUD();
	void Setup();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

#endif