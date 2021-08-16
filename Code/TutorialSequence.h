#ifndef __TUTORIALSEQUENCE_H__
#define __TUTORIALSEQUENCE_H__

#include "Sequence.h"

struct ShapeEmitter;
struct Session;
struct MovingGeoGroup;
struct TutorialBox;

struct TutorialSequence : Sequence
{
	enum State
	{
		TELL,
		Count
	};

	TutorialSequence();
	~TutorialSequence();
	void UpdateState();
	void SetupStates();
	void ReturnToGame();
	void AddGroups();
	void SetText(const std::string &str);
	void SetBoxPos(sf::Vector2f &pos);
	void Draw(sf::RenderTarget *target,
		EffectLayer layer = EffectLayer::IN_FRONT);
	void Reset();

	Session *sess;

	TutorialBox *tutBox;

	sf::Vertex overlayRect[4];
};

#endif