#ifndef __SHIELD_H__
#define __SHIELD_H__

struct Tileset;
#include <SFML/Graphics.hpp>
#include "VectorMath.h"
#include "Enemy.h"

struct Shield : HittableObject
{
	enum ShieldType
	{
		T_BLOCK,
		T_REFLECT,
		T_Count
	};

	enum Action
	{
		S_WAIT,
		S_HURT,
		S_BREAK,
		S_REFLECT,
		S_Count
	};

	Shield( ShieldType type, float rad, int maxH,
		Enemy *e, HitboxInfo *hInfo = NULL );
	~Shield();
	void ConfirmKill();
	void ConfirmHitNoKill();
	ShieldType sType;
	Action action;
	int frame;
	HitboxInfo *hitboxInfo;

	Enemy *parent;

	HitboxInfo * IsHit(int pIndex);
	void ProcessHit();
	int pauseFrames;

	bool ProcessState();
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
	void SetPosition(V2d &pos);
	V2d GetPosition();
	void FrameIncrement();

	float radius;
	Tileset *ts_mask;
	sf::Vertex *verts;

	CollisionBody body;
	int maxHits;
	int currHits;
	void Reset();
};

#endif