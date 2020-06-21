#include "Sequence.h"

using namespace sf;

PanInfo::PanInfo(sf::Vector2f &pos,
	sf::Vector2f &diff,
	int start, int len)
{
	totalDelta = diff;
	startFrame = start;
	frameLength = len;
	origPos = pos;
}

sf::Vector2f PanInfo::GetCurrPos(int f)
{
	Vector2f dest = origPos + totalDelta;
	double ff = (f - startFrame) + 1;
	double a = ff / frameLength;
	float bezVal = bez.GetValue(a);
	Vector2f final = dest * bezVal + origPos * (1.f - bezVal);
	return final;
}