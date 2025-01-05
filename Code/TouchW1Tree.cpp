#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

TouchW1Tree::TouchW1Tree(TouchGrassCollection *coll, int index,
	Edge *e, double quant, int p_variation)
	:TouchGrass(coll, index, e, quant)
{
	currTile = p_variation;//rand() % 4;

	int baseOffset;
	// base offsets 136, 276, 48, 157\
	//base offset is the left-most of the base

	//53, 99, 75, 19, 77

	/*Tree1: 256x234 - 0, 1529
	Tree2 : 369x307 - 274, 1529
	Tree3 : 354x338 - 660, 1529
	Tree4 : 173x207 - 1041, 1529
	Tree5 : 306x271 - 1241, 1529*/

	sf::Vector2i base(0, 1529);

	switch (currTile)
	{
	case 0:
		spriteOrigin = Vector2i(0, 0);
		size = Vector2i(256, 234);
		baseOffset = 53;
		break;
	case 1:
		spriteOrigin = Vector2i(274, 0);
		size = Vector2i(369, 307);
		baseOffset = 99;
		break;
	case 2:
		spriteOrigin = Vector2i(660, 0);
		size = Vector2i(354, 338);
		baseOffset = 75;
		break;
	case 3:
		spriteOrigin = Vector2i(1041, 0);
		size = Vector2i(173, 207);
		baseOffset = 19;
		break;
	case 4:
		spriteOrigin = Vector2i(1241, 0);
		size = Vector2i(306, 271);
		baseOffset = 77;
		break;

	}

	int groundWidth = TouchGrass::GetQuadWidth(TouchGrass::TYPE_W1_TREE);

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

void TouchW1Tree::Reset()
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

void TouchW1Tree::UpdateSprite()
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

void TouchW1Tree::Update()
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

void TouchW1Tree::Touch(Actor *a)
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

void TouchW1Tree::Destroy(Actor *a)
{
	//if (visible)
	//{
	//	//a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
	//	//	6, true, 4);

	//	ClearRect(myQuad);
	//	visible = false;
	//}
}