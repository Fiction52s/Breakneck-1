#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"

using namespace sf;
using namespace std;

TouchGrassCollection::TouchGrassCollection()
{
	touchGrassTree = new QuadTree(1000000, 1000000);
	touchGrassVA = NULL;
	ts_grass = NULL;
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

void TouchGrassCollection::Reset()
{
	for (auto it = myGrass.begin(); it != myGrass.end(); ++it)
	{
		(*it)->Reset();
	}
}

void TouchGrassCollection::Draw(sf::RenderTarget *target)
{
	target->draw(touchGrassVA, numTouchGrasses * 4, sf::Quads, ts_grass->texture);
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

void TouchGrassCollection::CreateGrass(int index, Edge *edge, double quant)
{
	TouchGrass *tg; 
	
	switch (gType)
	{
	case TouchGrass::TYPE_NORMAL:
		tg = new BasicTouchGrass(this, index, edge, quant);
		break;
	case TouchGrass::TYPE_TEST:
		tg = new TestTouchGrass(this, index, edge, quant);
		break;
	}

	myGrass.push_back(tg);
	touchGrassTree->Insert(tg);
}

TouchGrass::TouchGrass(TouchGrassCollection *p_coll, int index,
	Edge *e, double q )
	:coll( p_coll ), edge( e ), gIndex( index ), quant( q )
{
	myQuad = coll->touchGrassVA + gIndex * 4;
}

Tileset *TouchGrassCollection::GetTileset( GameSession *owner,
	TouchGrass::TouchGrassType gt)
{
	Tileset *t = NULL;
	switch (gt)
	{
	case TouchGrass::TYPE_NORMAL:
		t = owner->GetTileset("Env/bushtouch_1_01_64x64.png", 64, 64);
		break;
	case TouchGrass::TYPE_TEST:
		t = owner->GetTileset("Env/bushtouch_1_02_128x128.png", 128, 128);
		break;
	}
	return t;
}

int TouchGrass::GetQuadWidth(TouchGrassType gt)
{
	int width = 0;
	switch (gt)
	{
	case TYPE_NORMAL:
		width = 32;
		break;
	case TYPE_TEST:
		width = 128;
		break;
	}
	return width;
}

void TouchGrass::CommonInit( double yOff, double p_angle, double hitboxYOff, double hitboxXSize, double hitboxYSize)
{
	yOffset = yOff;

	V2d normal = edge->Normal();

	V2d p = edge->GetPoint(quant);
	center = p;
	center += normal * yOffset;

	V2d hitboxCenter = p + normal * hitboxYOff;

	angle = p_angle;

	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = angle;
	hurtBox.globalPosition = hitboxCenter;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = hitboxXSize;
	hurtBox.rh = hitboxYSize;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth, 
		coll->ts_grass->tileHeight, Vector2f(center));
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position);
	}

	Reset();
}

bool TouchGrass::Intersects(CollisionBody *cb, int frame)
{
	return cb->Intersects(frame, hurtBody, 0);
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

BasicTouchGrass::BasicTouchGrass(TouchGrassCollection *coll, int index,
	Edge *e, double quant )
	:TouchGrass( coll, index, e, quant )
{
	CommonInit(24, GetVectorAngleCW(e->Along()), 0, 16, 16);
}

void BasicTouchGrass::Reset()
{
	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth, 
		coll->ts_grass->tileHeight, Vector2f(center));
	visible = true;
	action = STILL;
	frame = 0;
}

void BasicTouchGrass::UpdateSprite()
{
	int tileIndex = 0;

	switch (action)
	{
	case TOUCHEDLEFT:
		tileIndex = 1;
		break;
	case TOUCHEDRIGHT:
		tileIndex = 2;
		break;
	case TOUCHEDLAND:
		tileIndex = 3;
		break;
	}

	SetRectSubRect(myQuad, coll->ts_grass->GetSubRect(tileIndex));

	/*switch (action)
	{
	case STILL:
	SetRectColor(myQuad, Color(Color::Red));
	break;
	case TOUCHEDLEFT:
	SetRectColor(myQuad, Color(Color::Green));
	break;
	}*/

}

void BasicTouchGrass::Update()
{
	int time = 40;
	switch (action)
	{
	case TOUCHEDLEFT:
		if (frame == time)
		{
			action = STILL;
			frame = 0;
		}
		break;
	case TOUCHEDRIGHT:
		if (frame == time)
		{
			action = STILL;
			frame = 0;
		}
		break;
	case TOUCHEDLAND:
		if (frame == time)
		{
			action = STILL;
			frame = 0;
		}
		break;
	}
	
	UpdateSprite();

	++frame;
}

void BasicTouchGrass::Touch( Actor *a)
{
	if (a->ground != NULL && a->groundSpeed > 0)
	{
		action = TOUCHEDRIGHT;
	}
	else if (a->ground != NULL && a->groundSpeed < 0)
	{
		action = TOUCHEDLEFT;
	}
	else if (a->action == Actor::LAND || a->action == Actor::LAND2)
	{
		action = TOUCHEDLAND;
	}

	
	frame = 0;
}

void BasicTouchGrass::Destroy(Actor *a)
{
	if (visible)
	{
		GameSession *owner = a->owner;
		owner->ActivateEffect(EffectLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 8,
			6, true, 4);
		ClearRect(myQuad);
		visible = false;
	}
}

TestTouchGrass::TestTouchGrass(TouchGrassCollection *coll, int index,
	Edge *e, double quant)
	:TouchGrass(coll, index, e, quant)
{
	CommonInit(24, GetVectorAngleCW(e->Along()), 0, 16, 16);
}

void TestTouchGrass::Reset()
{
	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth,
		coll->ts_grass->tileHeight, Vector2f(center));
	visible = true;
	action = STILL;
	frame = 0;
}

void TestTouchGrass::UpdateSprite()
{
	int tileIndex = 0;

	switch (action)
	{
	case TOUCHEDLEFT:
		tileIndex = 3;
		break;
	case TOUCHEDRIGHT:
		tileIndex = 2;
		break;
	case TOUCHEDLAND:
		tileIndex = 1;
		break;
	}

	SetRectSubRect(myQuad, coll->ts_grass->GetSubRect(tileIndex));
}

void TestTouchGrass::Update()
{
	int time = 40;
	switch (action)
	{
	case TOUCHEDLEFT:
		if (frame == time)
		{
			action = STILL;
			frame = 0;
		}
		break;
	case TOUCHEDRIGHT:
		if (frame == time)
		{
			action = STILL;
			frame = 0;
		}
		break;
	case TOUCHEDLAND:
		if (frame == time)
		{
			action = STILL;
			frame = 0;
		}
		break;
	}

	UpdateSprite();

	++frame;
}

void TestTouchGrass::Touch(Actor *a)
{
	if (a->ground != NULL && a->groundSpeed > 0)
	{
		action = TOUCHEDRIGHT;
	}
	else if (a->ground != NULL && a->groundSpeed < 0)
	{
		action = TOUCHEDLEFT;
	}
	else if (a->action == Actor::LAND || a->action == Actor::LAND2)
	{
		action = TOUCHEDLAND;
	}


	frame = 0;
}

void TestTouchGrass::Destroy(Actor *a)
{
	if (visible)
	{
		GameSession *owner = a->owner;
		owner->ActivateEffect(EffectLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
			6, true, 4);

		ClearRect(myQuad);
		visible = false;
	}
}