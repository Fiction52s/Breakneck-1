#ifndef __DEATHSEQUENCE_H__
#define __DEATHSEQUENCE_H__

#include "Sequence.h"

struct ShapeEmitter;
struct GameSession;
struct MovingGeoGroup;

struct DeathSequence : Sequence
{
	enum State
	{
		GET,
		END
	};

	DeathSequence(GameSession *owner);
	~DeathSequence();
	bool Update();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	State state;
	int stateLength[END];
	MovingGeoGroup *geoGroup;
	ShapeEmitter *emitter;
	GameSession *owner;

	sf::Vertex overlayRect[4];
};

#endif