#ifndef __ENEMY_H__
#define __ENEMY_H__

#include "Actor.h"
#include <list>



struct Enemy : QuadTreeCollider, QuadTreeEntrant
{
	enum EnemyType
	{
		BASICEFFECT,
		PATROLLER,
		CRAWLER,
		BASICTURRET,
		FOOTTRAP,
		GOAL,
		Count
	};

	Enemy( GameSession *owner, EnemyType t );
	//virtual void HandleEdge( Edge *e ) = 0;
	virtual void HandleEntrant( QuadTreeEntrant *qte ) = 0;
	virtual void UpdatePrePhysics() = 0;
	virtual void UpdatePhysics() = 0;
	virtual void UpdatePostPhysics() = 0;
	virtual void Draw( sf::RenderTarget *target) = 0;
	virtual bool IHitPlayer() = 0;
	virtual void UpdateHitboxes() = 0;
	virtual bool PlayerHitMe() = 0;
	virtual bool PlayerSlowingMe() = 0;
	virtual void DebugDraw(sf::RenderTarget *target) = 0;
	void SaveState();
	void LoadState();
	virtual void SaveEnemyState() = 0;
	virtual void LoadEnemyState() = 0;
	void Reset();
	virtual void ResetEnemy() = 0;
	Enemy *prev;
	Enemy *next;
	GameSession *owner;
	bool spawned;
	sf::Rect<float> spawnRect;
	HitboxInfo *receivedHit;
	int slowMultiple;
	int slowCounter;
	EnemyType type;
	bool spawnedByClone;

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<float> &r );

	struct Stored
	{
		Enemy *prev;
		Enemy *next;
		bool spawned;		
		HitboxInfo *receivedHit;
		int slowMultiple;
		int slowCounter;
	};
	Stored stored;
};

struct BasicEffect : Enemy
{
	BasicEffect ( GameSession *owner );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2f vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	void Init( Tileset *ts, 
		sf::Vector2f position, 
		float angle, int frameCount,
		int animationFactor, bool right );
	int frameCount;
	sf::Sprite s;
	Tileset *ts;
	int frame;
	bool pauseImmune;
	bool activated;
	int animationFactor;
	bool facingRight;

	int stored_frame;
	//sf::Vector2f position;
};

struct Patroller : Enemy
{
	Patroller( GameSession *owner, sf::Vector2i pos, std::list<sf::Vector2i> &path, bool loop, float speed );
	//void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	void DebugDraw(sf::RenderTarget *target);
	bool IHitPlayer();
	bool PlayerHitMe();
	void UpdateSprite();
	void UpdateHitboxes();
	bool PlayerSlowingMe();
	void ResetEnemy();

	void AdvanceTargetNode();

	void SaveEnemyState();
	void LoadEnemyState();

	bool dead;
	int deathFrame;
	sf::Vector2f deathVector;
	float deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	//std::list<sf::Vector2i> path;
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;

	int targetNode;
	bool forward;
	//sf::Vector2f
	int frame;

	float acceleration;
	float speed;
	int nodeWaitFrames;
	sf::Vector2f position;
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


	struct Stored
	{
		bool dead;
		int deathFrame;
		//sf::Vector2f deathVector;
		//float deathPartingSpeed;
		int targetNode;
		bool forward;
		int frame;
		sf::Vector2f position;

		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

struct Crawler : Enemy
{
	Crawler( GameSession *owner, Edge *ground, float quantity, bool clockwise, float speed );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2f vel );
	void ResetEnemy();
	
	void SaveEnemyState();
	void LoadEnemyState();
	void UpdatePhysics2();
	void UpdatePhysics3();
	sf::Sprite sprite;
	Tileset *ts_walk;
	Tileset *ts_roll;

	bool clockwise;
	float groundSpeed;
	Edge *ground;
	//sf::Vector2f offset;
	float edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	CollisionBox physBody;
	sf::Vector2f position;
	sf::Vector2f tempVel;
	

	float rollFactor;
	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	Edge *startGround;
	float startQuant;
	sf::Vector2f offset;
	int frame;
	bool roll;
	bool dead;
	int deathFrame;
	int crawlAnimationFactor;
	int rollAnimationFactor;
	sf::Vector2f deathVector;
	float deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;
};

struct BasicTurret : Enemy
{
	BasicTurret( GameSession *owner, Edge *ground, float quantity, 
		float bulletSpeed,
		int framesWait );
//	void HandleEdge( Edge *e );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();

	sf::Sprite sprite;
	Tileset *ts;
	
	const static int maxBullets = 16;
	sf::Vector2f bulletPositions[maxBullets];
	sf::Vector2f tempVel;
	



	sf::VertexArray bulletVA;
	CollisionBox bulletHurtBody[maxBullets];
	CollisionBox bulletHitBody[maxBullets];
	struct Bullet
	{
		Bullet();
		Bullet *prev;
		Bullet *next;
		sf::Vector2f position;
		CollisionBox hurtBody;
		CollisionBox hitBody;
		CollisionBox physBody;
		int frame;
		int slowCounter;
		int slowMultiple;
	};
	Bullet *queryBullet;
	bool ResolvePhysics( Bullet *b, sf::Vector2f vel );

	void AddBullet();
	void DeactivateBullet( Bullet *bullet );
	Bullet * ActivateBullet();
	Tileset * ts_bullet;

	Bullet *activeBullets;
	Bullet *inactiveBullets;
	HitboxInfo *bulletHitboxInfo;

	int framesWait;
	int firingCounter;
	Edge *ground;
	float edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	
	sf::Vector2f position;
	float angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2f gn;
	float bulletSpeed;
};

struct FootTrap : Enemy
{
	FootTrap( GameSession *owner, Edge *ground, float quantity );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2f vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	

	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	float edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	sf::Vector2f position;
	float angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2f gn;

	sf::Vector2f deathVector;
	float deathPartingSpeed;
	sf::Sprite botDeathSprite;
	sf::Sprite topDeathSprite;
	Tileset * ts_death;
	Tileset *ts_testBlood;
	sf::Sprite bloodSprite;
	int bloodFrame;

	struct Stored
	{
		bool dead;
		int deathFrame;
		int frame;
		int hitlagFrames;
		int hitstunFrames;
	};
	Stored stored;
};

struct Goal : Enemy
{
	Goal( GameSession *owner, Edge *ground, float quantity );
	void HandleEntrant( QuadTreeEntrant *qte );
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target );
	bool IHitPlayer();
	bool PlayerHitMe();
	bool PlayerSlowingMe();
	void UpdateSprite();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics( sf::Vector2f vel );
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	

	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	float edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
	
	sf::Vector2f position;
	float angle;

	//Contact minContact;
	//bool col;
	//std::string queryMode;
	//int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2f gn;
};

struct EnemyParentNode;



struct EnemyQNode
{
	EnemyQNode():parent(NULL),debug(NULL){}
	sf::Vector2f pos;
	float rw;
	float rh;
	sf::RenderWindow *debug;
	EnemyParentNode *parent;
	bool leaf;
};



struct EnemyParentNode : EnemyQNode
{
	EnemyParentNode( const sf::Vector2f &pos, float rw, float rh );
	EnemyQNode *children[4];
	// 0    |     1
	//--------------
	// 2    |     3
	
};

struct EnemyLeafNode : EnemyQNode
{
	int objCount;
	EnemyLeafNode( const sf::Vector2f &pos, float rw, float rh );
	Enemy *enemies[4];
};

EnemyQNode *Insert( EnemyQNode *node, Enemy* e );
//void Query( EnemyQNode *node, void (*f)( Enemy *e ) );

void DebugDrawQuadTree( sf::RenderWindow *rw, EnemyQNode *node );

struct EnemyQuadTreeCollider
{
	virtual void HandleEnemy( Enemy *e ) = 0;
};

void Query( EnemyQuadTreeCollider *qtc, EnemyQNode *node, const sf::Rect<float> &r );

bool IsEnemyTouchingBox( Enemy *e, const sf::Rect<float> & ir );




#endif