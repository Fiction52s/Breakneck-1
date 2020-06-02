#include "GateMarker.h"

#include "Physics.h"
#include "EditorGateInfo.h"
#include "EditorTerrain.h"
#include "Session.h"
#include "MainMenu.h"

using namespace sf;
using namespace std;

GateMarker::GateMarker( GateMarkerGroup *g, sf::Vertex *p_quad)
{
	group = g;
	quad = p_quad;


	
	numText.setFillColor(Color::Black);
	numText.setCharacterSize(32);
	numText.setFont(*group->font);


	SetRectColor(quad, Color::Green);
	group->ts_gateMarker->SetQuadSubRect(quad, 0);

	currInfo = NULL;

	currGate = NULL;

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

void GateMarker::SetGate(Gate *g)
{
	currGate = g;
	numText.setString(to_string(g->numToOpen));
	auto bounds = numText.getLocalBounds();
	numText.setOrigin(bounds.left + bounds.width / 2,
		bounds.top + bounds.height / 2);
}

void GateMarker::Update(sf::View &v )
{		
	if (state == HIDE)
		return;

	switch (state)
	{
	case FADEIN:
		if (frame == group->fadeInFrames)
		{
			state = SHOW;
		}
		break;
	case FADEOUT:
		if (frame == group->fadeOutFrames)
		{
			state = HIDE;
			ClearRect(quad);
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

	Color testColor = Color::Green;
	testColor.a = alpha;
	SetRectColor(quad, testColor);

	Color textColor = Color::Black;
	textColor.a = alpha;
	numText.setFillColor(textColor);

	Vector2f size = v.getSize();
	Vector2f vCenter = v.getCenter();
	sf::FloatRect vRect(vCenter.x - size.x / 2, vCenter.y - size.y / 2, size.x, size.y);

	V2d center(currGate->edgeA->v0 + currGate->edgeA->v1);
	center = center / 2.0;

	if (vRect.contains( Vector2f( center ) ) )
	{
		state = HIDE;
		frame = 0;
		ClearRect(quad);
		return;
	}

	
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

	double ang = GetVectorAngleCW(dir);
	

	V2d currPos = li.position - dir * 60.0;
	Vector2f currPosF(currPos);

	numText.setPosition(currPosF);

	SetRectRotation(quad, ang, 100, 30, currPosF);
	//SetRectCenter(quad, 50, 50, Vector2f(currPos));
}

void GateMarker::Draw(sf::RenderTarget *target)
{
	if (state != HIDE)
	{
		//target->draw(quad, 4, sf::Quads, ts_gateMarker->texture);
		target->draw(numText);
	}
}


GateMarkerGroup::GateMarkerGroup(int maxGates)
{
	allQuads = new Vertex[maxGates * 4];

	Session *sess = Session::GetSession();

	ts_gateMarker = sess->GetSizedTileset("HUD/gatemarker_100x30.png");
	font = &sess->mainMenu->arial;
	fadeInFrames = 60;
	fadeOutFrames = 60;
}

GateMarkerGroup::~GateMarkerGroup()
{
	delete[] allQuads;
}

void GateMarkerGroup::SetToZone(Zone *z)
{

}

void GateMarkerGroup::Update(sf::View &v)
{

}

void GateMarkerGroup::Draw(sf::RenderTarget *target)
{
	target->draw( allQuads, )
}