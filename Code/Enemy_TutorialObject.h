#ifndef __ENEMY_TUTORIALOBJECT_H__
#define __ENEMY_TUTORIALOBJECT_H__

#include "Enemy.h"

struct TutorialObject : Enemy
{
	enum Action
	{
		NEUTRAL,
		SHOW,
		Count
	};

	TutorialObject(ActorParams *ap);//sf::Vector2i &pos, int level);

	bool CountsForEnemyGate() { return false; }
	void ProcessState();
	void SetLevel(int lev);
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	bool ShowTutorial();
	bool IsTutorialShowable();
	void AddToWorldTrees();
	
	Tileset *ts;
	Tileset *ts_tutorial;
	sf::Sprite tutorialSpr;
};

#endif