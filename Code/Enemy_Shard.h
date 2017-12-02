#ifndef __ENEMY_SHARD_H__
#define __ENEMY_SHARD_H__

#include "Enemy.h"

struct Shard : Enemy
{

	//MovementSequence testSeq;
	Shard(GameSession *owner, sf::Vector2i pos,
		int shardsLoaded);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;

	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int shardIndex;

	int deathFrame;

	int radius;

	//sf::Vector2<double> deathVector;
	//double deathPartingSpeed;
	//sf::Sprite botDeathSprite;
	//sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;

	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	bool caught;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
	//bool facingRight;

	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2<double> deathVector;
		//double deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2<double> position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

#endif