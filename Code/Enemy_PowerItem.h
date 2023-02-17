#ifndef __ENEMY_POWERITEM_H__
#define __ENEMY_POWERITEM_H__

#include "Enemy.h"
#include <map>
#include "MovingGeo.h"


struct EffectPool;
struct TilesetManager;
struct Session;
struct TutorialBox;


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
	void SetName(const std::string &name);

	TutorialBox *tutBox;
	sf::Sprite powerSpr;

	float borderHeight;
	float powerBorder;
	float width;
	float height;
	float powerSize;
	float nameHeight;

	sf::Vertex bgQuad[4];
	sf::Vertex topBorderQuad[4];
	sf::Vertex powerBorderQuad[4];

	sf::Text nameText;

	sf::Vector2f topLeft;
	sf::Vector2f descBorder;
	sf::Vector2f powerRel;

	Tileset *ts_powers;

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

	bool CountsForEnemyGate() { return false; }
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