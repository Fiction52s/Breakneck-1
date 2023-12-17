#ifndef __ENEMY_TUTORIALOBJECT_H__
#define __ENEMY_TUTORIALOBJECT_H__

#include "Enemy.h"

struct TutorialSequence;

struct TutorialObject : Enemy
{
	enum Action
	{
		NEUTRAL,
		SHOW,
		Count
	};

	struct MyData : StoredEnemyData
	{
	};

	MyData data;
	TutorialObject(ActorParams *ap);//sf::Vector2i &pos, int level);
	~TutorialObject();
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool ShowTutorial();
	bool IsTutorialShowable();
	bool IsShowing();
	void HideTutorial();
	void AddToWorldTrees();
	bool TryActivate();
	bool TryDeactivate();
	void UpdateParamsSettings();
	sf::FloatRect GetAABB();
	void SetExtraIDsAndAddToVectors();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	
	
	double entranceRadius;
	double exitRadius;
	Tileset *ts;
	TutorialSequence *tutorialSeq;
};

#endif