#ifndef __ENEMY_TOUCHKEY_H__
#define __ENEMY_TOUCHKEY_H__

#include "Enemy.h"
#include "EnemyChain.h"

struct TouchKey;

struct TouchKeyChain : EnemyChain
{
	TouchKeyChain(ActorParams *ap);
	//void InitReadParams(ActorParams *params);
	void ReadParams(ActorParams *params);
	Tileset *GetTileset(int variation);
	Enemy *CreateEnemy(V2d &pos, int ind);
	void UpdateStartPosition(int ind, V2d &pos);
	void EnemyDraw(sf::RenderTarget *target);
	void CreateCustomResources();
	void DeleteCustomResources();

	Tileset *ts_key;
	sf::Vertex *keyQuads;
	sf::Vertex *keyFXQuads;
};

struct TouchKey : Enemy, ChainableObject
{
	enum Action
	{
		NEUTRAL,
		DEATH,
		KEY_NEUTRAL,
		Count
	};

	struct MyData : StoredEnemyData
	{

	};
	MyData data;

	TouchKeyChain *chain;

	int index;
	V2d preTransformPos;

	Tileset *ts;
	sf::Vertex *quad;

	sf::Vertex *keyQuad;
	int keyFrame;

	sf::Vertex *keyFXQuad;

	TouchKey(TouchKeyChain *fc, int index,
		V2d &pos, int level, sf::Vertex *p_quad, Tileset *p_ts, sf::Vertex *p_keyQuad, sf::Vertex *p_keyFXQuad);
	TouchKey(TouchKey &hf);
	//void HandleQuery(QuadTreeCollider * qtc);
	void SetLevel(int lev);
	void AddToWorldTrees();
	void ProcessHit();
	bool IsCollectable();
	//bool IsTouchingBox(const sf::Rect<double> &r);
	sf::FloatRect GetAABB();
	void SetStartPosition(V2d &pos);
	void IHitPlayer(int index);
	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void ClearSprite();
	void ResetEnemy();
	bool IsCollectible(); //depending on type of fly
	bool Collect( Actor *p );
	void FrameIncrement();

	void UpdateKeySprite();

	int GetHealAmount();
	int GetCounterAmount();

	void HandleQuery(QuadTreeCollider * qtc);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif