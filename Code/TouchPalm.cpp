#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

TouchPalm::TouchPalm(TouchGrassCollection *coll, int index,
	Edge *e, double quant)
	:TouchGrass(coll, index, e, quant)
{
	size.x = 288;
	size.y = 386;
	yOffset = 0;
	V2d normal = edge->Normal();

	V2d p = edge->GetPosition(quant);
	center = p;
	center += normal * yOffset;

	center.y -= size.y / 2.f;// -20; //arbitrary 20 rn

	V2d hitboxCenter = center;//p + normal * hitboxYOff;

	angle = 0;

	//probably make this a rectangle later.
	//hurtBody.BasicCircleSetup(16, hitboxCenter);

	SetRectRotation(myQuad, angle, size.x,//coll->ts_grass->tileWidth,
		size.y, Vector2f(center));

	V2d polyCenter = coll->myTerrain->GetDCenter();
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position) - polyCenter;
	}

	Reset();
}

void TouchPalm::Reset()
{	
	//float treeHeight = 500;//coll->ts_grass->tileHeight
	SetRectRotation(myQuad, angle, size.x,//coll->ts_grass->tileWidth,
		size.y, Vector2f(center));
	visible = true;
	action = STILL;
	frame = 0;
	currTile = -1;
	UpdateSprite();
}

void TouchPalm::UpdateSprite()
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

	if (tileIndex != currTile)
	{
		currTile = tileIndex;
		SetRectSubRect(myQuad, coll->ts_grass->GetCustomSubRect( size, Vector2i( 0, 0 ), Vector2i( 1, 1 ), 0));
	}

}

void TouchPalm::Update()
{
	/*int time = 40;
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
	}*/

	UpdateSprite();

	++frame;
}

void TouchPalm::Touch(Actor *a)
{
	/*if (a->ground != NULL && a->groundSpeed > 0)
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
	}*/


	frame = 0;
}

void TouchPalm::Destroy(Actor *a)
{
	//if (visible)
	//{
	//	//a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
	//	//	6, true, 4);

	//	ClearRect(myQuad);
	//	visible = false;
	//}
}