#ifndef __ENEMY_FOOTTRAP_H__
#define __ENEMY_FOOTTRAP_H__

#include "Enemy.h"

struct FootTrap : Enemy
{
	FootTrap(GameSession *owner, bool hasMonitor,
		Edge *ground, double quantity);
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
	void DebugDraw(sf::RenderTarget *target);
	void UpdateHitboxes();
	bool ResolvePhysics(sf::Vector2<double> vel);
	void SaveEnemyState();
	void LoadEnemyState();
	void ResetEnemy();
	//void DirectKill();


	sf::Sprite sprite;
	Tileset *ts;

	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	CollisionBox hitBody;
	HitboxInfo *hitboxInfo;

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