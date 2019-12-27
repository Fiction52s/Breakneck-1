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
		END
	};

	State state;
	int stateLength[END];
	GetShardSequence(GameSession *owner);
	~GetShardSequence();
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	Shard *shard;
	ShardPopup *shardPop;
	MovingGeoGroup geoGroup;
	ShapeEmitter *emitter;
	GameSession *owner;

	sf::Vertex overlayRect[4];
};

#endif