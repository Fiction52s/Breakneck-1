#ifndef __ENEMY_BLOCKER_H__
#define __ENEMY_BLOCKER_H__

#include "Enemy.h"

struct BlockerChain;

struct Blocker : Enemy, QuadTreeEntrant
{
	enum Action
	{
		WAIT,
		MALFUNCTION,
		FASTDEATH,
		Count
	};

	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsFastDying();
	Action action;
	Blocker(BlockerChain *bc, sf::Vector2i &pos, int index);
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	bool physicsOver;
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void DebugDraw(sf::RenderTarget *target);
	void ClearSprite();
	bool IHitPlayer(int index);
	std::pair<bool, bool> PlayerHitMe(int index);
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void SaveEnemyState();
	void LoadEnemyState();

	int frame;

	CollisionBox hurtBody;
	CollisionBox hitBody;

	int animationFactor;

	BlockerChain *bc;

	bool checkCol;
	//Tileset *ts_testBlood;
	//sf::Sprite bloodSprite;
	//int bloodFrame;
	//bool facingRight;

	int vaIndex;
};

struct BlockerChain : Enemy
{
	enum BlockerType
	{
		NORMAL,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA
	};

	sf::Vertex *va;

	BlockerChain(GameSession *owner,
		sf::Vector2i &pos, std::list<sf::Vector2i> &path,
		int bType, bool armored, int spacing = 0);
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

	int animationFactor;
	int frame;
	int liveFrames;
	Blocker **blockers;
	int numBlockers;

	sf::Vector2<double> origPosition;
	bool dying;
	Tileset *ts;
	//Tileset *ts_swarm;

	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	bool checkCol;

	//const static int NUM_SWARM = 5;
	//sf::VertexArray swarmVA;
	//sf::Sprite nestSprite;
	//SwarmMember *members[NUM_SWARM];

	//CollisionBox hurtBody;
	//CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	//Tileset *ts_swarmExplode;

	//sf::Vector2f spriteSize;
	//double maxSpeed;

	//sf::Vector2<double> deathVector;
	//double deathPartingSpeed;
	//sf::Sprite botDeathSprite;
	//sf::Sprite topDeathSprite;
	Tileset * ts_death;

	//int deathFrame;


	bool armored;
	BlockerType bType;
};

#endif