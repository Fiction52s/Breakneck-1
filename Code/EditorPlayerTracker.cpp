#include "EditorPlayerTracker.h"
#include "CircleGroup.h"
#include "MovingGeo.h"

using namespace sf;

PlayerTracker::PlayerTracker()
{
	trackerOn = false;
	trackPoints = 0;
	mostRecentTrackPoint = 0;
	testPlayerTracker = new CircleGroup(1000, 12, Color::Green, 6);
	startTrackColor = Color::Red;
	endTrackColor = Color::Green;
}

PlayerTracker::~PlayerTracker()
{
	delete testPlayerTracker;
}

void PlayerTracker::SetOn(bool on)
{
	trackerOn = on;
	if (!trackerOn)
	{
		Reset();
	}
}

void PlayerTracker::HideAll()
{
	testPlayerTracker->HideAll();
}

void PlayerTracker::Draw(sf::RenderTarget *target)
{
	if (trackerOn)
	{
		testPlayerTracker->Draw(target);
	}
}

void PlayerTracker::SwitchOnOff()
{
	SetOn(!trackerOn);
}

bool PlayerTracker::IsOn()
{
	return trackerOn;
}

void PlayerTracker::SetOldTrackPos(
	V2d &groundPos, V2d &airPos)
{
	playerOldGroundTrackPos = groundPos;
	playerOldTrackPos = airPos;
}

void PlayerTracker::CalcShownCircles()
{
	int numCircles = testPlayerTracker->numCircles;
	float blend;
	float diff;

	if (trackPoints >= numCircles)
	{
		testPlayerTracker->ShowAll();

		for (int i = 0; i < numCircles; ++i)
		{
			if (i <= mostRecentTrackPoint)
			{
				diff = mostRecentTrackPoint - i;
			}
			else
			{
				diff = numCircles - (i - mostRecentTrackPoint);
			}
			blend = 1.f - diff / (trackPoints);

			testPlayerTracker->SetColor(i, GetBlendColor(startTrackColor,
				endTrackColor, blend));
		}
	}
	else
	{
		testPlayerTracker->HideAll();
		for (int i = 0; i <= mostRecentTrackPoint; ++i)
		{
			testPlayerTracker->SetVisible(i, true);
			diff = mostRecentTrackPoint - i;
			blend = 1.f - diff / (trackPoints);
			testPlayerTracker->SetColor(i, GetBlendColor(startTrackColor,
				endTrackColor, blend));
		}
	}
}

void PlayerTracker::TryAddTrackPoint(V2d &playerPos)
{
	int lastTrackPoint = mostRecentTrackPoint - 1;
	if (lastTrackPoint < 0 && trackPoints > 0)
		lastTrackPoint = testPlayerTracker->numCircles - 1;

	Vector2f pTrackPos = Vector2f(playerPos);
	if (lastTrackPoint < 0 || pTrackPos != testPlayerTracker->GetPosition(lastTrackPoint))
	{
		if (mostRecentTrackPoint == testPlayerTracker->numCircles)
			mostRecentTrackPoint = 0;

		testPlayerTracker->SetPosition(mostRecentTrackPoint, pTrackPos);
		mostRecentTrackPoint++;
		trackPoints++;
	}
}

bool PlayerTracker::IsTrackStarted()
{
	return (trackPoints > 0);
}

void PlayerTracker::Reset()
{
	mostRecentTrackPoint = 0;
	trackPoints = 0;
	testPlayerTracker->HideAll();
}