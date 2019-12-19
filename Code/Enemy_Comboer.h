#ifndef __ENEMY_Comboer_H__
#define __ENEMY_Comboer_H__

#include "Enemy.h"

struct ComboObject;

struct Comboer : Enemy
{
	enum Action
	{
		S_FLOAT,
		S_SHOT,
		S_EXPLODE,
		S_Count
	};

	//enum ComboerType
	//{
	//	T_STRAIGHT,
	//	T_GRAVITY,
	//	T_REVERSEGRAVITY,
	//	T_BOUNCE,
	//	T_Count
	//};

	Comboer(GameSession *owner,
		sf::Vector2i pos, std::list<sf::Vector2i> &path, 
		bool loop, int p_level );
	

	void ProcessState();
	void ProcessHit();
	void UpdateEnemyPhysics();
	void FrameIncrement();
	void ComboHit();
	void EnemyDraw(sf::RenderTarget *target);
	void UpdateSprite();
	void ResetEnemy();
	void HandleNoHealth();
	void AdvanceTargetNode();

	V2d velocity;
	int shootFrames;
	int shootLimit;
	int hitLimit;
	int currHits;

	Action action;
	int actionLength[S_Count];
	int animFactor[S_Count];
	sf::Vector2i *path; //global
	int pathLength;
	bool loop;
	int targetNode;
	bool forward;
	double acceleration;
	double speed;
	int nodeWaitFrames;
	sf::Sprite sprite;
	Tileset *ts;
	bool facingRight;
};

#endif