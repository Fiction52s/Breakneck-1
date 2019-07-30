#ifndef __GATE_H__
#define __GATE_H__

#include "Physics.h"
#include "Tileset.h"
#include "ShardTypes.h"

struct GameSession;
struct Zone;

struct Gate : public QuadTreeEntrant//: public Edge
{
	
	enum GateType
	{
		BLACK,
		KEYGATE,
		CRAWLER_UNLOCK,
		SECRET,
		SHARD,
		Count
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
		REFORM,
		LOCKFOREVER,
		OPEN,
		State_Count
	};
	
	bool IsTwoWay();
	bool IsAlwaysUnlocked();
	bool IsReformingType();
	bool CanUnlock();
	V2d GetCenter();
	//bool keyGate;
	//int requiredKeys;
	Gate( GameSession *owner, GateType type );
	int dissolveLength;
	~Gate();
	void Reset();
	GateType type;
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

	sf::Shader gateShader;
	sf::Shader centerShader;

	void HandleQuery( QuadTreeCollider * qtc );
	bool IsTouchingBox( const sf::Rect<double> &r );

	void Draw( sf::RenderTarget *target );
	void CalcAABB();
	void SetShard(int w, int li);

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

	Gate *activeNext;
	//
	sf::Sprite shardSprite;
	sf::Sprite shardBGSprite;
	Tileset *ts_shard;
	int shardWorld;
	int shardIndex;
	ShardType shardType;
	
};

#endif