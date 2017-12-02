#ifndef __ENEMY_SECURITYWEB_H__
#define __ENEMY_SECURITYWEB_H__

#include "Enemy.h"

struct SecurityWeb : Enemy, RayCastHandler
{
	SecurityWeb(GameSession *owner,
		bool hasMonitor,
		sf::Vector2i &pos, int numProtrusions,
		float angleOffset,
		double bulletSpeed
	);
	virtual ~SecurityWeb();
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void PhysicsResponse();
	void UpdatePostPhysics();
	void ActionEnded();
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
	sf::VertexArray *edges;
	sf::VertexArray *nodes;
	void HandleRayCollision(Edge *edge,
		double edgeQuantity, double rayPortion);
	void DirectKill();
	void ResetNodes();

	sf::Vector2<double> *origins;
	Tileset *ts_bulletExplode;
	sf::VertexArray *armVA;
	int *armLength;


	struct NodeProjectile : Movable
	{

		NodeProjectile(SecurityWeb *parent,
			int vaIndex);
		void Reset(
			sf::Vector2<double> &pos);
		void UpdatePrePhysics();
		void SetNode(int subIndex);
		void UpdatePostPhysics();
		void HitPlayer();
		void IncrementFrame();
		bool PlayerSlowingMe();
		//bool activated;
		int vaIndex;
		//void Fire( sf::Vector2<double> vel );
		//void Draw( sf::RenderTarget *target );
		//Tileset *ts;
		int frame;
		int framesToLive;
		bool active;
		NodeProjectile *nextProj;
		double startAngle;
		double moveAngle;

		//NodeProjectile *revNode;
		SecurityWeb *parent;
	};

	bool dynamicMode;
	int dynamicFrame;
	CollisionBox *edgeHitboxes;
	NodeProjectile *activeNodes;
	NodeProjectile **allNodes;

	//NodeProjectile *centerNode;

	//CollisionBox *centerHitbox;


	double bulletSpeed;
	float angleOffset;
	int numProtrusions;
	int maxProtLength;

	int deathFrame;

	Edge *rcEdge;
	double rcQuantity;
	sf::Vector2<double> rayStart;
	sf::Vector2<double> rayEnd;

	int targetFrames;

	bool dead;
	bool dying;
	int frame;

	int slowCounter;
	int slowMultiple;

	int nodeRadius;

	sf::Sprite sprite;
	Tileset *ts;
	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;
};

#endif