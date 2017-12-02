#ifndef __ENEMY_PULSER_H__
#define __ENEMY_PULSER_H__

#include "Enemy.h"

struct Pulser : Enemy
{
	Pulser(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos,
		std::list<sf::Vector2i> &path,
		int framesBetween,
		bool loop);

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

	//void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	int framesBetween;
	MovementSequence testSeq;
	//sf::Vector2<double> basePos;
	int deathFrame;
	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	//Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	//int targetNode;
	//bool forward;
	//sf::Vector2<double>
	int frame;

	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	int hitlagFrames;
	int hitstunFrames;
	int animationFactor;

	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
	bool facingRight;

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