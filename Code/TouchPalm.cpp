#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

TouchPalm::TouchPalm(TouchGrassCollection *coll, int index,
	Edge *e, double quant, int p_variation)
	:TouchGrass(coll, index, e, quant)
{
	currTile = p_variation;//rand() % 4;

	int baseOffset;
	// base offsets 136, 276, 48, 157

	switch (currTile)
	{
	case 0:
		spriteOrigin = Vector2i(0, 0);
		size = Vector2i(285, 383);
		baseOffset = 136;
		break;
	case 1:
		spriteOrigin = Vector2i(286, 0);
		size = Vector2i(343, 422);
		baseOffset = 276;
		break;
	case 2:
		spriteOrigin = Vector2i(632, 0);
		size = Vector2i(295, 403);
		baseOffset = 48;
		break;
	case 3:
		spriteOrigin = Vector2i(930, 0);
		size = Vector2i(231, 426);
		baseOffset = 157;
		break;
	}

	int groundWidth = TouchGrass::GetQuadWidth(TouchGrass::TYPE_PALM);

	baseOffset = (size.x / 2 - baseOffset) - groundWidth / 2;

	yOffset = 0;
	V2d normal = edge->Normal();

	V2d p = edge->GetPosition(quant);

	V2d testPoint = p;//

	if (normal.x != 0)
	{
		if (normal.x > 0)
		{
			testPoint.x += groundWidth / 2;
		}
		else if (normal.x < 0)
		{
			testPoint.x -= groundWidth / 2;
		}

		LineIntersection li;
		lineIntersection(li, testPoint, testPoint + V2d(0, 1), edge->v0, edge->v1);
		double verticalDiff = li.position.y - p.y;

		p.y += verticalDiff;
	}
	
	//+V2d(groundWidth / 2, 0);

	center = p;
	center += normal * yOffset;

	center.y -= size.y / 2.f;// -20; //arbitrary 20 rn

	center.x += baseOffset;

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

	

	SetRectSubRect(myQuad, coll->ts_grass->GetCustomSubRect(size, spriteOrigin, Vector2i(1, 1), 0));

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
	//currTile = -1;
	UpdateSprite();
}

void TouchPalm::UpdateSprite()
{
	int tileIndex = 0;

	/*switch (action)
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
	}*/

	//if (tileIndex != currTile)
	{
		//currTile = tileIndex;
		
	}

	/*
	Tree1
	Size: 300x404
	Coord: 0,0

	Tree2
	Size: 362x446
	Coord: 347,0

	Tree3
	Size: 311x426
	Coord: 755x0

	Tree4
	Size: 243x450
	Coord: 1113x0
	*/

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