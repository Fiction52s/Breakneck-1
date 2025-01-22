#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

TouchW2Plant::TouchW2Plant(TouchGrassCollection *coll, int index,
	Edge *e, double quant, int p_variation)
	:TouchGrass(coll, index, e, quant)
{
	currTile = p_variation;//rand() % 4;

	int baseOffset;
	
	//base offset is the left-most of the base
	//0, 0, 60, 37, 208, 136

	//base width
	//226, 198, 66, 36, 14, 10

	/*Plant1: 254x389 - 924, 1067
	Plant2 : 198x225 - 1239, 1231
	Plant3 : 154x223 - 1576, 1216
	Plant4 : 108x160 - 1812, 1279
	Plant5 : 409x338 - 928, 1596
	Plant6 : 260x116 - 1427, 1818*/

	sf::Vector2i base(0, 0);

	switch (currTile)
	{
	case 0:
		spriteOrigin = Vector2i(924, 1067);
		size = Vector2i(254, 389);
		baseOffset = 0;
		break;
	case 1:
		spriteOrigin = Vector2i(1239, 1231);
		size = Vector2i(198, 225);
		baseOffset = 0;
		break;
	case 2:
		spriteOrigin = Vector2i(1576, 1216);
		size = Vector2i(154, 223);
		baseOffset = 60;
		break;
	case 3:
		spriteOrigin = Vector2i(1812, 1279);
		size = Vector2i(108, 160);
		baseOffset = 37;
		break;
	case 4:
		spriteOrigin = Vector2i(928, 1596);
		size = Vector2i(409, 338);
		baseOffset = 208;
		break;
	case 5:
		spriteOrigin = Vector2i(1427, 1818);
		size = Vector2i(260, 116);
		baseOffset = 136;
		break;

	}

	int groundWidth = TouchGrass::GetQuadWidth(TouchGrass::TYPE_W2_PLANT);

	baseOffset = (size.x / 2 - baseOffset) - groundWidth / 2;

	yOffset = 0;






	//yOffset = yOff;

	//V2d normal = edge->Normal();

	//V2d p = edge->GetPosition(quant);
	//center = p;
	//center += normal * yOffset;

	//V2d hitboxCenter = p + normal * hitboxYOff;

	//angle = p_angle;

	////probably make this a rectangle later.
	//hurtBody.BasicCircleSetup(hitboxXSize, hitboxCenter);

	//SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth,
	//	coll->ts_grass->tileHeight, Vector2f(center));


	//V2d polyCenter = coll->myTerrain->GetDCenter();
	//for (int i = 0; i < 4; ++i)
	//{
	//	points[i] = V2d(myQuad[i].position) - polyCenter;
	//}

	//Reset();




	V2d normal = edge->Normal();

	V2d p = edge->GetPosition(quant);

	V2d along = e->Along();

	center = p;
	center += along * (double)baseOffset;
	center += normal * (yOffset + size.y / 2.f);

	angle = GetVectorAngleCW(along);

	SetRectRotation(myQuad, angle, size.x, size.y, Vector2f(center));

	V2d polyCenter = coll->myTerrain->GetDCenter();
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position) - polyCenter;
	}

	SetRectSubRect(myQuad, coll->ts_grass->GetCustomSubRect(size, spriteOrigin + base, Vector2i(1, 1), 0));

	Reset();
}

void TouchW2Plant::Reset()
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

void TouchW2Plant::UpdateSprite()
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

void TouchW2Plant::Update()
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

void TouchW2Plant::Touch(Actor *a)
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

void TouchW2Plant::Destroy(Actor *a)
{
	//if (visible)
	//{
	//	//a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
	//	//	6, true, 4);

	//	ClearRect(myQuad);
	//	visible = false;
	//}
}