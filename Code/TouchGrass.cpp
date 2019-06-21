#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"

using namespace sf;
using namespace std;

TouchGrass::TouchGrass(TouchGrassType type, int index, sf::Vertex *v,
	Tileset *p_ts, Edge *e, double q, double rad, double yOff)
	:gType( type ), ts(p_ts), edge( e ), gIndex( index ), radius( rad ), yOffset( yOff ), va( v ),
	quant( q )
{
	visible = true;
	myQuad = va + gIndex * 4;

	action = STILL;

	V2d normal = e->Normal();

	center = e->GetPoint(quant);
	center += normal * yOffset;

	angle = GetVectorAngleCW( e->Along());

	SetRectRotation(myQuad, angle, ts->tileWidth, ts->tileHeight, Vector2f(center));
	//SetRectRotation(myQuad, angle, 32, 32, Vector2f(center));
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position);
	}

	CollisionBox hurtBox;
	hurtBox.type = CollisionBox::Hurt;
	hurtBox.isCircle = true;
	hurtBox.globalAngle = angle;
	hurtBox.globalPosition = center;
	hurtBox.offset.x = 0;
	hurtBox.offset.y = 0;
	hurtBox.rw = 16;
	hurtBox.rh = 16;
	hurtBody = new CollisionBody(1);
	hurtBody->AddCollisionBox(0, hurtBox);

	/*CollisionBox hitBox;
	hitBox.type = CollisionBox::Hit;
	hitBox.isCircle = true;
	hitBox.globalAngle = angle;
	hitBox.offset.x = 0;
	hitBox.offset.y = 0;
	hitBox.rw = 32;
	hitBox.rh = 32;
	hitBody = new CollisionBody(1);
	hitBody->AddCollisionBox(0, hitBox);


	hitboxInfo = new HitboxInfo;
	hitboxInfo->damage = 3 * 60;
	hitboxInfo->drainX = 0;
	hitboxInfo->drainY = 0;
	hitboxInfo->hitlagFrames = 0;
	hitboxInfo->hitstunFrames = 10;
	hitboxInfo->knockback = 4;

	hitBody->hitboxInfo = hitboxInfo;*/
	//SetHitboxes(hitBody, 0);
	//SetHurtboxes(hurtBody, 0);
}

void TouchGrass::Reset()
{
	visible = true;
	SetRectRotation(myQuad, angle, ts->tileWidth, ts->tileHeight, Vector2f(center));
	action = STILL;
	frame = 0;
}

bool TouchGrass::Intersects(CollisionBody *cb, int frame)
{
	return cb->Intersects(frame, hurtBody, 0);
}

void TouchGrass::UpdateSprite()
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

	SetRectSubRect(myQuad, ts->GetSubRect(tileIndex));

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

void TouchGrass::Touch( Actor *a)
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

void TouchGrass::Destroy(Actor *a)
{
	if (visible)
	{
		GameSession *owner = a->owner;
		owner->ActivateEffect(EffectLayer::BEHIND_ENEMIES, ts, center, false, angle, 8,
			6, true, 4);
		ClearRect(myQuad);
		visible = false;
	}
}


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
	target->draw(touchGrassVA, numTouchGrasses * 4, sf::Quads, ts_grass->texture );
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