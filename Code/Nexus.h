#ifndef __NEXUS_H__
#define __NEXUS_H__

#include "Enemy.h"


struct Sequence;
struct Nexus : Enemy
{
	enum Action
	{
		A_SITTING,
		A_KINKILLING,
		A_EXPLODING,
		A_DESTROYED,
		A_NEXUSDESTROYED,
	};

	Nexus(GameSession *owner, Edge *ground, double quantity);
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void FinishDestruction();

	Tileset *ts_node1;
	Tileset *ts_node2;

	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ProcessState();
	void ConfirmKill();
	V2d GetKillPos();
	int explosionLength;

	Sequence *insideSeq;

	int explosionAnimFactor;
	int explosionYOffset;
	int initialYOffset;
	Action action;
	sf::Sprite sprite;
	sf::Sprite miniSprite;
	sf::Sprite nexSprite;
	//Tileset *ts;
	Tileset *ts_nexusOpen;
	Tileset *ts_nexusDestroyed;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	Tileset *ts_explosion1;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	Edge *ground;
	double edgeQuantity;

	double angle;

	//int frame;
	int deathFrame;
	int animationFactor;
	bool dead;
	sf::Vector2<double> gn;
};


#endif