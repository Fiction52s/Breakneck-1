#ifndef __ENEMY_INSPECT_OBJECT_H__
#define __ENEMY_INSPECT_OBJECT_H__

#include "Enemy.h"

struct InspectSequence;

//you get close to it, press a button, and something pops up
struct InspectObject : Enemy
{
	enum Action
	{
		A_NEUTRAL,
		A_SHOW_ICON,
		A_SHOW_INSPECTABLE,
		A_RECOVERY,
		Count
	};

	struct MyData : StoredEnemyData
	{
	};



	MyData data;

	Tileset *ts_inspect;

	int currControllerType;
	sf::Vertex buttonQuad[4];

	InspectObject(ActorParams *ap);//sf::Vector2i &pos, int level);
	~InspectObject();
	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	
	void ShowInspectable();
	void HideInspectable();
	void ShowIcon();
	void HideIcon();
	bool IsShowingInspectable();
	bool IsShowingIcon();
	void AddToWorldTrees();
	bool TryActivate();
	bool TryDeactivate();
	bool IsUsed();
	sf::FloatRect GetAABB();
	void UpdateButtonIconsWhenControllerIsChanged();
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