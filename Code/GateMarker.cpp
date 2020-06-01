#include "GateMarker.h"

#include "Physics.h"
#include "EditorGateInfo.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

GateMarker::GateMarker()
{
	SetRectColor(quad, Color::Green);
	fadeInFrames = 60;
	fadeOutFrames = 60;
	Reset();
}

void GateMarker::Reset()
{
	state = HIDE;
	frame = 0;
}

void GateMarker::FadeIn()
{
	state = FADEIN;
	frame = 0;
}

void GateMarker::FadeOut()
{
	state = FADEOUT;
	frame = 0;
}

//void GateMarker::Show()
//{
//
//}
//void GateMarker::Hide()
//{
//
//}

void GateMarker::Update(sf::View &v,
	GateInfo *gi)
{
	if (state == HIDE)
		return;

	switch (state)
	{
	case FADEIN:
		if (frame == fadeInFrames)
		{
			state = SHOW;
		}
		break;
	case FADEOUT:
		if (frame == fadeOutFrames)
		{
			state = HIDE;
			return;
		}
	}

	int alpha = 255;
	switch (state)
	{
	case FADEIN:
	{
		float fac = ((float)frame) / fadeInFrames;
		alpha = 255 * fac;
		break;
	}
	case FADEOUT:
	{
		float fac = ((float)frame) / fadeOutFrames;
		alpha = 255 * (1.f - fac);
		break;
	}
	}

	sf::FloatRect aabb(gi->GetAABB());

	Vector2f size = v.getSize();
	Vector2f vCenter = v.getCenter();
	sf::FloatRect vRect(vCenter.x - size.x / 2, vCenter.y - size.y / 2, size.x, size.y);

	/*if (vRect.intersects(aabb) )
	{
		show = false;
		return;
	}*/

	//sf::FloatRect vAABB( v.)

	V2d center(gi->point0->pos + gi->point1->pos);
	center = center / 2.0;
	V2d pos = center - V2d(v.getCenter());
	SetGatePos(pos);

	++frame;

	if (state == SHOW)
		frame = 0;
	//show = true;
}

void GateMarker::SetGatePos(V2d pos)
{
	V2d start(960, 540);
	
	
	V2d dir = normalize( pos );
	V2d end = start + dir * 2000.0;//pos;//+ dir * 2000.0;

	V2d quadPoints[4];
	quadPoints[0] =V2d(0, 0);
	quadPoints[1] = V2d(1920, 0);
	quadPoints[2] = V2d(1920, 1080);
	quadPoints[3] = V2d(0, 1080);

	int nexti;
	LineIntersection li;
	for (int i = 0; i < 4; ++i)
	{
		if (i == 4 - 1)
		{
			nexti = 0;
		}
		else
		{
			nexti = i + 1;
		}
		li = SegmentIntersect(start, end, quadPoints[i], quadPoints[nexti]);
		if (!li.parallel)
		{
			break;
		}
	}

	V2d currPos = li.position;// -dir * 200.0;

	SetRectCenter(quad, 50, 50, Vector2f(currPos));
}

void GateMarker::Draw(sf::RenderTarget *target)
{
	if (state != SHOW)
	{
		target->draw(quad, 4, sf::Quads);
	}
}