#ifndef __SHARDSEQUENCE_H__
#define __SHARDSEQUENCE_H__

#include "Sequence.h"

struct Shard;
struct ShardPopup;
struct GetShardSequence : Sequence
{
	enum State
	{
		GET,
		Count
	};

	Shard *shard;
	ShardPopup *shardPop;
	MovingGeoGroup geoGroup;
	ShapeEmitter *emitter;

	sf::Vertex overlayRect[4];
	int freezeFrame;

	GetShardSequence();
	~GetShardSequence();
	void Reset();
	void UpdateState();
	void ReturnToGame();
	void SetupStates();
	void SetIDs();
	void LayeredDraw(int layer, sf::RenderTarget *target);
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif