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

	MovingGeoGroup *geoGroup;
	ShapeEmitter *emitter;
	Session *sess;
	sf::Vertex overlayRect[4];

	DeathSequence();
	~DeathSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void Draw(sf::RenderTarget *target,
		int layer = DrawLayer::IN_FRONT);
	void Reset();
	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif