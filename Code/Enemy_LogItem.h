#ifndef __ENEMY_LOG_H__
#define __ENEMY_LOG_H__

#include "Enemy.h"
#include <map>
#include "MovingGeo.h"


struct EffectPool;
struct TilesetManager;
struct Session;
struct ShapeEmitter;

struct LogItem : Enemy
{
	enum Action
	{
		FLOAT,
		DISSIPATE,
		LAUNCH,
		Count
	};


	ShapeEmitter *testEmitter;
	MovingGeoGroup geoGroup;
	bool alreadyCollected;

	Tileset *ts_sparkle;
	EffectPool *sparklePool;
	Tileset *ts_explodeCreate;
	V2d rootPos;
	int radius;

	Tileset *ts;
	Tileset *ts_shine;
	int tile;

	int logType;
	int logWorld;
	int localIndex;
	
	sf::Sprite shineSprite;

	bool caught;
	int totalFrame;

	static int GetLogTypeFromWorldAndIndex(int w, int li);
	static int GetNumLogsTotal();

	LogItem(ActorParams *ap);//sf::Vector2i pos,
							   //int w, int li);
	~LogItem();
	//void UpdateParamsSettings();
	void Setup();
	void UpdateParamsSettings();

	void Launch();
	void DirectKill();

	void ProcessState();
	void EnemyDraw(sf::RenderTarget *target);
	void DrawMinimap(sf::RenderTarget *target);
	void UpdateSprite();
	void IHitPlayer(int index);
	void ProcessHit();
	void ResetEnemy();
	void DissipateOnTouch();
	void Capture();
	void FrameIncrement();
};

#endif