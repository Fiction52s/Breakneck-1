#ifndef __MEDAL_SEQUENCE_H__
#define __MEDAL_SEQUENCE_H__

#include "Sequence.h"

struct Medal;
struct ShardPopup;

struct MedalSequence : Sequence
{
	enum State
	{
		SHOW_MEDAL,
		END,
		Count
	};

	sf::Vertex overlayRect[4];
	Medal *shownMedal;

	MedalSequence();
	~MedalSequence();

	void StartGold();
	void StartSilver();
	void StartBronze();
	void StartRunning();
	void SetupStates();
	void ReturnToGame();
	void AddShots();
	void AddFlashes();
	void AddEnemies();
	void UpdateState();
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
};

#endif