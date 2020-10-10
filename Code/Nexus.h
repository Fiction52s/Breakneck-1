#ifndef __NEXUS_H__
#define __NEXUS_H__

#include "Enemy.h"

struct NexusCore1Seq;
struct Nexus : Enemy
{
	enum Action
	{
		A_SITTING,
		A_KINKILLING,
		A_EXPLODING,
		A_DESTROYED,
		A_NEXUSDESTROYED,
		A_Count
	};

	Nexus(ActorParams *ap);
	~Nexus();
	void Setup();
	void DrawMinimap(sf::RenderTarget *target);
	void EnemyDraw(sf::RenderTarget *target);
	void FinishDestruction();
	void StartInsideSeq();

	Tileset *ts_node1;
	Tileset *ts_node2;

	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void ProcessState();
	void ConfirmKill();
	V2d GetKillPos();
	int explosionLength;

	NexusCore1Seq *insideSeq;

	int explosionAnimFactor;
	int explosionYOffset;
	int initialYOffset;
	sf::Sprite miniSprite;
	sf::Sprite nodeSprite;
	//Tileset *ts;
	Tileset *ts_nexusOpen;
	Tileset *ts_nexusDestroyed;
	Tileset *ts_mini;
	Tileset *ts_explosion;
	Tileset *ts_explosion1;
	float goalKillStartZoom;
	sf::Vector2f goalKillStartPos;

	int deathFrame;
	int animationFactor;
	sf::Vector2<double> gn;
};


#endif