#ifndef __ENEMY_WIREBALL_H__
#define __ENEMY_WIREBALL_H__

#include "Enemy.h"

struct ComboObject;

struct WireBall : Enemy, SurfaceMoverHandler
{
	enum Action
	{
		S_FLOAT,
		S_FLY,
		S_BOUNCE,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int currHits;
		int juggleTextNumber;
		int currJuggle;
		int waitFrame;
		bool doneBeingHittable;
	};
	MyData data;

	double flySpeed;

	bool limitedJuggles;
	sf::Text numJugglesText;


	int hitLimit;


	Tileset *ts;

	int juggleReps;


	V2d *guidedDir;


	int maxWaitFrames;

	bool CountsForEnemyGate() { return false; }
	WireBall(ActorParams *ap);
	~WireBall();
	void SetLevel(int lev);
	bool CanBeHitByWireTip(bool red);
	void HitTerrainAerial(Edge * edge, double quant);
	void UpdateParamsSettings();
	void ProcessState();
	bool CanBeAnchoredByWire(bool red);
	void HandleWireHit(Wire *w);
	HitboxInfo * IsHit(int pIndex);
	void ProcessHit();
	bool CanComboHit(Enemy *e);
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void ComboKill(Enemy *e);
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void Return();
	void Pop();
	void PopThrow();
	V2d GetBounceVel(V2d &normal);

	void Throw(double a, double strength);
	void Throw(V2d vel);
	void UpdateJuggleRepsText(int reps);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif