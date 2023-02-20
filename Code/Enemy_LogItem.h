#ifndef __ENEMY_LOG_H__
#define __ENEMY_LOG_H__

#include "Enemy.h"
#include <map>
#include "MovingGeo.h"


struct EffectPool;
struct TilesetManager;
struct Session;
struct ShapeEmitter;
struct TutorialBox;
struct GetLogSequence;
struct LogPreview;

struct LogItem : Enemy
{
	enum Action
	{
		FLOAT,
		DISSIPATE,
		LAUNCH,
		Count
	};

	LogPreview *logPreview;
	ShapeEmitter *testEmitter;
	//MovingGeoGroup geoGroup;
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

	GetLogSequence *logSeq;

	static int GetLogTypeFromWorldAndIndex(int w, int li);
	static int GetNumLogsTotal();

	LogItem(ActorParams *ap);//sf::Vector2i pos,
							   //int w, int li);
	~LogItem();
	bool CountsForEnemyGate() { return false; }
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

struct LogPopup
{
	enum State
	{
		SHOW,
		Count
	};

	TutorialBox *tutBox;
	sf::Sprite logSpr;

	float borderHeight;
	float logBorder;
	float width;
	float height;
	float logSize;
	float nameHeight;

	Tileset *ts_log;

	float previewBGWidth;
	float previewBGHeight;

	sf::Vertex previewBGQuad[4];

	sf::Vertex bgQuad[4];
	sf::Vertex topBorderQuad[4];

	sf::Text nameText;

	sf::Vector2f topLeft;
	sf::Vector2f descBorder;
	sf::Vector2f logRel;

	Session *sess;
	int logWorld;
	int logLocalIndex;
	int frame;
	State state;

	LogPreview *logPreview;

	LogPopup();
	~LogPopup();
	void SetTopLeft(sf::Vector2f &pos);
	void SetCenter(sf::Vector2f &pos);
	void Reset();
	void Update();
	void SetLog(int w, int i );
	void SetDescription(const std::string &desc);
	void Draw(sf::RenderTarget *target);
};

#endif