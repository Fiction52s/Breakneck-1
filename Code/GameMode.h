#ifndef __GAMEMODE_H__
#define __GAMEMODE_H__

#include <vector>

struct Session;
struct MultiplayerBase;
struct BasicAirEnemyParams;
struct Enemy;
struct HUD;
struct Gator;
struct Bird;
struct FightEndSequence;
struct GameSession;

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
		int done; //better than a bool for packing rn
		int health[4];
		int meter[4];
		int endSeqState;
		int endSeqFrame;
	};

	MyData data;

	BasicAirEnemyParams *gatorParams;
	Gator *testGator;
	BasicAirEnemyParams *birdParams;
	Bird *testBird;

	FightEndSequence *endSeq;


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

struct RaceMode : GameMode
{
	struct MyData
	{
		int done;
		int test;
		int endSeqState;
		int endSeqFrame;
		/*int p0Health;
		int p1Health;
		int p0Meter;
		int p1Meter;*/
	};

	MyData data;

	FightEndSequence *endSeq;
	//int maxHealth;
	//int meterSection;
	//int numMeterSections;

	RaceMode();
	~RaceMode();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	HUD *CreateHUD();
	void Setup();
	void StartGame();
	bool CheckVictoryConditions();
	void EndGame();
};

struct ParallelRaceMode : GameMode
{
	struct MyData
	{
		int done;
		int test;
		int endSeqState;
		int endSeqFrame;
		/*int p0Health;
		int p1Health;
		int p0Meter;
		int p1Meter;*/
	};

	MyData data;

	FightEndSequence *endSeq;
	GameSession *parallelGames[3];
	//int maxHealth;
	//int meterSection;
	//int numMeterSections;

	ParallelRaceMode();
	~ParallelRaceMode();
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