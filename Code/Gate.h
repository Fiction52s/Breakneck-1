#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"
#include "Tileset.h"
#include "ShardTypes.h"

struct GameSession;
struct Zone;

struct Gate : public QuadTreeEntrant//: public Edge
{
	
	enum GateCategory
	{
		KEY,
		SHARD,
		BOSS,
		SECRET,
		PICKUP,
		BLACK,
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
		State_Count
	};

	int stateLength[State_Count];
	
	bool IsTwoWay();
	bool IsAlwaysUnlocked();
	bool IsReformingType();
	bool CanUnlock();
	bool IsInUnlockableState();
	
	V2d GetCenter();
	//bool keyGate;
	//int requiredKeys;
	Gate( GameSession *owner, int tcat,
		int var);
	int dissolveLength;
	~Gate();
	void Reset();
	int category;
	int variation;
	GameSession *owner;
	GateState gState;
	bool visible;
	bool locked;
	int frame;
	int flowFrame;
	sf::Color c;
	sf::Vertex testLine[4];
	sf::Vertex centerLine[4];
	sf::VertexArray thickLine;
	sf::VertexArray *gQuads;
	sf::Vertex nodes[8];
	Tileset *ts_node;
	Tileset *ts;
	Tileset *ts_black;
	Tileset *ts_lightning;
	sf::Vertex *blackGate;
	int numBlackQuads;
	void UpdateLine();
	void SetLocked( bool on );
	void Update();
	void SetNodeSprite(bool active);
	void UpdateSprite();
	void UpdateShaders();
	void CheckSoften();

	void TotalDissolve();
	void ReverseDissolve();
	void Soften();
	void Reform();
	void Close();

	bool IsZoneType();

	bool CanSoften();
	void ResetAttachedWires();
	void ActionEnded();

	sf::Shader gateShader;
	sf::Shader centerShader;

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );

	void Draw( sf::RenderTarget *target );
	void CalcAABB();
	void SetShard(int w, int li);
	void SetNumToOpen(int num);

	sf::Rect<double> aabb;

	Edge *temp0prev;
	Edge *temp0next;
	Edge *temp1prev;
	Edge *temp1next;

	Edge *edgeA;
	Edge *edgeB;

	Zone *zoneA;//edgeA is part of zoneA
	Zone *zoneB;

	Gate *next;
	Gate *prev;

	//Gate *activeNext;
	//
	sf::Sprite shardSprite;
	sf::Sprite shardBGSprite;
	Tileset *ts_shard;
	int shardWorld;
	int shardIndex;
	int shardType;

	int numToOpen;

	Tileset *ts_orb;
	sf::Vertex orbQuad[4];
	//sf::CircleShape numberCircle;
	sf::Text numberText;
	int orbFrame;
	enum OrbState
	{
		ORB_RED,
		ORB_GREEN,
		ORB_GO,
	};

	int orbState;
	
};

#endif