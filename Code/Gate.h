#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"
#include "Tileset.h"
#include "ISelectable.h"

struct Session;
struct Zone;

struct Gate : public QuadTreeEntrant
{
	enum GateCategory
	{
		ALLKEY,
		NUMBER_KEY,
		ENEMY,
		TIME_GLOBAL,
		TIME_ROOM,
		SHARD,
		BOSS,
		SECRET,
		PICKUP,
		BLACK,
		POWER,
		Count
	};

	enum BossVariation
	{
		CRAWLER,
		BIRD,
	};

	enum PickupVariation
	{
		FLY,
		FLY2,
	};

	enum GateState
	{
		HARD,
		SOFTEN,
		SOFT,
		DISSOLVE,
		//for gates that need to unlock 
		//and dissolve at the same time
		TOTALDISSOLVE, 
		REVERSEDISSOLVE,
		REFORM,
		LOCKFOREVER,
		OPEN,
		GLITCH,
		State_Count
	};

	enum OrbState
	{
		ORB_RED,
		ORB_GREEN,
		ORB_GO,
	};

	struct MyData
	{
		GateState gState;
		int frame;
		bool locked; //when loading state, make sure to set this using SetLocked
		int orbState;
		int orbFrame;
		int flowFrame;
		bool secretTimeGateIsOpened;
		bool timeLocked;
	};

	MyData data;

	Session *sess;
	int gateIndex;
	int seconds;
	int category;
	int variation;
	int numToOpen;
	bool visible; //I don't think this variable is even used.
	bool timeGateIsSecret;
	
	
	sf::Color mapLineColor;
	sf::Vertex hardLine[4];
	sf::Vertex centerLine[4];
	sf::Vertex mapLine[4];
	sf::Vertex nodes[8];

	Tileset *ts_node;
	Tileset *ts;
	Tileset *ts_lockedAndHardened;
	Tileset *ts_glitch;
	Tileset *ts_wiggle;
	sf::Vertex *gateQuads;
	int numGateQuads;

	sf::Shader gateShader;
	sf::Shader centerShader;

	sf::Rect<double> aabb;

	int stateLength[State_Count];

	Edge *temp0prev;
	Edge *temp0next;
	Edge *temp1prev;
	Edge *temp1next;

	Edge *edgeA;
	Edge *edgeB;

	Zone *zoneA;//edgeA is part of zoneA
	Zone *zoneB;

	//temporary for queries
	Gate *next;
	Gate *prev;

	//key or pickup
	
	Tileset *ts_orb;
	sf::Vertex orbQuad[4];
	sf::Text numberText;

	//just for shard stuff
	sf::Sprite shardSprite;
	sf::Sprite shardBGSprite;
	Tileset *ts_shard;
	int shardWorld;
	int shardIndex;
	int shardType;
	sf::Vertex secretLine[2];

	Gate(Session *sess, int tcat,
		int var);
	~Gate();
	void Setup(GateInfoPtr gi);
	void PassThrough(double alongAmount);
	bool IsTwoWay();
	bool IsAlwaysUnlocked();
	bool IsReformingType();
	bool CanUnlock();
	bool IsInUnlockableState();
	V2d GetCenter();
	void Reset();
	void Init();
	void SetLocked( bool on );
	void Update();
	void SetNodeSprite(bool active);
	void UpdateSprite();
	void UpdateShaders();
	void CheckSoften();
	void CheckTimeLock();
	void TotalDissolve();
	void ReverseDissolve();
	void Soften();
	void Reform();
	void Close();
	bool IsZoneType();
	bool CanSoften();
	void ResetAttachedWires();
	void ActionEnded();
	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );
	void Draw( sf::RenderTarget *target );
	void CalcAABB();
	void SetShard(int w, int li);
	void SetNumToOpen(int num);
	void SetTime(int sec);
	void UpdateOrb();
	void SetMapLineColor();
	void MapDraw(sf::RenderTarget *target);
	void MiniDraw(sf::RenderTarget *target);
	void OpenSecretTimeGate();
	void SetToTwoWay();
	bool IsSoft();
	bool IsLocked();
	bool IsLockedForever();
	void Open();
	bool IsReformable();
	bool CanBeHitByWire();
	bool IsSecret();
	void UpdateSecretLine();
	void DrawSecret(sf::RenderTarget *target);

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif