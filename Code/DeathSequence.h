#ifndef __DEATHSEQUENCE_H__
#define __DEATHSEQUENCE_H__

#include "Sequence.h"

struct ShapeEmitter;
struct Session;
struct MovingGeoGroup;

struct DeathSequence : Sequence
{
	enum State
	{
		DIE,
		Count
	};

	DeathSequence();
	~DeathSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	MovingGeoGroup *geoGroup;
	ShapeEmitter *emitter;
	Session *sess;

	sf::Vertex overlayRect[4];
};

#endif