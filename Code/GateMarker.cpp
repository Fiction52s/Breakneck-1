#include "GateMarker.h"

#include "Physics.h"
#include "EditorGateInfo.h"
#include "EditorTerrain.h"
#include "Session.h"
#include "MainMenu.h"
#include "Zone.h"

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
	ClearRect(quad);
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
	Vector2f size = v.getSize();
	Vector2f vCenter = v.getCenter();
	sf::FloatRect vRect(vCenter.x - size.x / 2, vCenter.y - size.y / 2, size.x, size.y);

	V2d center(currGate->edgeA->v0 + currGate->edgeA->v1);
	center = center / 2.0;

	if (state == HIDE)
	{
		if (!vRect.contains(Vector2f(center)))
		{
			state = FADEIN;
			frame = 0;
		}
		return;
	}
	else if( state == SHOW )
	{
		if (vRect.contains(Vector2f(center)))
		{
			state = HIDE;
			frame = 0;
			ClearRect(quad);
			//state = HIDE;
			//ClearRect(quad);
			return;
		}
	}
		

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
		break;
	}

	int alpha = 255;
	switch (state)
	{
	case FADEIN:
	{
		float fac = ((float)frame) / group->fadeInFrames;
		alpha = 255 * fac;
		break;
	}
	case FADEOUT:
	{
		float fac = ((float)frame) / group->fadeOutFrames;
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


GateMarkerGroup::GateMarkerGroup(int p_maxGates)
{
	assert(p_maxGates > 0);
	currActive = 0;
	maxGates = p_maxGates;
	allQuads = new Vertex[maxGates * 4];

	Session *sess = Session::GetSession();

	ts_gateMarker = sess->GetSizedTileset("HUD/gatemarker_100x30.png");
	font = &sess->mainMenu->arial;
	fadeInFrames = 20;
	fadeOutFrames = 20;

	markers.resize(maxGates);
	for (int i = 0; i < maxGates; ++i)
	{
		markers[i] = new GateMarker(this, allQuads + 4 * i);
	}
}

GateMarkerGroup::~GateMarkerGroup()
{
	delete[] allQuads;

	for (int i = 0; i < maxGates; ++i)
	{
		delete markers[i];
	}
}

void GateMarkerGroup::Reset()
{
	currActive = 0;
	for (int i = 0; i < maxGates; ++i)
	{
		markers[i]->Reset();
	}
}

void GateMarkerGroup::SetToZone(Zone *z)
{
	Reset();
	int counter = 0;
	Gate *g;
	for (auto it = z->gates.begin(); it != z->gates.end(); ++it)
	{
		g = (Gate*)(*it)->info;
		if (g->gState != Gate::LOCKFOREVER && g->gState != Gate::REFORM )
		{
			markers[counter]->SetGate(g);
			++counter;
		}
	}
	currActive = counter;
}

void GateMarkerGroup::Update(sf::View &v)
{
	for (int i = 0; i < currActive; ++i)
	{
		markers[i]->Update(v);
	}
}

void GateMarkerGroup::FadeIn()
{
	for (int i = 0; i < currActive; ++i)
	{
		markers[i]->FadeIn();
	}
}

void GateMarkerGroup::FadeOut()
{
	for (int i = 0; i < currActive; ++i)
	{
		markers[i]->FadeOut();
	}
}


void GateMarkerGroup::Draw(sf::RenderTarget *target)
{
	target->draw(allQuads, currActive * 4, sf::Quads, ts_gateMarker->texture);
	for (int i = 0; i < currActive; ++i)
	{
		markers[i]->Draw(target);
	}
}