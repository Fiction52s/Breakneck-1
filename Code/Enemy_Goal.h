#ifndef __ENEMY_GOAL_H__
#define __ENEMY_GOAL_H__

#include "Enemy.h"

struct Goal : Enemy
{
	Goal(GameSession *owner, Edge *ground, double quantity);
	void HandleEntrant(QuadTreeEntrant *qte);
	//void UpdatePrePhysics();
	void UpdatePhysics();
	//void UpdatePostPhysics();
	void DrawMinimap(sf::RenderTarget *target);
	void Draw(sf::RenderTarget *target);
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
	void HandleNoHealth();
	void ProcessState();

	bool exploding;
	bool kinKilling;
	bool destroyed;
	//int kinKillFrame;
	sf::Sprite sprite;
	sf::Sprite miniSprite;
	Tileset *ts;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	Edge *ground;
	double edgeQuantity;

	CollisionBox hurtBody;
	//CollisionBox hitBody;
	//HitboxInfo *hitboxInfo;

	double angle;

	//Contact minContact;
	//bool col;
	//std::string queryMode;
	//int possibleEdgeCount;

	int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2<double> gn;
};


#endif