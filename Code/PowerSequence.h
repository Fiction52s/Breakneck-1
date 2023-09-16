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

	PowerItem *powerItem;
	PowerPopup *powerPop;
	MovingGeoGroup geoGroup;
	ShapeEmitter *emitter;
	int freezeFrame;
	sf::Vertex overlayRect[4];

	GetPowerSequence();
	~GetPowerSequence();
	void UpdateState();
	void ReturnToGame();
	void SetupStates();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();
	void SetIDs();

	int GetNumStoredBytes();
	void StoreBytes(unsigned char *bytes);
	void SetFromBytes(unsigned char *bytes);
};

#endif