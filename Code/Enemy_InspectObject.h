#ifndef __ENEMY_INSPECT_OBJECT_H__
#define __ENEMY_INSPECT_OBJECT_H__

#include "Enemy.h"

struct InspectSequence;

//you get close to it, press a button, and something pops up
struct InspectObject : Enemy
{
	enum Action
	{
		NEUTRAL,
		READY_TO_SHOW,
		SHOW,
		RECOVERY,
		Count
	};

	struct MyData : StoredEnemyData
	{
	};

	MyData data;
	InspectObject(ActorParams *ap);//sf::Vector2i &pos, int level);
	~InspectObject();
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool ShowTutorial();
	bool IsReadyToShow();
	bool IsShowing();
	void HideTutorial();
	void AddToWorldTrees();
	bool TryActivate();
	void CheckReady();
	bool TryDeactivate();
	sf::FloatRect GetAABB();
	void SetExtraIDsAndAddToVectors();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	
	
	double entranceRadius;
	double exitRadius;
	Tileset *ts;
	InspectSequence *inspectSeq;
};

#endif