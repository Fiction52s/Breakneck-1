#ifndef __ENEMY_COYOTESHOCKWAVE_H__
#define __ENEMY_COYOTESHOCKWAVE_H__

#include "Enemy.h"
#include <vector>

struct CoyoteShockwave;

struct CoyoteShockwavePool
{
	CoyoteShockwavePool();
	~CoyoteShockwavePool();
	void Reset();
	CoyoteShockwave * Throw(V2d &pos,
		double startRadius,
		double endRadius,
		double waveThickness,
		int expandFrames );
	void Draw(sf::RenderTarget *target);
	std::vector<CoyoteShockwave*> shockwaveVec;
	sf::Vertex *verts;
	Tileset *ts;
	int numShockwaves;
};

struct CoyoteShockwave : Enemy
{
	enum Action
	{
		STARTUP,
		EXPANDING,
		DISSIPATE,
		A_Count
	};

	Tileset *ts;

	int hitlagFrames;
	int hitstunFrames;

	V2d velocity;

	double waveThickness;

	double startRadius;
	double endRadius;

	sf::Vertex *quad;

	double distToTarget;

	CoyoteShockwave(sf::Vertex *quad,
		CoyoteShockwavePool *pool);
	void Die();
	void UpdateEnemyPhysics();
	void Throw(V2d &pos, double startRadius,
		double endRadius,
		double waveThickness,
		int expandFrames);
	void SetLevel(int lev);
	void ProcessState();
	bool CheckHitPlayer(int index = 0);
	void EnemyDraw(sf::RenderTarget *target);
	void HandleHitAndSurvive();
	void IHitPlayer(int index = 0);
	void UpdateSprite();
	void ResetEnemy();
	void FrameIncrement();
};

#endif