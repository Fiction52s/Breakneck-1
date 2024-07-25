#ifndef __WARPTRANSITIONSEQUENCE_H__
#define __WARPTRANSITIONSEQUENCE_H__

#include "Sequence.h"

struct ShapeEmitter;
struct Session;
struct MovingGeoGroup;

struct WarpTransitionSequence : Sequence
{
	enum State
	{
		FADEOUT,
		Count
	};

	WarpTransitionSequence();
	~WarpTransitionSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void Draw(sf::RenderTarget *target,
		int layer = DrawLayer::IN_FRONT);
	void Reset();
	GameSession *bonus;
	//MovingGeoGroup *geoGroup;
	//ShapeEmitter *emitter;
	Session *sess;

	//sf::Vertex overlayRect[4];
};

#endif
