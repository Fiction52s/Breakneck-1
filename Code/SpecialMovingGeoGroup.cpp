#include "MovingGeo.h"
#include "VectorMath.h"
#include <assert.h>
#include <iostream>

using namespace std;
using namespace sf;

SpecialMovingGeoGroup::SpecialMovingGeoGroup()
{
	frame = 0;
	points = NULL;
	numTotalPoints = 0;
}

SpecialMovingGeoGroup::~SpecialMovingGeoGroup()
{
	RemoveAll();
}

void SpecialMovingGeoGroup::RemoveAll()
{
	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		delete (*it);
	}
	geoVector.clear();
	delete[] points;
	points = NULL;
}

void SpecialMovingGeoGroup::Reset()
{
	frame = 0;
	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		(*it)->Reset();
		(*it)->done = true;
	}
	running = false;
}

void SpecialMovingGeoGroup::Start()
{
	Reset();
	running = true;
	continueGenerating = true;
}

void SpecialMovingGeoGroup::StopGenerating()
{
	continueGenerating = false;
}

void SpecialMovingGeoGroup::SetBase(sf::Vector2f &pos)
{
	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		(*it)->SetBase(pos);
	}
}


bool SpecialMovingGeoGroup::Update()
{
	if (!running)
		return false;

	bool geoActive = false;

	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		(*it)->Update();
		if (!(*it)->done)
		{
			geoActive = true;
		}
	}

	if (continueGenerating)
	{
		StartGeo();
	}
	

	++frame;

	if (!continueGenerating && !geoActive)
	{
		running = false;
	}
	
	return running;
}

void SpecialMovingGeoGroup::Draw(sf::RenderTarget *target)
{
	if( running )
		target->draw(points, numTotalPoints, sf::Quads);
}

void SpecialMovingGeoGroup::AddGeo(MovingGeo *mg)
{
	geoVector.push_back(mg);
}

void SpecialMovingGeoGroup::Init()
{
	numTotalPoints = 0;
	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		numTotalPoints += (*it)->GetNumPoints();
	}

	points = new Vertex[numTotalPoints];

	sf::Vertex *curr = points;
	int np;
	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		np = (*it)->GetNumPoints();
		(*it)->SetPoints(curr);
		curr += np;
		(*it)->Init();
	}

	Reset();
}

PokeTriangleScreenGeoGroup::PokeTriangleScreenGeoGroup()
	:SpecialMovingGeoGroup()
{
	int numTris = 300;

	Vector2f offset(2000, 0);
	Transform t;

	double extraThresh = 100;
	double extraYThresh = 100;
	V2d A(-960 - extraThresh, -540 - extraThresh - extraYThresh);
	V2d B(960 + extraThresh, -540 - extraThresh - extraYThresh);
	V2d C(960 + extraThresh, 540 + extraThresh + extraYThresh);
	V2d D(-960 - extraThresh, 540 + extraThresh + extraYThresh);

	LineIntersection li;

	lengthFactor = 0;

	for (int i = 0; i < numTris; ++i)
	{
		//geoGroup.AddGeo(new PokeTri((2 * PI / numTris) * i), i * 10);
		Vector2f tPoint = t.transformPoint(offset);

		li = SegmentIntersect(V2d(), V2d(tPoint), A, B);
		if (li.parallel)
		{
			li = SegmentIntersect(V2d(), V2d(tPoint), B, C);
			if (li.parallel)
			{
				li = SegmentIntersect(V2d(), V2d(tPoint), C, D);
				if (li.parallel)
				{
					li = SegmentIntersect(V2d(), V2d(tPoint), D, A);
					assert(!li.parallel);
				}
			}
		}

		AddGeo(new PokeTri(Vector2f(li.position)));
		t.rotate(360.f / numTris);
	}

	Init();

	SetBase(Vector2f(960, 540));
}

PokeTriangleScreenGeoGroup::~PokeTriangleScreenGeoGroup()
{

}

void PokeTriangleScreenGeoGroup::StartGeo()
{
	for (int i = 0; i < 4; ++i)
	{
		while (true)
		{
			int r = rand() % geoVector.size();
			if (geoVector[r]->done)
			{
				geoVector[r]->Reset();
				break;
			}
		}
	}
}

void PokeTriangleScreenGeoGroup::SetLengthFactor(float f)
{
	lengthFactor = f;
	PokeTri *pt;
	for (auto it = geoVector.begin(); it != geoVector.end(); ++it)
	{
		pt = (PokeTri*)(*it);
		pt->SetLengthFactor(f);
	}
}