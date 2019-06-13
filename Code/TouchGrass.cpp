#include "TouchGrass.h"
#include "GameSession.h"

using namespace sf;
using namespace std;

TouchGrass::TouchGrass(TouchGrassType type, int index, sf::Vertex *v,
	Tileset *p_ts, Edge *e, double q, double rad, double yOff)
	:gType( type ), ts(p_ts), edge( e ), gIndex( index ), radius( rad ), yOffset( yOff ), va( v ),
	quant( q )
{
	myQuad = va + gIndex * 4;

	action = STILL;

	V2d normal = e->Normal();

	V2d center = e->GetPoint(quant);
	center += normal * yOffset;

	float angle = GetVectorAngleCW( normal);

	//SetRectRotation(myQuad, angle, ts->tileWidth, ts->tileHeight, Vector2f(center));
	SetRectRotation(myQuad, angle, 32, 32, Vector2f(center));
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position);
	}
}

void TouchGrass::UpdateSprite()
{
	//SetRectSubRect(myQuad, ts->GetSubRect(0));

	switch (action)
	{
	case STILL:
		SetRectColor(myQuad, Color(Color::Red));
		break;
	case TOUCHED:
		SetRectColor(myQuad, Color(Color::Green));
		break;
	}
	
}

void TouchGrass::HandleQuery(QuadTreeCollider * qtc)
{
	qtc->HandleEntrant(this);
}

bool TouchGrass::IsTouchingBox(const sf::Rect<double> &r)
{
	V2d A(r.left, r.top);
	V2d B(r.left + r.width, r.top);
	V2d C(r.left + r.width, r.top + r.height);
	V2d D(r.left, r.top + r.height);
	bool touching = isQuadTouchingQuad(points[0], points[1], points[2], points[3],
		A, B, C, D);

	return touching;
}

void TouchGrass::Update()
{
	if (action == TOUCHED)
	{
		if (frame == 60)
		{
			action = STILL;
			frame = 0;
		}
	}
	
	UpdateSprite();

	++frame;
}

void TouchGrass::Touch()
{
	action = TOUCHED;
	frame = 0;
}


TouchGrassCollection::TouchGrassCollection()
{
	touchGrassTree = new QuadTree(1000000, 1000000);
}

TouchGrassCollection::~TouchGrassCollection()
{
	delete touchGrassTree;
	delete[] touchGrassVA;

	for (auto it = myGrass.begin(); it != myGrass.end(); ++it)
	{
		delete (*it);
	}
}

void TouchGrassCollection::Draw(sf::RenderTarget *target)
{
	target->draw(touchGrassVA, numTouchGrasses * 4, sf::Quads);
}

void TouchGrassCollection::Query(QuadTreeCollider *qtc, sf::Rect<double> &r)
{
	touchGrassTree->Query(qtc, r);
}

void TouchGrassCollection::UpdateGrass()
{
	for (auto it = myGrass.begin(); it != myGrass.end(); ++it)
	{
		(*it)->Update();
	}
}