#ifndef __EDITORPLAYERTRACKER_H__
#define __EDITORPLAYERTRACKER_H__

#include <sfml/Graphics.hpp>
#include "VectorMath.h"

struct CircleGroup;

struct PlayerTracker
{
	PlayerTracker();
	~PlayerTracker();
	CircleGroup *testPlayerTracker;
	int trackPoints;
	int mostRecentTrackPoint;
	sf::Color startTrackColor;
	sf::Color endTrackColor;
	V2d playerOldTrackPos;
	V2d playerOldGroundTrackPos;
	bool trackerOn;
	void SetOn(bool on);
	void SetOldTrackPos(
		V2d &groundPos, V2d &airPos);
	void CalcShownCircles();
	void TryAddTrackPoint( V2d &playerPos );
	bool IsTrackStarted();
	void Reset();
	bool IsOn();
	void SwitchOnOff();
	void Draw(sf::RenderTarget *target);
	void HideAll();
};

#endif