#ifndef __ENEMY_REMOTECOMBOER_H__
#define __ENEMY_REMOTECOMBOER_H__

#include "Enemy.h"

struct ComboObject;

struct RemoteComboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_RETURN,
		S_Count
	};

	struct MyData : StoredEnemyData
	{
		int currHits;
		int shootFrames;
		V2d velocity;
	};
	MyData data;

	
	
	int shootLimit;
	int hitLimit;
	

	double acceleration;
	double speed;

	Tileset *ts;

	Actor *controlPlayer;

	void SetLevel(int lev);
	bool CountsForEnemyGate() { return false; }
	RemoteComboer(ActorParams * ap);
	~RemoteComboer();

	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif