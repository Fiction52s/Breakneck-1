#ifndef __LOGSEQUENCE_H__
#define __LOGSEQUENCE_H__

#include "Sequence.h"

struct LogItem;
struct LogPopup;
struct GetLogSequence : Sequence
{
	enum State
	{
		GET,
		Count
	};


	int freezeFrame;
	GetLogSequence();
	~GetLogSequence();
	void UpdateState();
	void ReturnToGame();
	void SetupStates();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	LogItem *log;
	LogPopup *logPop;
	MovingGeoGroup geoGroup;
	ShapeEmitter *emitter;

	sf::Vertex overlayRect[4];
};

#endif