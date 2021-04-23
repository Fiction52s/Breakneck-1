#ifndef __ENEMY_POWERITEM_H__
#define __ENEMY_POWERITEM_H__

#include "Enemy.h"
#include <map>
#include "ShardTypes.h"

#include "MovingGeo.h"


struct EffectPool;
struct TilesetManager;
struct Session;


struct PowerPopup
{
	enum State
	{
		SHOW,
		Count
	};


	PowerPopup();
	void SetTopLeft(sf::Vector2f &pos);
	void SetCenter(sf::Vector2f &pos);
	void Reset();
	void Update();
	void SetPower(int index);
	void SetDescription(const std::string &desc);
	void Draw(sf::RenderTarget *target);
	sf::Text desc;
	sf::Sprite powerSpr;
	sf::Sprite effectSpr;
	sf::Sprite bgSpr;

	sf::Vector2f topLeft;
	sf::Vector2f descRel;
	sf::Vector2f effectRel;
	sf::Vector2f powerRel;

	Session *sess;
	int powerIndex;
	int frame;
	State state;
};


struct ShapeEmitter;
struct GetPowerSequence;
struct PowerItem : Enemy
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

	int powerIndex;

	bool caught;
	int totalFrame;
	GetPowerSequence *powerSeq;

	PowerItem(ActorParams *ap);//sf::Vector2i pos,
						   //int w, int li);
	~PowerItem();
	//void UpdateParamsSettings();
	void Setup();

	
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