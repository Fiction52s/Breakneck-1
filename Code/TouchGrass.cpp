#include "TouchGrass.h"
#include "GameSession.h"
#include "Actor.h"
#include "EditorTerrain.h"

using namespace sf;
using namespace std;

TouchGrassCollection::TouchGrassCollection( TerrainPolygon *tp )
	:myTerrain(tp)
{
	touchGrassTree = new QuadTree(1000000, 1000000);
	touchGrassVA = NULL;
	ts_grass = NULL;
}

TouchGrassCollection::~TouchGrassCollection()
{
	delete touchGrassTree;

	if (touchGrassVA != NULL)
	{
		delete[] touchGrassVA;
	}

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

void TouchGrassCollection::Move(V2d &move)
{
	for (auto it = myGrass.begin(); it != myGrass.end(); ++it)
	{
		(*it)->Move(move);
	}
}

void TouchGrassCollection::Draw(sf::RenderTarget *target)
{
	/*for (auto it = myGrass.begin(); it != myGrass.end(); ++it)
	{
		(*it)->hurtBody->DebugDraw( 0, target);
	}*/
	target->draw(touchGrassVA, numTouchGrasses * 4, sf::Quads, ts_grass->texture);
}

void TouchGrassCollection::Query(QuadTreeCollider *qtc, sf::Rect<double> &r)
{
	touchGrassTree->Query(qtc, r);
}

void TouchGrassCollection::UpdateGrass()
{
	auto end = myGrass.end();
	for (auto it = myGrass.begin(); it != end; ++it)
	{
		(*it)->Update();
	}
}

void TouchGrassCollection::CreateGrass(int index, Edge *edge, double quant)
{
	TouchGrass *tg = NULL;
	
	switch (gType)
	{
	case TouchGrass::TYPE_NORMAL_W1:
	case TouchGrass::TYPE_NORMAL_W2:
	case TouchGrass::TYPE_NORMAL_W3:
	case TouchGrass::TYPE_NORMAL_W4:
	case TouchGrass::TYPE_NORMAL_W5:
	case TouchGrass::TYPE_NORMAL_W6:
	case TouchGrass::TYPE_LARGE_W1:
	case TouchGrass::TYPE_LARGE_W2:
	case TouchGrass::TYPE_LARGE_W3:
	case TouchGrass::TYPE_LARGE_W4:
	case TouchGrass::TYPE_LARGE_W5:
	case TouchGrass::TYPE_LARGE_W6:
		tg = new BasicTouchGrass(this, index, edge, quant);
		break;
	case TouchGrass::TYPE_TREE:
		tg = new TouchTree(this, index, edge, quant);
		break;
	}

	assert(tg != NULL);

	myGrass.push_back(tg);
	touchGrassTree->Insert(tg);
}

TouchGrass::TouchGrass(TouchGrassCollection *p_coll, int index,
	Edge *e, double q )
	:coll( p_coll ), edge( e ), gIndex( index ), quant( q ), hurtBody( CollisionBox::Hurt )
{
	myQuad = coll->touchGrassVA + gIndex * 4;
	//SetRectColor(myQuad, Color::Red);
}

TouchGrass::~TouchGrass()
{
}

//remove this later after gamesession is a tilesetmanager
//Tileset *TouchGrassCollection::GetTileset( GameSession *owner,
//	TouchGrass::TouchGrassType gt)
//{
//	Tileset *t = NULL;
//	switch (gt)
//	{
//	case TouchGrass::TYPE_NORMAL:
//		t = owner->GetTileset("Env/bushtouch_1_01_64x64.png", 64, 64);
//		break;
//	case TouchGrass::TYPE_TEST:
//		t = owner->GetTileset("Env/bushtouch_1_02_128x128.png", 128, 128);
//		break;
//	}
//	return t;
//}


Tileset *TouchGrassCollection::GetTileset(TilesetManager *tm,
	TouchGrass::TouchGrassType gt)
{
	Tileset *t = NULL;
	switch (gt)
	{
	case TouchGrass::TYPE_NORMAL_W1:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_1_01_64x64.png");
		break;
	case TouchGrass::TYPE_NORMAL_W2:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_2_01_64x64.png");
		break;
	case TouchGrass::TYPE_NORMAL_W3:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_3_01_64x64.png");
		break;
	case TouchGrass::TYPE_NORMAL_W4:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_4_01_64x64.png");
		break;
	case TouchGrass::TYPE_NORMAL_W5:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_5_01_64x64.png");
		break;
	case TouchGrass::TYPE_NORMAL_W6:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_6_01_64x64.png");
		break;
	/*case TouchGrass::TYPE_NORMAL_W7:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_7_01_64x64.png");
		break;*/
	case TouchGrass::TYPE_LARGE_W1:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_1_02_128x128.png");
		break;
	case TouchGrass::TYPE_LARGE_W2:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_2_02_128x128.png");
		break;
	case TouchGrass::TYPE_LARGE_W3:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_3_02_128x128.png");
		break;
	case TouchGrass::TYPE_LARGE_W4:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_4_02_128x128.png");
		break;
	case TouchGrass::TYPE_LARGE_W5:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_5_02_128x128.png");
		break;
	case TouchGrass::TYPE_LARGE_W6:
		t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_6_02_128x128.png");
		break;
	//case TouchGrass::TYPE_LARGE_W7:
	//	t = tm->GetSizedTileset("Env/TouchGrass/bushtouch_7_02_128x128.png");
	//	break;
	case TouchGrass::TYPE_TREE:
		t = tm->GetTileset("Env/TouchGrass/trees_128x128.png", 128, 128);
		break;
	default:
		assert(0);
	}
	return t;
}

int TouchGrass::GetQuadWidth(TouchGrassType gt)
{
	int width = 0;
	switch (gt)
	{
	case TYPE_NORMAL_W1:
	case TYPE_NORMAL_W2:
	case TYPE_NORMAL_W3:
	case TYPE_NORMAL_W4:
	case TYPE_NORMAL_W5:
	case TYPE_NORMAL_W6:
		width = 32;
		break;
	case TYPE_LARGE_W1:
	case TYPE_LARGE_W2:
	case TYPE_LARGE_W3:
	case TYPE_LARGE_W4:
	case TYPE_LARGE_W5:
	case TYPE_LARGE_W6:
		width = 128;
		break;
	case TYPE_TREE:
		width = 128;
		break;
	}

	if (width == 0)
	{
		assert(0);
	}

	return width;
}

void TouchGrass::Move(V2d &move)
{
	hurtBody.Move(move);
	center += move;

	Vector2f fMove(move);
	for (int i = 0; i < 4; ++i)
	{
		myQuad[i].position += fMove;
	}

	//points dont move because they are relatively spaced to the poly center
}

void TouchGrass::CommonInit( double yOff, double p_angle, double hitboxYOff, double hitboxXSize, double hitboxYSize)
{
	yOffset = yOff;

	V2d normal = edge->Normal();

	V2d p = edge->GetPosition(quant);
	center = p;
	center += normal * yOffset;

	V2d hitboxCenter = p + normal * hitboxYOff;

	angle = p_angle;

	//probably make this a rectangle later.
	hurtBody.BasicCircleSetup(hitboxXSize, hitboxCenter);

	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth, 
		coll->ts_grass->tileHeight, Vector2f(center));

	
	V2d polyCenter = coll->myTerrain->GetDCenter();
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position) - polyCenter;
	}

	Reset();
}

bool TouchGrass::Intersects(CollisionBody *cb, int frame)
{
	return cb->Intersects(frame, &hurtBody, 0);
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

bool TouchGrass::IsPlacementOkay( TouchGrassType grassType, int p_eat,
	Edge *edge, int quadIndex)
{
	EdgeAngleType eat = (EdgeAngleType)p_eat;
	switch (grassType)
	{
	case TYPE_NORMAL_W1:
	case TYPE_NORMAL_W2:
	case TYPE_NORMAL_W3:
	case TYPE_NORMAL_W4:
	case TYPE_NORMAL_W5:
	case TYPE_NORMAL_W6:
	{
		int r = rand() % 100;
		return (r < 50);
		break;
	}
	case TYPE_LARGE_W1:
	case TYPE_LARGE_W2:
	case TYPE_LARGE_W3:
	case TYPE_LARGE_W4:
	case TYPE_LARGE_W5:
	case TYPE_LARGE_W6:
	{
		int r = rand() % 100;
		return (r < 15);
		break;
	}
	case TYPE_TREE:
	{
		int r = rand() % 100;
		return (r < 15);
		break;
	}
	default:
		assert(0);
		break;
	}

	return false;
}

BasicTouchGrass::BasicTouchGrass(TouchGrassCollection *coll, int index,
	Edge *e, double quant )
	:TouchGrass( coll, index, e, quant )
{
	double yOff = 0;
	switch (coll->gType)
	{
	case TYPE_NORMAL_W1:
	case TYPE_NORMAL_W2:
	case TYPE_NORMAL_W3:
	case TYPE_NORMAL_W4:
	case TYPE_NORMAL_W5:
	case TYPE_NORMAL_W6:
		yOff = 29;
		//yOff = 14;
		break;
	case TYPE_LARGE_W1:
	case TYPE_LARGE_W2:
	case TYPE_LARGE_W3:
	case TYPE_LARGE_W4:
	case TYPE_LARGE_W5:
	case TYPE_LARGE_W6:
		yOff = 29;
		break;
	}
	CommonInit(yOff, GetVectorAngleCW(e->Along()), 0, 16, 16);
}

void BasicTouchGrass::Reset()
{
	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth, 
		coll->ts_grass->tileHeight, Vector2f(center));
	visible = true;
	action = STILL;
	frame = 0;
	currTile = -1;
	UpdateSprite();
}

void BasicTouchGrass::UpdateSprite()
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
		SetRectSubRect(myQuad, coll->ts_grass->GetSubRect(tileIndex));
	}

	

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
		switch (coll->gType)
		{
		/*case TYPE_NORMAL_W1:
		{
			a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 8,
				6, true, 4);
			break;
		}
		case TYPE_LARGE_W1:
		{
			a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
				6, true, 4);
			break;
		}*/
		case TYPE_NORMAL_W1:
		case TYPE_NORMAL_W2:
		case TYPE_NORMAL_W3:
		case TYPE_NORMAL_W4:
		case TYPE_NORMAL_W5:
		case TYPE_NORMAL_W6:
		{
			a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 14,
				6, true, 4);
			break;
		}
		case TYPE_LARGE_W1:
		case TYPE_LARGE_W2:
		case TYPE_LARGE_W3:
		case TYPE_LARGE_W4:
		case TYPE_LARGE_W5:
		case TYPE_LARGE_W6:
		{
			a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 14,
				6, true, 4);
			break;
		}
		}

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
	currTile = -1;
	UpdateSprite();
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

	if (tileIndex != currTile)
	{
		currTile = tileIndex;
		SetRectSubRect(myQuad, coll->ts_grass->GetSubRect(tileIndex));
	}

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
		if (coll->gType == TouchGrass::TYPE_LARGE_W1)
		{
			a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
				6, true, 4);
		}
		else if( coll->gType == TouchGrass::TYPE_LARGE_W5 )
		{
			a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 17,
				6, true, 4);
		}

		ClearRect(myQuad);
		visible = false;
	}
}





TouchTree::TouchTree(TouchGrassCollection *coll, int index,
	Edge *e, double quant)
	:TouchGrass(coll, index, e, quant)
{
	treeHeight = 500.f;

	yOffset = 0;
	V2d normal = edge->Normal();

	V2d p = edge->GetPosition(quant);
	center = p;
	center += normal * yOffset;

	center.y -= treeHeight / 2.f - 20; //arbitrary 20 rn

	V2d hitboxCenter = center;//p + normal * hitboxYOff;

	angle = 0;

	//probably make this a rectangle later.
	hurtBody.BasicCircleSetup(16, hitboxCenter);

	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth,
		treeHeight, Vector2f(center));

	V2d polyCenter = coll->myTerrain->GetDCenter();
	for (int i = 0; i < 4; ++i)
	{
		points[i] = V2d(myQuad[i].position) - polyCenter;
	}

	Reset();
}

void TouchTree::Reset()
{
	float treeHeight = 500;//coll->ts_grass->tileHeight
	SetRectRotation(myQuad, angle, coll->ts_grass->tileWidth,
		treeHeight, Vector2f(center));
	visible = true;
	action = STILL;
	frame = 0;
	currTile = -1;
	UpdateSprite();
}

void TouchTree::UpdateSprite()
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
		SetRectSubRect(myQuad, coll->ts_grass->GetSubRect(1));
	}

}

void TouchTree::Update()
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

void TouchTree::Touch(Actor *a)
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

void TouchTree::Destroy(Actor *a)
{
	if (visible)
	{
		//a->ActivateEffect(DrawLayer::BEHIND_ENEMIES, coll->ts_grass, center, false, angle, 26,
		//	6, true, 4);

		ClearRect(myQuad);
		visible = false;
	}
}