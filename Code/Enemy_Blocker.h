#ifndef __ENEMY_BLOCKER_H__
#define __ENEMY_BLOCKER_H__

#include "Enemy.h"
#include "EnemyChain.h"

struct CircleGroup;

struct BlockerChain;


struct BlockerChain : EnemyChain
{
	/*struct MyData : StoredEnemyData
	{

	};
	MyData data;*/

	sf::Shader blockerShader;
	sf::Glsl::Vec4 paletteArray[16];
	int blockerType;

	BlockerChain(ActorParams *ap);
	//void InitReadParams(ActorParams *params);
	void ReadParams(ActorParams *params);
	Tileset *GetTileset(int variation);
	Enemy *CreateEnemy(V2d &pos, int ind);
	void UpdateStartPosition(int ind, V2d &pos);
	void EnemyDraw(sf::RenderTarget *target);

	/*int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);*/
};


struct Blocker : Enemy, QuadTreeEntrant
{
	enum BlockerType : int
	{
		GREY,
		BLUE,
		GREEN,
		YELLOW,
		ORANGE,
		RED,
		MAGENTA,
		BLACK,
	};

	enum Action
	{
		WAIT,
		MALFUNCTION,
		HITTING,
		EXPLODE,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	BlockerChain *bc;
	int vaIndex;
	int minimapCirclePoints;
	int minimapCircleRadius;
	int randomStartFrame;

	Blocker( BlockerChain *bc, V2d &pos, int index);
	bool CountsForEnemyGate() { return false; }
	void HandleQuery(QuadTreeCollider * qtc);
	bool IsTouchingBox(const sf::Rect<double> &r);
	bool IsFastDying();
	void ProcessState();
	//void DrawMinimap(sf::RenderTarget *target);
	void ClearSprite();
	void UpdateSprite();
	void ResetEnemy();
	void ProcessHit();
	void IHitPlayer(int index);
	void SetStartPosition(V2d &pos);
	sf::FloatRect GetAABB();
	void UpdateHitboxes();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif