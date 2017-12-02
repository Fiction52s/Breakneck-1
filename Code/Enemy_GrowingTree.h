#ifndef __ENEMY_GROWINGTREE_H__
#define __ENEMY_GROWINGTREE_H__

#include "Enemy.h"

struct GrowingTree : Enemy, LauncherEnemy
{
	enum Action
	{
		RECOVER0,
		RECOVER1,
		RECOVER2,
		LEVEL0,
		LEVEL0TO1,
		LEVEL1,
		LEVEL1TO2,
		LEVEL2,
		EXPLODE,
		Count
	};
	Action action;

	GrowingTree(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity,
		int numBullets, int startLevel,
		int pulseRadius);

	void ActionEnded();
	void HandleEntrant(QuadTreeEntrant *qte);
	void UpdatePrePhysics();
	void UpdatePhysics();
	void UpdatePostPhysics();
	void Draw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	bool IHitPlayer(int index = 0);
	std::pair<bool, bool> PlayerHitMe(int index = 0);
	bool PlayerSlowingMe();
	void UpdateSprite();
	void Fire();
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics(sf::Vector2<double> vel);
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	void BulletHitTerrain(BasicBullet *b,
		Edge *edge,
		sf::Vector2<double> &pos);
	void DirectKill();
	void InitRangeMarkerVA();
	void BulletHitPlayer(BasicBullet *b);
	//void DirectKill();
	double pulseRadius;
	int pulseFrame;
	Tileset *ts_bulletExplode;
	int powerLevel;
	int totalBullets;
	Launcher *launcher;
	sf::Sprite sprite;
	Tileset *ts;
	int startPowerLevel;
	Edge *ground;
	sf::VertexArray rangeMarkerVA;
	double edgeQuantity;

	int actionLength[Action::Count];
	int animFactor[Action::Count];

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

	bool dying;
	double angle;

	Contact minContact;
	bool col;
	std::string queryMode;
	int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;

	sf::Vector2<double> deathVector;
	double deathPartingSpeed;
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

#endif