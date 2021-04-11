#ifndef __POWERSEQUENCE_H__
#define __POWERSEQUENCE_H__

#include "Sequence.h"

struct PowerItem;
struct PowerPopup;
struct GetPowerSequence : Sequence
{
	enum State
	{
		GET,
		Count
	};

	GetPowerSequence();
	~GetPowerSequence();
	void UpdateState();
	void ReturnToGame();
	void SetupStates();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	PowerItem *powerItem;
	PowerPopup *powerPop;
	MovingGeoGroup geoGroup;
	ShapeEmitter *emitter;

	sf::Vertex overlayRect[4];
};

#endif