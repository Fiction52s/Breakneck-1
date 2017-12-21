#ifndef __ENEMY_RACEFIGHTTARGET_H__
#define __ENEMY_RACEFIGHTTARGET_H__

#include "Enemy.h"

struct RaceFightTarget : Enemy
{
	enum Action
	{
		NEUTRAL,
		PLAYER1,
		PLAYER2,
		PLAYER3,
		PLAYER4,
		Count
	};

	RaceFightTarget *pPrev;
	RaceFightTarget *pNext;
	RaceFightTarget *p2Prev;
	RaceFightTarget *p2Next;

	int gameTimeP1Hit;
	int gameTimeP2Hit;

	Action action;
	//MovementSequence testSeq;
	RaceFightTarget(GameSession *owner,
		sf::Vector2i &pos);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	//CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;
};

#endif