#ifndef __ENEMY_PULSER_H__
#define __ENEMY_PULSER_H__

#include "Enemy.h"
#include "MovingGeo.h"

struct Pulser : Enemy
{
	enum Action
	{
		WAIT,
		CHARGE,
		ELECTRIFY,
		RECOVER,
		A_COUNT
	};

	int actionLength[A_COUNT];
	int animFactor[A_COUNT];
	Action action;

	Pulser(GameSession *owner, bool hasMonitor,
		sf::Vector2i pos, std::list<sf::Vector2i> &path,
		bool loop,
		int level);
	~Pulser();
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);

	void UpdateSprite();
	void ResetEnemy();
	void UpdateEnemyPhysics();
	void AdvanceTargetNode();

	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int targetNode;
	bool forward;
	double speed;
	int nodeWaitFrames;

	sf::Sprite sprite;
	Tileset *ts;

	Tileset *ts_aura;
	sf::Sprite auraSprite;

	double checkRadius;
	double hitRadius;

	bool facingRight;

	MovingGeoGroup geoGroupWait;
	MovingGeoGroup geoGroupCharge;
	MovingGeoGroup geoGroupExplode;
	//MovingRing *exRing;

	
};

#endif