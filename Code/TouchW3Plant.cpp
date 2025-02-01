#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

TouchW3Plant::TouchW3Plant(TouchGrassCollection *coll, int index,
	Edge *e, double quant, int p_variation)
	:TouchGrass(coll, index, e, quant)
{
	currTile = p_variation;//rand() % 4;

	int baseOffset;
	
	//base offset is the left-most of the base
	//120, 50, 25, 23, 51,77

	//base width
	//76, 56, 112, 220, 47,36
	

	/*Plant1: 335x297 - 950, 1443
	Plant2 : 132x201 - 1328, 1443
	Plant3 : 184x371 - 1538, 1443
	Plant4 : 243x252 - 1742, 1443
	Plant5 : 141x91 - 1047, 1805
	Plant6 : 173x141 - 1242, 1805*/

	sf::Vector2i base(0, 0);

	switch (currTile)
	{
	case 0:
		spriteOrigin = Vector2i(950, 1443);
		spriteSize = Vector2i(335, 297);
		baseOffset = 0;
		break;
	case 1:
		spriteOrigin = Vector2i(1328, 1443);
		spriteSize = Vector2i(132, 201);
		baseOffset = 0;
		break;
	case 2:
		spriteOrigin = Vector2i(1538, 1443);
		spriteSize = Vector2i(184, 371);
		baseOffset = 60;
		break;
	case 3:
		spriteOrigin = Vector2i(1742, 1443);
		spriteSize = Vector2i(243, 252);
		baseOffset = 37;
		break;
	case 4:
		spriteOrigin = Vector2i(1047, 1805);
		spriteSize = Vector2i(141, 91);
		baseOffset = 208;
		break;
	case 5:
		spriteOrigin = Vector2i(1242, 1805);
		spriteSize = Vector2i(173, 141);
		baseOffset = 136;
		break;

	}

	int groundWidth = TouchGrass::GetQuadWidth(TouchGrass::TYPE_W3_PLANT);

	double minScale = .6;
	double maxScale = .8;
	double f = (double)rand() / RAND_MAX;
	double testScale = minScale * (1.0 - f) + maxScale * f;

	size.x = spriteSize.x * testScale;
	size.y = spriteSize.y * testScale;

	//baseOffset = ((spriteSize.x / 2 - baseOffset) - groundWidth / 2) * testScale;
	baseOffset = (spriteSize.x / 2 - baseOffset) * testScale;

	yOffset = 0;


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

	SetRectSubRect(myQuad, coll->ts_grass->GetCustomSubRect(spriteSize, spriteOrigin + base, Vector2i(1, 1), 0));

	Reset();
}

void TouchW3Plant::Reset()
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

void TouchW3Plant::UpdateSprite()
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

void TouchW3Plant::Update()
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

void TouchW3Plant::Touch(Actor *a)
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

void TouchW3Plant::Destroy(Actor *a)
{
	//if (visible)
	//{
	//	//a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
	//	//	6, true, 4);

	//	ClearRect(myQuad);
	//	visible = false;
	//}
}