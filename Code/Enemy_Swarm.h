#ifndef __ENEMY_SWARM_H__
#define __ENEMY_SWARM_H__

#include "Enemy.h"

struct Swarm;
struct SwarmMember : Enemy
{
	SwarmMember(Swarm *parent,
		sf::VertexArray &va, int index,
		sf::Vector2<double> &targetOffset,
		double p_maxSpeed);
	void ClearSprite();
	void UpdatePostPhysics();
	void UpdateSprite();
	void UpdatePrePhysics();
	void UpdatePhysics();
	void HandleEntrant(QuadTreeEntrant *qte);
	void PhysicsResponse();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	int framesToLive;

	Swarm *parent;
	double maxSpeed;
	int vaIndex;
	int frame;
	int animFactor;
	sf::Vector2<double> targetOffset;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	sf::Vector2<double> velocity;


	bool active;
	//Edge *ground;

	//bool active;



	sf::VertexArray &va;

	//CollisionBox hurtBody;
	//CollisionBox hitBody;
	//CollisionBox physBody;
	//HitboxInfo *hitboxInfo;

	//double angle;
	//sf::Vector2<double> tempVel;
	//sf::Vector2<double> dir;

	//Tileset *ts;
	//int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
	double bulletSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
};

struct Swarm : Enemy
{
	enum Action
	{
		NEUTRAL,
		FIRE,
		USED,
		REFILL,
		Count
	};

	Swarm(GameSession *owner,
		sf::Vector2i &pos,
		int liveFrames);
	void ActionEnded();
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
	//bool IHitPlayer( int index = 0 );
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void Launch();

	int animationFactor;
	int actionLength[Action::Count];
	int animFactor[Action::Count];
	int frame;
	Action action;
	int liveFrames;

	sf::Vector2<double> origPosition;
	bool dying;
	Tileset *ts;
	Tileset *ts_swarm;

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	const static int NUM_SWARM = 5;
	sf::VertexArray swarmVA;
	sf::Sprite nestSprite;
	SwarmMember *members[NUM_SWARM];

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	Tileset *ts_swarmExplode;

	sf::Vector2f spriteSize;
	double maxSpeed;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;

	int deathFrame;
};

#endif