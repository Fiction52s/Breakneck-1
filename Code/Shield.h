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

	struct MyData
	{
		HittableObject hittableObjectData;

		int currHits;
		int pauseFrames;
		int frame;
		Action action;
	};
	MyData data;

	int shieldID;


	float radius;
	Tileset *ts_mask;
	sf::Vertex *verts;

	CollisionBody body;
	int maxHits;
	

	ShieldType sType;
	
	
	
	HitboxInfo *hitboxInfo;
	Enemy *parent;

	Shield( ShieldType type, float rad, int maxH,
		Enemy *e, HitboxInfo *hInfo = NULL );
	~Shield();
	void ConfirmKill();
	void ConfirmHitNoKill();
	
	HitboxInfo * IsHit(int pIndex);
	void ProcessHit();
	
	void Reset();
	bool ProcessState();
	void UpdateSprite();
	void Draw(sf::RenderTarget *target);
	void SetPosition(V2d &pos);
	V2d GetPosition();
	void FrameIncrement();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
	
};

#endif